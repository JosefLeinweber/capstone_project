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
    std::cout << "ConnectionManagerThread Destructor" << std::endl;
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

        while (!m_incomingConnection /** or a event from gui happens */)
        {
            wait(100);
        }

        if (validateConnection() &&
            startUpProviderAndConsumerThreads(consumerAddress,
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
        try
        {
            std::cout
                << "ConnectionManager::validateConnection getRemoteAddress: "
                << m_host->getRemoteAddress().ip << ":"
                << m_host->getRemoteAddress().port << std::endl;
            m_host->sendHandshake(m_host->getRemoteAddress());
            std::cout << "ConnectionManager::validateConnection sendHandshake"
                      << std::endl;
            connected = m_host->waitForHandshake();
        }
        catch (std::exception &e)
        {
            std::cerr << "ConnectionManager::validateConnection exception: "
                      << e.what() << std::endl;
        }
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

bool ConnectionManagerThread::startUpProviderAndConsumerThreads(
    addressData providerAddress,
    addressData consumerAddress,
    addressData remoteConsumerAddress)
{
    //TODO: implement a way to get the provider and consumer address from the handshake

    m_providerThread = std::make_unique<ProviderThread>(providerAddress,
                                                        remoteConsumerAddress,
                                                        m_outputRingBuffer,
                                                        m_isProviderConnected);
    m_consumerThread = std::make_unique<ConsumerThread>(consumerAddress,
                                                        m_inputRingBuffer,
                                                        m_isConsumerConnected);
    m_consumerThread->startThread();
    m_providerThread->startThread();


    std::cout << "are threads running : " << m_providerThread->isThreadRunning()
              << " " << m_consumerThread->isThreadRunning() << std::endl;

    //time now
    auto startTime = std::chrono::high_resolution_clock::now();

    while (!m_isProviderConnected || !m_isConsumerConnected)
    {
        //TODO: make timeout not hardcoded!
        if (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - startTime)
                .count() > 5)
        {
            std::cout << "ConnectionManager::startUpProviderAndConsumerThreads "
                         "timeout"
                      << std::endl;
            std::cout << " CM Provider connected: " << m_isProviderConnected
                      << std::endl;
            std::cout << " CM Consumer connected: " << m_isConsumerConnected
                      << std::endl;

            m_providerThread->waitForThreadToExit(1000);
            m_consumerThread->waitForThreadToExit(1000);
            return false;
        }
        wait(1000);
    }
    return true;
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
