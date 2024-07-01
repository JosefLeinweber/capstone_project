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
    setup();

    asyncWaitForConnection(std::chrono::milliseconds(2000000));

    while (!m_incomingConnection && !m_startConnection)
    {
        if (threadShouldExit())
        {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (m_startConnection)
    {
        std::cout << "ConnectionManagerThread | got notified by other "
                     "thread, shoudl start connection"
                  << std::endl;

        initializeConnection(m_remoteConfigurationData);
    }

    if (exchangeConfigurationDataWithRemote(m_localConfigurationData) &&
        startUpProviderAndConsumerThreads(m_localConfigurationData,
                                          m_remoteConfigurationData,
                                          std::chrono::milliseconds(20000)))
    {
        MessageToGUI *start_message =
            new MessageToGUI("status", "Start streaming...");
        sendMessageToGUI(start_message);
        while (m_providerThread->isThreadRunning() &&
               m_consumerThread->isThreadRunning() && !m_stopConnection &&
               !threadShouldExit())

        {
            wait(100);
        }
        stopProviderAndConsumerThreads(std::chrono::seconds(5));
        MessageToGUI *stop_message =
            new MessageToGUI("status", "Stoped streaming!");
        sendMessageToGUI(stop_message);
        resetToStartState();
        if (threadShouldExit())
        {
            waitForThreadToExit(1000);
        }
    }
}

void ConnectionManagerThread::setup()
{
    m_fileLogger = std::make_unique<FileLogger>("ConnectDAWs");
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

void ConnectionManagerThread::setupHost()
{
    m_host = std::make_unique<TcpHost>(m_ioContext,
                                       m_localConfigurationData.host_port());
    m_host->setupSocket();
    MessageToGUI *message = new MessageToGUI(
        "localIpAndPort",
        ":" + std::to_string(m_localConfigurationData.host_port()));
    sendMessageToGUI(message);
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
    MessageToGUI *message =
        new MessageToGUI("status", "Waiting for connection...");
    sendMessageToGUI(message);
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


bool ConnectionManagerThread::exchangeConfigurationDataWithRemote(
    ConfigurationData configurationData)
{
    m_fileLogger->logMessage(
        "ConnectionManagerThread | exchangeConfigurationDataWithRemote");
    if (m_host->incomingConnection())
    {
        return receiveConfigurationData() &&
               sendConfigurationData(configurationData);
    }
    else
    {
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


// ConfigurationData ConnectionManagerThread::remoteConfigurationDataFromGUI()
// {
//     //TODO: implement this function
//     //! only placeholder
//     ConfigurationData remoteConfigurationData;

//     remoteConfigurationData.set_consumer_port(7001);
//     remoteConfigurationData.set_provider_port(7002);

//     return remoteConfigurationData;
// }


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
        MessageToGUI *message1 = new MessageToGUI(
            "status",
            "Trying to connected to remote: " + remoteConfigurationData.ip() +
                ":" + std::to_string(remoteConfigurationData.host_port()));
        sendMessageToGUI(message1);
        m_host->initializeConnection(remoteConfigurationData.ip(),
                                     remoteConfigurationData.host_port());
        MessageToGUI *message = new MessageToGUI(
            "status",
            "Connected to remote: " + remoteConfigurationData.ip() + ":" +
                std::to_string(remoteConfigurationData.host_port()));
        sendMessageToGUI(message);
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
        MessageToGUI *message = new MessageToGUI(
            "status",
            "Failed to connect to remote: " + remoteConfigurationData.ip() +
                ":" + std::to_string(remoteConfigurationData.host_port()));
        sendMessageToGUI(message);
    }
}


bool ConnectionManagerThread::startUpProviderAndConsumerThreads(
    ConfigurationData localConfigurationData,
    ConfigurationData remoteConfigurationData,
    std::chrono::milliseconds timeout)
{
    m_fileLogger->logMessage(
        "ConnectionManagerThread | startUpProviderAndConsumerThreads");
    m_providerThread = std::make_unique<ProviderThread>(remoteConfigurationData,
                                                        localConfigurationData,
                                                        m_outputRingBuffer);
    m_consumerThread = std::make_unique<ConsumerThread>(remoteConfigurationData,
                                                        localConfigurationData,
                                                        m_inputRingBuffer);

    //TODO: change to realtime threads
    m_providerThread->startThread(juce::Thread::Priority::highest);
    m_consumerThread->startThread(juce::Thread::Priority::highest);


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

void ConnectionManagerThread::asyncWaitForClosingRequest()
{
    while (!threadShouldExit())
    {
        // 6. wait for a closing request from the gui or the connected virtual studio
        wait(100);
    }

    m_providerThread->stopThread(1000);
    m_consumerThread->stopThread(1000);
}


bool ConnectionManagerThread::incomingConnection() const
{
    return m_incomingConnection;
}

ConfigurationData ConnectionManagerThread::getRemoteConfigurationData() const
{
    return m_remoteConfigurationData;
}

void ConnectionManagerThread::sendMessageToGUI(juce::Message *message)
{
    // juce::MessageManager::callAsync(
    //     [this, message]() { m_guiMessenger->postMessage(message); });
    m_guiMessenger->postMessage(message);
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
