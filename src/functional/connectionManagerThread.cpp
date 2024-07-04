#include "ConnectDAWs/connectionManagerThread.h"


ConnectionManagerThread::ConnectionManagerThread(
    std::shared_ptr<Messenger> &guiMessenger,
    std::shared_ptr<Messenger> &cmtMessenger,
    ConfigurationData localConfigurationData,
    AudioBufferFIFO &inputRingBuffer,
    AudioBufferFIFO &outputRingBuffer,
    std::atomic<bool> &startConnection,
    std::atomic<bool> &stopConnection,
    const std::string threadName)
    : juce::Thread(threadName), m_guiMessenger(guiMessenger),
      m_cmtMessenger(cmtMessenger),
      m_localConfigurationData(localConfigurationData),
      m_inputRingBuffer(inputRingBuffer), m_outputRingBuffer(outputRingBuffer),
      m_startConnection(startConnection), m_stopConnection(stopConnection)
{
    m_fileLogger = std::make_unique<FileLogger>("ConnectDAWs", threadName);
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

        //TODO change m_incomingConnection to isConnected()??
        while (!m_incomingConnection && !m_startConnection)
        {
            if (threadShouldExit())
            {
                return;
            }
            wait(100);
        }

        m_fileLogger->logMessage("ConnectionManagerThread | run | Incoming "
                                 "connection: " +
                                 std::to_string(isConnected()));

        if (m_startConnection &&
            validateIpAddress(m_remoteConfigurationData.ip()))
        {
            initializeConnection(m_remoteConfigurationData);
        }

        if (isConnected())
        {
            if (exchangeConfigurationDataWithRemote(m_localConfigurationData) &&
                startUpProviderAndConsumerThreads(
                    m_localConfigurationData,
                    m_remoteConfigurationData,
                    std::chrono::milliseconds(2000)))
            {
                sendMessageToGUI("status", "Started stream");
                while (m_providerThread->isThreadRunning() &&
                       m_consumerThread->isThreadRunning() &&
                       !m_stopConnection && !threadShouldExit())

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
                sendMessageToGUI("status", "Stoped stream");
            }
            else
            {
                sendMessageToGUI("status", "Failed to start stream");
            }
        }
        else
        {
            sendMessageToGUI("status", "Failed to connect");
        }
        while (!m_readyForNextConnection && !threadShouldExit())
        {

            wait(100);
        }

        // m_startConnection = false;
        resetToStartState();
    }
}

void ConnectionManagerThread::setup()
{
    if (!m_cmtMessenger)
    {
        initCMTMessenger();
    }

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
    m_fileLogger->logMessage("ConnectionManagerThread | handleMessage | "
                             "Received message from GUI");
    if (auto *addressMessage = dynamic_cast<const AddressMessage *>(&message))
    {
        m_fileLogger->logMessage(addressMessage->m_ip + " | " +
                                 std::to_string(addressMessage->m_port));
        m_remoteConfigurationData.set_ip(addressMessage->m_ip);
        m_remoteConfigurationData.set_host_port(addressMessage->m_port);
    }
    else if (auto *statusMessage =
                 dynamic_cast<const StatusMessage *>(&message))
    {
        m_fileLogger->logMessage(statusMessage->m_messageType + " | " +
                                 statusMessage->m_message);
        if (statusMessage->m_messageType == "stop")
        {
            m_stopConnection = true;
        }
        else if (statusMessage->m_messageType == "start")
        {
            m_startConnection = true;
        }
        else if (statusMessage->m_messageType == "ready")
        {
            m_readyForNextConnection = true;
        }
        else
        {
            m_fileLogger->logMessage(
                "type of message unknown, no action taken");
        }
    }
}

void ConnectionManagerThread::setupHost()
{

    m_host = std::make_unique<TcpHost>(m_ioContext,
                                       m_localConfigurationData.host_port());
    m_host->setupSocket();
    sendMessageToGUI("ip", "   ");
}


void ConnectionManagerThread::callbackFunction(
    const boost::system::error_code &error)
{
    if (!error)
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | callbackFunction | Incoming connection "
            "accepted");
        m_incomingConnection = true;
    }
    else
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | callbackFunction | Failed to accept "
            "incoming connection with error: ");
        m_fileLogger->logMessage(error.message());
        m_incomingConnection = false;
    }
}

void ConnectionManagerThread::asyncWaitForConnection(
    std::chrono::milliseconds timeout)
{
    m_fileLogger->logMessage(
        "ConnectionManagerThread | asyncWaitForConnection");
    sendMessageToGUI("status", "Ready to connect");
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

void ConnectionManagerThread::stopAsyncWaitForConnection()
{
    try
    {
        m_host->stopAsyncOperations();
        m_fileLogger->logMessage(
            "ConnectionManagerThread | stopAsyncWaitForConnection");
    }
    catch (std::exception &e)
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | stopAsyncWaitForConnection | Failed to "
            "stop io context thread with error: ");
        m_fileLogger->logMessage(e.what());
    }
}

bool ConnectionManagerThread::validateIpAddress(std::string ip)
{
    return m_host->validateIpAddress(ip);
}

bool ConnectionManagerThread::initializeConnection(
    ConfigurationData remoteConfigurationData)
{
    try
    {
        //TODO: refactor this function, it does too much

        stopAsyncWaitForConnection();
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
        // blocking call to wait for either connection or timeout
        m_ioContext.run_one_for(std::chrono::milliseconds(5000));
        m_fileLogger->logMessage(
            "ConnectionManagerThread | "
            "initializeConnection | Connected to remote? " +
            isConnected());
        return isConnected();
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
        return false;
    }
}

bool ConnectionManagerThread::isConnected()
{
    return m_host->isConnected();
}

bool ConnectionManagerThread::exchangeConfigurationDataWithRemote(
    ConfigurationData configurationData)
{
    sendMessageToGUI("status", "Exchanging configuration data...");
    if (m_incomingConnection)
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
        m_fileLogger->logMessage(localConfigurationData.DebugString());
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
        m_fileLogger->logMessage(m_remoteConfigurationData.DebugString());
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
    sendMessageToGUI("status", "Starting audio streams...");
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
    m_fileLogger->logMessage(
        "ConnectionManagerThread | stopProviderAndConsumerThreads | "
        "Provider and Consumer threads stopped");
}

void ConnectionManagerThread::resetToStartState()
{
    m_fileLogger->logMessage("ConnectionManagerThread | resetToStartState");
    m_startConnection = false;
    m_stopConnection = false;
    m_incomingConnection = false;
    m_readyForNextConnection = false;
    m_remoteConfigurationData = ConfigurationData();

    m_host.reset();

    if (m_ioContextThread.joinable())
    {
        m_ioContext.stop();
        m_ioContextThread.join();
    }
    if (!m_ioContext.stopped())
    {
        m_ioContext.stop();
    }
    m_ioContext.restart();
    m_fileLogger->logMessage("ConnectionManagerThread | ready for new "
                             "connection...");
}

void ConnectionManagerThread::sendMessageToGUI(std::string type,
                                               std::string message)
{
    m_fileLogger->logMessage("ConnectionManagerThread | sendMessageToGUI | "
                             "Sending message to GUI | " +
                             message);
    m_guiMessenger->postMessage(new StatusMessage(type, message));
}
