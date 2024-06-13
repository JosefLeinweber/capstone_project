#include "ConnectionManagerThread.h"
#include <iostream>

ConnectionManagerThread::ConnectionManagerThread(
    AudioBufferFIFO &inputRingBuffer,
    AudioBufferFIFO &outputRingBuffer)
    : juce::Thread("Network Thread"), m_inputRingBuffer(inputRingBuffer),
      m_outputRingBuffer(outputRingBuffer)
{
}

void ConnectionManagerThread::run()
{
    while (!threadShouldExit())
    {

        //! HARDOCDED IP AND PORT
        addressData hostAddress("127.0.0.1", 8000);
        setupHost(hostAddress);

        asyncWaitForConnection(std::chrono::milliseconds(10000));

        while (!m_incomingConnection /** or a event from gui happens */)
        {
            wait(100);
        }

        bool valid_connection = validateConnection();

        if (valid_connection)
        {
            startUpProviderAndConsumerThreads();

            waitForClosingRequest();

            closeProviderAndConsumerThreads();
        }
    }
}

void ConnectionManagerThread::setupHost(addressData hostAddress)
{
    m_host = std::make_unique<Host>(hostAddress);
    m_host->setupSocket();
}

void ConnectionManagerThread::callbackFunction(
    const boost::system::error_code &error,
    std::size_t bytes_transferred)
{
    if (!error && bytes_transferred > 0)
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
                  std::placeholders::_1,
                  std::placeholders::_2),
        timeout);
}

void ConnectionManagerThread::stopThreadSafely()
{
    signalThreadShouldExit();
    waitForThreadToExit(1000);
}

bool ConnectionManagerThread::validateConnection()
{
    bool connected = false;
    if (m_incomingConnection)
    {
        std::cout << "ConnectionManager::validateConnection getRemoteAddress: "
                  << m_host->getRemoteAddress().ip << std::endl;
        m_host->sendHandshake(m_host->getRemoteAddress());
        connected = m_host->waitForHandshake();
    }
    else
    {
        connected = m_host->waitForHandshake();
        if (connected)
        {
            m_host->sendHandshake(m_host->getRemoteAddress());
        }
    }
    return connected;
}

bool ConnectionManagerThread::startUpProviderAndConsumerThreads()
{
    //! temporary hardcoded implementation of provider and consumer address in header!!!!!!!!!!
    addressData m_providerAddress("127.0.0.1", 8001);
    addressData m_consumerAddress("127.0.0.1", 8002);
    addressData m_remoteConsumerAddress("127.0.0.1", 8012);

    m_isProviderConnected = false;
    m_isConsumerConnected = false;
    m_providerThread = std::make_unique<ProviderThread>(m_providerAddress,
                                                        m_remoteConsumerAddress,
                                                        m_outputRingBuffer,
                                                        m_isProviderConnected);
    m_consumerThread = std::make_unique<ConsumerThread>(m_consumerAddress,
                                                        m_inputRingBuffer,
                                                        m_isConsumerConnected);
    m_providerThread->startThread();
    m_consumerThread->startThread();

    //time now
    auto startTime = std::chrono::high_resolution_clock::now();

    while (!m_isProviderConnected || !m_isConsumerConnected)
    {
        if (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - startTime)
                .count() > 5)
        {
            m_providerThread->waitForThreadToExit(1000);
            m_consumerThread->waitForThreadToExit(1000);
            return false;
        }
        wait(100);
    }
    return true;
}


void ConnectionManagerThread::waitForClosingRequest()
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
