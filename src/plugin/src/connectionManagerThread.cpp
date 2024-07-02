#include "ConnectDAWs/connectionManagerThread.h"


ConnectionManagerThread::ConnectionManagerThread(
    std::shared_ptr<Messenger> &guiMessenger,
    std::shared_ptr<Messenger> &cmtMessenger,
    ConfigurationData localConfigurationData,
    AudioBufferFIFO &inputRingBuffer,
    AudioBufferFIFO &outputRingBuffer,
    std::atomic<bool> &startConnection,
    std::atomic<bool> &stopConnection)
    : juce::Thread("ConnectionManagerThread"), m_guiMessenger(guiMessenger),
      m_cmtMessenger(cmtMessenger),
      m_localConfigurationData(localConfigurationData),
      m_inputRingBuffer(inputRingBuffer), m_outputRingBuffer(outputRingBuffer),
      m_startConnection(startConnection), m_stopConnection(stopConnection)
{
    m_fileLogger = std::make_unique<FileLogger>("ConnectDAWs");
}

ConnectionManagerThread::~ConnectionManagerThread()
{
    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }

    if (m_providerThread && m_providerThread->isThreadRunning())
    {
        m_providerThread->signalThreadShouldExit();
    }

    if (m_consumerThread && m_consumerThread->isThreadRunning())
    {
        m_consumerThread->signalThreadShouldExit();
    }


    if (m_ioContextThread.joinable())
    {
        m_ioContext.stop();
        m_ioContextThread.join();
    }

    //m_fileLogger->logMessage("ConnectionManagerThread | closing...");
    waitForThreadToExit(1000);
}

void ConnectionManagerThread::run()
{
    while (!threadShouldExit())
    {
        setup();

        asyncWaitForConnection(std::chrono::milliseconds(0));

        while (!m_incomingConnection && !m_startConnection)
        {
            if (threadShouldExit())
            {
                return;
            }
            wait(100);
        }

        if (m_startConnection)
        {
            initializeConnection(m_remoteConfigurationData);
        }

        if (exchangeConfigurationDataWithRemote(m_localConfigurationData) &&
            startUpProviderAndConsumerThreads(m_localConfigurationData,
                                              m_remoteConfigurationData,
                                              std::chrono::milliseconds(2000)))
        {
            sendMessageToGUI("status", "Start streaming...");
            while (m_providerThread->isThreadRunning() &&
                   m_consumerThread->isThreadRunning() && !m_stopConnection &&
                   !threadShouldExit())

            {
                wait(100);
            }
            m_fileLogger->logMessage("ConnectionManagerThread | "
                                     "run | Stopped streaming...");
            m_fileLogger->logMessage(
                "providerThread is running: " +
                std::to_string(m_providerThread->isThreadRunning()));
            m_fileLogger->logMessage(
                "consumerThread is running: " +
                std::to_string(m_consumerThread->isThreadRunning()));
            m_fileLogger->logMessage("m_stopConnection: " +
                                     std::to_string(m_stopConnection));
            stopProviderAndConsumerThreads(std::chrono::seconds(5));
            sendMessageToGUI("status", "Stoped streaming!");
        }
        else
        {
            sendMessageToGUI("status",
                             "Failed to start streaming, please try again...");
        }
        resetToStartState();
    }
}

void ConnectionManagerThread::setup()
{
    initCMTMessenger();
    setupHost();
    m_fileLogger->logMessage("ConnectionManagerThread | setup finished!");
}

void ConnectionManagerThread::initCMTMessenger()
{
    m_cmtMessenger = std::make_shared<Messenger>(
        std::bind(&ConnectionManagerThread::handleMessage,
                  this,
                  std::placeholders::_1));
    while (!m_guiMessenger)
    {
        m_fileLogger->logMessage("ConnectionManagerThread | waiting for GUI "
                                 "messenger to be initialized...");
        wait(100);
    }
}

void ConnectionManagerThread::handleMessage(const juce::Message &message)
{
    if (auto *customMessage = dynamic_cast<const MessageToCMT *>(&message))
    {
        // if (customMessage->type == "stop")
        // {
        //     m_stopConnection = true;
        // }
        // else
        // {
        //     std::cout << "ConnectionManagerThread::handleMessage | "
        //                  "Received unknown message from GUI"
        //               << std::endl;
        // }
        m_fileLogger->logMessage("ConnectionManagerThread | handleMessage | "
                                 "Received message from GUI");
        m_fileLogger->logMessage(customMessage->ip + " | " +
                                 std::to_string(customMessage->port));
        m_fileLogger->logMessage("ConnectionManagerThread | handleMessage | "
                                 "setting m_startConnection to true...");
        m_remoteConfigurationData.set_ip(customMessage->ip);
        m_remoteConfigurationData.set_host_port(customMessage->port);
        m_startConnection = true;
    }
    else
    {
        m_fileLogger->logMessage("ConnectionManagerThread | handleMessage | "
                                 "Received unknown message from GUI");
    }
}

void ConnectionManagerThread::setupHost()
{
    m_host = std::make_unique<TcpHost>(m_ioContext,
                                       m_localConfigurationData.host_port());
    m_host->setupSocket();
    sendMessageToGUI("localIpAndPort",
                     ":" +
                         std::to_string(m_localConfigurationData.host_port()));
}


void ConnectionManagerThread::callbackFunction(
    const boost::system::error_code &error)
{
    if (!error)
    {
        m_incomingConnection = true;
    }
    else
    {
        m_incomingConnection = false;
    }
}

void ConnectionManagerThread::asyncWaitForConnection(
    std::chrono::milliseconds timeout)
{
    m_fileLogger->logMessage(
        "ConnectionManagerThread | asyncWaitForConnection");
    sendMessageToGUI("status", "Waiting for connection...");
    m_host->asyncWaitForConnection(
        std::bind(&ConnectionManagerThread::callbackFunction,
                  this,
                  std::placeholders::_1),
        timeout);
    startIOContextInDifferentThread();
}

void ConnectionManagerThread::startIOContextInDifferentThread()
{
    try
    {
        m_ioContextThread = std::jthread([this]() { m_ioContext.run(); });
        m_fileLogger->logMessage(
            "ConnectionManagerThread | startIOContextInDifferentThread | "
            "Started io context thread");
    }
    catch (std::exception &e)
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | startIOContextInDifferentThread | "
            "Failed to start io context thread with error: ");
        m_fileLogger->logMessage(e.what());
    }
}

void ConnectionManagerThread::initializeConnection(
    ConfigurationData remoteConfigurationData)
{
    try
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | initializeConnection | Trying to "
            "connect to remote: " +
            remoteConfigurationData.ip() + ":" +
            std::to_string(remoteConfigurationData.host_port()));
        sendMessageToGUI(
            "status",
            "Trying to connected to remote: " + remoteConfigurationData.ip() +
                ":" + std::to_string(remoteConfigurationData.host_port()));
        m_host->initializeConnection(remoteConfigurationData.ip(),
                                     remoteConfigurationData.host_port());
        if (m_ioContextThread.joinable())
        {
            m_ioContextThread.join();
        }
        startIOContextInDifferentThread();
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "initializeConnection | Connected to remote!");
    }
    catch (std::exception &e)
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | initializeConnection | Failed to "
            "connect to remote: " +
            remoteConfigurationData.ip() + ":" +
            std::to_string(remoteConfigurationData.host_port()) +
            " with error: ");
        m_fileLogger->logMessage(e.what());
        sendMessageToGUI(
            "status",
            "Failed to connect to remote: " + remoteConfigurationData.ip() +
                ":" + std::to_string(remoteConfigurationData.host_port()));
    }
}

bool ConnectionManagerThread::exchangeConfigurationDataWithRemote(
    ConfigurationData configurationData)
{

    if (m_host->incomingConnection())
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | exchangeConfigurationDataWithRemote | "
            "incoming connection...");
        return receiveConfigurationData() &&
               sendConfigurationData(configurationData);
    }
    else
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | exchangeConfigurationDataWithRemote | "
            "outgoing connection...");
        return sendConfigurationData(configurationData) &&
               receiveConfigurationData();
    }
}

bool ConnectionManagerThread::sendConfigurationData(
    ConfigurationData localConfigurationData)
{
    std::string serializedData =
        m_host->serializeConfigurationData(localConfigurationData);
    try
    {
        m_host->send(serializedData);
        m_fileLogger->logMessage(
            "ConnectionManagerThread | sendConfigurationData | Sent "
            "configuration data");
        return true;
    }
    catch (std::exception &e)
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | sendConfigurationData | Failed to send "
            "configuration data with error: ");
        m_fileLogger->logMessage(e.what());
        return false;
    }
}

bool ConnectionManagerThread::receiveConfigurationData()
{
    try
    {
        std::string serializedData = m_host->receiveConfiguration();
        m_remoteConfigurationData =
            m_host->deserializeConfigurationData(serializedData);
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "receiveConfigurationData | Received "
                                 "configuration data");
        return true;
    }
    catch (std::exception &e)
    {
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "receiveConfigurationData | Failed to receive "
                                 "configuration data with error: ");
        m_fileLogger->logMessage(e.what());
        return false;
    }
}


bool ConnectionManagerThread::startUpProviderAndConsumerThreads(
    ConfigurationData localConfigurationData,
    ConfigurationData remoteConfigurationData,
    std::chrono::milliseconds timeout)
{
    m_fileLogger->logMessage(
        "ConnectionManagerThread | startUpProviderAndConsumerThreads");
    std::chrono::milliseconds timeoutForProviderAndConsumerThreads =
        std::chrono::milliseconds(5000);
    m_providerThread =
        std::make_unique<ProviderThread>(remoteConfigurationData,
                                         localConfigurationData,
                                         m_outputRingBuffer,
                                         timeoutForProviderAndConsumerThreads);
    m_consumerThread =
        std::make_unique<ConsumerThread>(remoteConfigurationData,
                                         localConfigurationData,
                                         m_inputRingBuffer,
                                         timeoutForProviderAndConsumerThreads);

    //TODO: change to realtime threads
    m_providerThread->startRealtimeThread(juce::Thread::RealtimeOptions());
    m_consumerThread->startRealtimeThread(juce::Thread::RealtimeOptions());


    auto startTime = std::chrono::high_resolution_clock::now();
    while (!m_providerThread->isThreadRunning() ||
           !m_consumerThread->isThreadRunning())
    {
        if (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - startTime)
                .count() > timeout.count())
        {
            m_fileLogger->logMessage(
                "ConnectionManagerThread | startUpProviderAndConsumerThreads | "
                "Timeout");
            return false;
        }
        wait(100);
    }
    m_fileLogger->logMessage(
        "ConnectionManagerThread | startUpProviderAndConsumerThreads | "
        "Provider and Consumer threads started");
    return true;
}


void ConnectionManagerThread::stopProviderAndConsumerThreads(
    std::chrono::seconds timeout)
{
    if (m_providerThread->isThreadRunning())
        m_providerThread->signalThreadShouldExit();

    if (m_consumerThread->isThreadRunning())
        m_consumerThread->signalThreadShouldExit();

    m_providerThread->waitForThreadToExit(timeout.count());
    m_consumerThread->waitForThreadToExit(timeout.count());
}

void ConnectionManagerThread::resetToStartState()
{
    m_fileLogger->logMessage("ConnectionManagerThread | resetToStartState");
    m_startConnection = false;
    m_stopConnection = false;
    m_incomingConnection = false;

    m_remoteConfigurationData = ConfigurationData();

    if (m_ioContextThread.joinable())
    {
        m_ioContext.stop();
        m_ioContextThread.join();
    }
    m_fileLogger->logMessage("ConnectionManagerThread | ready for new "
                             "connection...");
}

bool ConnectionManagerThread::incomingConnection() const
{
    return m_incomingConnection;
}

void ConnectionManagerThread::sendMessageToGUI(std::string type,
                                               std::string message)
{
    m_fileLogger->logMessage("ConnectionManagerThread | sendMessageToGUI | "
                             "Sending message to GUI");
    m_guiMessenger->postMessage(new MessageToGUI(type, message));
}
