#include "ConnectionManagerThread.h"
#include <iostream>

ConnectionManagerThread::ConnectionManagerThread(
    AudioBufferFIFO &inputRingBuffer,
    AudioBufferFIFO &outputRingBuffer)
    : juce::Thread("Network Thread"), m_inputRingBuffer(inputRingBuffer),
      m_outputRingBuffer(outputRingBuffer)
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
    waitForThreadToExit(1000);
}

void ConnectionManagerThread::run()
{
    while (!threadShouldExit())
    {

        //! HARDOCDED IP AND PORT
        addressData hostAddress("127.0.0.1", 8000);
        addressData consumerAddress("127.0.0.1", 8001);
        addressData providerAddress("127.0.0.1", 8002);
        addressData remoteConsumerAddress("127.0.0.1", 8003);
        setupHost(hostAddress);

        asyncWaitForConnection(std::chrono::milliseconds(10000));

        // run ioContext on ioContext thread ?

        while (!m_incomingConnection /** or a event from gui happens */)
        {
            wait(100);
        }

        //generateConfigurationData();

        //exchangeConfigurationDataWithRemote();


        if (startUpProviderAndConsumerThreads(consumerAddress,
                                              providerAddress,
                                              remoteConsumerAddress))
        {
            asyncWaitForClosingRequest();
            while (!m_closingRequest /* or a event from gui happens */)
            {
                wait(100);
            }

            closeProviderAndConsumerThreads();
        }
    }
}

void ConnectionManagerThread::generateConfigurationData()
{
    //TODO: implement automatic configuration data generation
    //! HARDCODED CONFIGURATION DATA CHANGE IT!
    ConfigurationDataStruct configurationData;
    configurationData.providerAddress = addressData("127.0.0.1", 8002);
    configurationData.consumerAddress = addressData("127.0.0.1", 8001);
    m_localConfigurationData = configurationData;
}

bool ConnectionManagerThread::exchangeConfigurationDataWithRemote(
    ConfigurationDataStruct configurationData)
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
    ConfigurationDataStruct configurationData)
{
    pbConfigurationData pbConfigurationData = configurationData.toPb();
    std::string serializedData =
        m_host->serializeConfigurationData(pbConfigurationData);
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
        m_localConfigurationData =
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


void ConnectionManagerThread::setupHost(addressData hostAddress)
{
    m_host = std::make_unique<TcpHost>(m_ioContext, hostAddress.port);
    m_host->setupSocket();
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
}

void ConnectionManagerThread::initializeConnection(addressData remoteAddress)
{
    try
    {
        m_host->initializeConnection(remoteAddress);
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
    ConfigurationDataStruct remoteConfigurationData,
    ConfigurationDataStruct localConfigurationData,
    std::chrono::seconds timeout)
{
    //TODO: implement a way to get the provider and consumer address from the handshake

    m_providerThread = std::make_unique<ProviderThread>(remoteConfigurationData,
                                                        localConfigurationData,
                                                        m_outputRingBuffer,
                                                        m_isProviderConnected);
    m_consumerThread = std::make_unique<ConsumerThread>(remoteConfigurationData,
                                                        localConfigurationData,
                                                        m_inputRingBuffer,
                                                        m_isConsumerConnected);

    m_providerThread->startThread();
    m_consumerThread->startThread();


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
    m_providerThread->signalThreadShouldExit();
    m_consumerThread->signalThreadShouldExit();

    auto startTime = std::chrono::high_resolution_clock::now();

    while (m_providerThread->isThreadRunning() ||
           m_consumerThread->isThreadRunning())
    {
        if (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - startTime)
                .count() > timeout.count())
        {
            m_providerThread->waitForThreadToExit(1000);
            m_consumerThread->waitForThreadToExit(1000);
            return;
        }
        wait(100);
    }
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

void ConnectionManagerThread::closeProviderAndConsumerThreads()
{
    m_providerThread->waitForThreadToExit(1000);
    m_consumerThread->waitForThreadToExit(1000);
}

bool ConnectionManagerThread::incomingConnection() const
{
    return m_incomingConnection;
}

ConfigurationDataStruct ConnectionManagerThread::getConfigurationData() const
{
    return m_localConfigurationData;
}

void ConnectionManagerThread::stopThreadSafely()
{
    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }
}
