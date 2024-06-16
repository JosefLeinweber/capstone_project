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
    if (m_host)
    {
        m_host->stopHost();
    }

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
    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }
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

    std::cout << "CMT::startUpProviderAndConsumerThreads" << std::endl;
    std::cout << "Provider address: " << providerAddress.ip << ":"
              << providerAddress.port << std::endl;
    std::cout << "Consumer address: " << consumerAddress.ip << ":"
              << consumerAddress.port << std::endl;
    std::cout << "Remote Consumer address: " << remoteConsumerAddress.ip << ":"
              << remoteConsumerAddress.port << std::endl;

    m_providerThread = std::make_unique<ProviderThread>(providerAddress,
                                                        remoteConsumerAddress,
                                                        m_outputRingBuffer,
                                                        m_isProviderConnected);
    m_consumerThread = std::make_unique<ConsumerThread>(consumerAddress,
                                                        m_inputRingBuffer,
                                                        m_isConsumerConnected);

    m_providerThread->startThread();
    m_consumerThread->startThread();


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

            m_providerThread->signalThreadShouldExit();
            m_consumerThread->signalThreadShouldExit();
            return false;
        }
        wait(1000);
    }
    return true;
}

void ConnectionManagerThread::onlyStartConsumerThread(
    addressData consumerAddress)
{
    AudioBufferFIFO remoteInputRingBuffer(2, 1024);

    std::atomic<bool> isConsumerConnected = false;
    ConsumerThread consumerThread(consumerAddress,
                                  remoteInputRingBuffer,
                                  isConsumerConnected,
                                  "ConsumerThread");
    consumerThread.startThread();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    consumerThread.signalThreadShouldExit();
    while (consumerThread.isThreadRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
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
