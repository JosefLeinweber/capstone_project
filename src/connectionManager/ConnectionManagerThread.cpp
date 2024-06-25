#include "ConnectionManagerThread.h"

#include <iostream>

MyCustomMessage::MyCustomMessage(const std::string &ipAddress, int port)
    : ip(ipAddress), port(port)
{
}


ConnectionManagerThread::ConnectionManagerThread(
    ConfigurationData localConfigurationData,
    AudioBufferFIFO &inputRingBuffer,
    AudioBufferFIFO &outputRingBuffer,
    std::atomic<bool> &startConnection,
    std::atomic<bool> &stopConnection)
    : juce::Thread("ConnectionManagerThread"),
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


    waitForThreadToExit(1000);
}

void ConnectionManagerThread::run()
{
    std::cout << "ConnectionManagerThread | run" << std::endl;

    setupHost();

    asyncWaitForConnection(std::chrono::milliseconds(2000000));

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
        while (m_providerThread->isThreadRunning() &&
               m_consumerThread->isThreadRunning() && !m_stopConnection &&
               !threadShouldExit())

        {
            wait(100);
        }
        stopProviderAndConsumerThreads(std::chrono::seconds(5));
        resetToStartState();
        if (threadShouldExit())
        {
            waitForThreadToExit(1000);
        }
    }
}

bool ConnectionManagerThread::exchangeConfigurationDataWithRemote(
    ConfigurationData configurationData)
{
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
        return true;
    }
    catch (std::exception &e)
    {
        std::cout << "ConnectionManagerThread::sendConfigurationData | "
                     "Failed to send configuration data"
                  << std::endl;
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
        return true;
    }
    catch (std::exception &e)
    {
        std::cout << "ConnectionManagerThread::receiveConfigurationData | "
                     "Failed to receive configuration data"
                  << std::endl;
        return false;
    }
}

ConfigurationData ConnectionManagerThread::remoteConfigurationDataFromGUI()
{
    //TODO: implement this function
    //! only placeholder
    ConfigurationData remoteConfigurationData;

    remoteConfigurationData.set_consumer_port(7001);
    remoteConfigurationData.set_provider_port(7002);

    return remoteConfigurationData;
}


void ConnectionManagerThread::setupHost()
{
    m_host = std::make_unique<TcpHost>(m_ioContext,
                                       m_localConfigurationData.host_port());
    m_host->setupSocket();
}

void ConnectionManagerThread::resetToStartState()
{
    std::cout << "ConnectionManagerThread | resetToStartState" << std::endl;
    m_startConnection = false;
    m_stopConnection = false;
    m_incomingConnection = false;

    m_remoteConfigurationData = ConfigurationData();

    if (m_ioContextThread.joinable())
    {
        m_ioContext.stop();
        m_ioContextThread.join();
    }
}

void ConnectionManagerThread::callbackFunction(
    const boost::system::error_code &error)
{
    if (!error)
    {
        m_incomingConnection = true;
        std::cout << "callback > received data" << std::endl;
    }
    else
    {
        m_incomingConnection = false;
        std::cout << "callback > no data received" << std::endl;
        throw std::runtime_error("Failed to receive data! With error: " +
                                 error.message());
    }
}

void ConnectionManagerThread::asyncWaitForConnection(
    std::chrono::milliseconds timeout)
{

    std::cout << "ConncectionManager::asyncWaitForConnection" << std::endl;
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
    }
    catch (std::exception &e)
    {
        std::cout
            << "ConnectionManagerThread::startIOContextInDifferentThread | "
               "Failed to start ioContext in different thread"
            << std::endl;
        throw std::runtime_error(
            "Failed to start ioContext in different thread! With error: " +
            std::string(e.what()));
    }
}

void ConnectionManagerThread::initializeConnection(
    ConfigurationData remoteConfigurationData)
{
    try
    {
        m_host->initializeConnection(remoteConfigurationData.ip(),
                                     remoteConfigurationData.host_port());
    }
    catch (std::exception &e)
    {
        std::cout << "ConnectionManagerThread::initializeConnection | "
                     "Failed to initialize connection"
                  << std::endl;
        throw std::runtime_error(
            "Failed to initialize connection! With error: " +
            std::string(e.what()));
    }
}


bool ConnectionManagerThread::startUpProviderAndConsumerThreads(
    ConfigurationData localConfigurationData,
    ConfigurationData remoteConfigurationData,
    std::chrono::milliseconds timeout)
{
    //TODO: implement a way to get the provider and consumer address from the handshake


    m_providerThread = std::make_unique<ProviderThread>(remoteConfigurationData,
                                                        localConfigurationData,
                                                        m_outputRingBuffer);
    m_consumerThread = std::make_unique<ConsumerThread>(remoteConfigurationData,
                                                        localConfigurationData,
                                                        m_inputRingBuffer);

    m_providerThread->startThread(juce::Thread::Priority::highest);
    m_consumerThread->startThread(juce::Thread::Priority::highest);

    std::cout << "are threads running : " << m_providerThread->isThreadRunning()
              << " " << m_consumerThread->isThreadRunning() << std::endl;

    //time now
    auto startTime = std::chrono::high_resolution_clock::now();

    while (!m_providerThread->isThreadRunning() ||
           !m_consumerThread->isThreadRunning())
    {
        if (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - startTime)
                .count() > timeout.count())
        {
            return false;
        }
        wait(100);
    }
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

void ConnectionManagerThread::sendMessageToGUI(const std::string &ip, int port)
{
    std::cout << "ConnectionManagerThread::sendMessageToGUI | "
                 "Sending message to GUI"
              << std::endl;
    juce::MessageManager::callAsync([this, ip, port]() {
        MyCustomMessage *message = new MyCustomMessage(ip, port);
        this->postMessage(message);
    });
}

void ConnectionManagerThread::handleMessage(const juce::Message &message)
{
    if (auto *customMessage = dynamic_cast<const MyCustomMessage *>(&message))
    {
        std::cout << "ConnectionManagerThread::handleMessage | "
                     "Received message from GUI: "
                  << customMessage->ip << " " << customMessage->port
                  << std::endl;
        m_remoteConfigurationData.set_ip("127.0.0.1");
        m_remoteConfigurationData.set_host_port(8000);
        m_remoteConfigurationData.set_consumer_port(8001);
        m_remoteConfigurationData.set_provider_port(8002);
        m_startConnection = true;
    }
}

void ConnectionManagerThread::setAudioProcessor(
    juce::AudioProcessor *audioProcessor)
{
    m_audioProcessor = audioProcessor;
}
