#include "ConnectDAWs/connectionManagerThread.h"


ConnectionManagerThread::ConnectionManagerThread(
    std::shared_ptr<Messenger> &guiMessenger,
    std::shared_ptr<Messenger> &cmtMessenger,
    ConfigurationData localConfigurationData,
    RingBuffer &inputRingBuffer,
    RingBuffer &outputRingBuffer,
    std::atomic<bool> &startConnection,
    std::atomic<bool> &stopConnection,
    std::shared_ptr<Benchmark> &benchmark,
    const std::string threadName)
    : juce::Thread(threadName), m_guiMessenger(guiMessenger),
      m_cmtMessenger(cmtMessenger),
      m_localConfigurationData(localConfigurationData),
      m_inputRingBuffer(inputRingBuffer), m_outputRingBuffer(outputRingBuffer),
      m_startConnection(startConnection), m_stopConnection(stopConnection),
      m_benchmark(benchmark)
{
    m_fileLogger = generateFileLogger("ConnectDAWs");
    m_currentTask =
        std::bind(&ConnectionManagerThread::establishConnection, this);
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
        if (establishConnection())
        {
            streamAudio();
        }
    }
}


bool ConnectionManagerThread::establishConnection()
{
    //TODO: Move setup out of the while loop
    setup();
    asyncWaitForConnection(std::chrono::milliseconds(0));

    while (!isConnected() && !m_startConnection)
    {
        if (threadShouldExit())
        {
            return false;
        }
        wait(100);
    }

    if (m_startConnection)
    {
        stopAsyncWaitForConnection();
        initializeConnection(m_remoteConfigurationData);
    }

    if (!isConnected())
    {
        encounteredError("Failed to connect");
        return false;
    }

    if (!validatePluginConfiguration())
    {
        //TODO: add additional field to encounterError to pass extra information for logging (maybe create error class)
        encounteredError("Failed to validate plugin configuration");
        return false;
    }

    return true;
}

void ConnectionManagerThread::streamAudio()
{
    if (!startUpProviderAndConsumerThreads(m_localConfigurationData,
                                           m_remoteConfigurationData))
    {
        encounteredError("Failed to start stream");
        return;
    }

    sendMessageToGUI("status", "Started stream");
    while (m_providerThread->isThreadRunning() &&
           m_consumerThread->isThreadRunning() && !m_stopConnection &&
           !threadShouldExit())
    {
        wait(100);
    }

    stopProviderAndConsumerThreads(std::chrono::seconds(5));

    sendMessageToGUI("status", "Stoped stream");

    resetToStartState();

    waitForUserToReadErrorMessage();
}

void ConnectionManagerThread::waitForUserToReadErrorMessage()
{
    while (!m_readyForNextConnection && !threadShouldExit())
    {
        wait(100);
    }
}

bool ConnectionManagerThread::validatePluginConfiguration()
{
    if (!exchangeConfigurationDataWithRemote(m_localConfigurationData))
    {
        return false;
    }

    if (!areConfigurationsEqual(m_localConfigurationData,
                                m_remoteConfigurationData))
    {
        return false;
    }

    return true;
}

bool ConnectionManagerThread::areConfigurationsEqual(
    ConfigurationData localConfigurationData,
    ConfigurationData remoteConfigurationData)
{
    return localConfigurationData.samples_per_block() ==
               remoteConfigurationData.samples_per_block() &&
           localConfigurationData.sample_rate() ==
               remoteConfigurationData.sample_rate();
}


void ConnectionManagerThread::encounteredError(std::string errorString)
{
    m_fileLogger->logMessage(std::string("ConnectionManagerThread | "
                                         "encounteredError | Error: ") +
                             errorString);


    m_guiMessenger->postMessage(new StatusMessage("status", errorString));

    resetToStartState();

    waitForUserToReadErrorMessage();
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
}


void ConnectionManagerThread::callbackFunction(
    const boost::system::error_code &error)
{
    if (!error)
    {
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "callbackFunction | Incoming connection "
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
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "stopAsyncWaitForConnection | Failed to "
                                 "stop io context thread with error: ");
        m_fileLogger->logMessage(e.what());
    }
}

bool ConnectionManagerThread::validateIpAddress(std::string ip)
{
    return m_host->validateIpAddress(ip);
}

void ConnectionManagerThread::initializeConnection(
    ConfigurationData remoteConfigurationData)
{
    try
    {
        //TODO: refactor this function, it does too much

        stopAsyncWaitForConnection();
        m_fileLogger->logMessage(
            "ConnectionManagerThread | initializeConnection | Trying to "
            "connect to remote: " +
            remoteConfigurationData.ip());
        sendMessageToGUI(
            "status",
            "Trying to connected to remote: " + remoteConfigurationData.ip() +
                ":" + std::to_string(remoteConfigurationData.host_port()));
        m_host->initializeConnection(remoteConfigurationData.ip(),
                                     remoteConfigurationData.host_port(),
                                     std::chrono::milliseconds(5000));
        // blocking call to wait for either connection or timeout
        m_ioContext.run_one_for(std::chrono::milliseconds(5000));
        m_fileLogger->logMessage(
            "ConnectionManagerThread | "
            "initializeConnection | Connected to remote? " +
            isConnected());
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
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "exchangeConfigurationDataWithRemote | "
                                 "incoming connection...");
        return receiveConfigurationData() &&
               sendConfigurationData(configurationData);
    }
    else
    {
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "exchangeConfigurationDataWithRemote | "
                                 "outgoing connection...");
        return sendConfigurationData(configurationData) &&
               receiveConfigurationData();
    }
}

bool ConnectionManagerThread::sendConfigurationData(
    ConfigurationData localConfigurationData)
{
    std::string serializedData;
    try
    {
        serializedData =
            m_host->serializeConfigurationData(localConfigurationData);
    }
    catch (std::exception &e)
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | sendConfigurationData | Failed to "
            "serialize configuration data with error: ");
        m_fileLogger->logMessage(e.what());
        return false;
    }

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
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "sendConfigurationData | Failed to send "
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
                                         m_benchmark,
                                         m_fileLogger,
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
            m_fileLogger->logMessage("ConnectionManagerThread | "
                                     "startUpProviderAndConsumerThreads | "
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

    m_providerThread->waitForThreadToExit(static_cast<int>(timeout.count()));
    m_consumerThread->waitForThreadToExit(static_cast<int>(timeout.count()));
    m_fileLogger->logMessage(
        "ConnectionManagerThread | stopProviderAndConsumerThreads | "
        "Provider and Consumer threads stopped");
    //TODO: make gui show continue button when threads are stoped and set m_readyForNextConnection on continue button press
    //m_readyForNextConnection = true;
    if (m_benchmark != nullptr)
    {
        logBenchmarkResults();
        m_fileLogger->logMessage("ConnectionManagerThread | "
                                 "stopProviderAndConsumerThreads | Benchmark "
                                 "results logged");
    }
    else
    {
        m_fileLogger->logMessage(
            "ConnectionManagerThread | "
            "stopProviderAndConsumerThreads | No benchmark "
            "results to log");
    }
}

void ConnectionManagerThread::logBenchmarkResults()
{
    m_fileLogger->logMessage("ConnectionManagerThread | logBenchmarkResults");
    m_benchmark->logBenchmark(m_benchmark->m_startTimestamps,
                              m_benchmark->m_endTimestamps,
                              "Network Benchmark",
                              m_fileLogger);
    m_fileLogger->logMessage("ConnectionManagerThread | logBenchmarkResults | "
                             "Benchmark results logged");
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
