#include "ConnectionManagerThread.h"

ConnectionManagerThread::ConnectionManagerThread(
    AudioBufferFIFO &inputRingBuffer,
    AudioBufferFIFO &outputRingBuffer)
    : juce::Thread("Network Thread"), m_inputRingBuffer(inputRingBuffer),
      m_outputRingBuffer(outputRingBuffer) {};

void ConnectionManagerThread::run()
{
    while (!threadShouldExit())
    {
        setupHost();

        asyncWaitForConnection();

        while (!m_incommingConnection /** or a event from gui happens */)
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
};

void ConnectionManagerThread::setupHost()
{
    m_host = std::make_unique<Host>();
    m_host->setupSocket();
};

void ConnectionManagerThread::callbackFunction(
    const boost::system::error_code &error,
    std::size_t bytes_transferred)
{
    if (!error)
    {
        m_incommingConnection = true;
    }
    else
    {
        m_incommingConnection = false;
    }
};

void ConnectionManagerThread::asyncWaitForConnection()
{
    m_host->asyncWaitForConnection(&callbackFunction);
};

void ConnectionManagerThread::stopThreadSafely()
{
    signalThreadShouldExit();
    waitForThreadToExit(1000);
};

bool ConnectionManagerThread::validateConnection()
{
    bool connected = false;
    if (m_incommingConnection)
    {
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
};

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
};


void ConnectionManagerThread::waitForClosingRequest()
{
    while (!threadShouldExit())
    {
        // 6. wait for a closing request from the gui or the connected virtual studio
        wait(100);
    }

    m_providerThread->stopThread(1000);
    m_consumerThread->stopThread(1000);
};

void ConnectionManagerThread::closeProviderAndConsumerThreads()
{
    m_providerThread->waitForThreadToExit(1000);
    m_consumerThread->waitForThreadToExit(1000);
};
