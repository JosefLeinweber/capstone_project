#include "ConsumerThread.h"
#include <iostream>
#include <thread>

ConsumerThread::ConsumerThread(addressData &hostAddress,
                               AudioBufferFIFO &inputRingBuffer,
                               std::atomic<bool> &isConsumerConnected,
                               const std::string threadName)

    : juce::Thread(threadName), m_inputRingBuffer(inputRingBuffer),
      m_isConsumerConnected(isConsumerConnected), m_hostAddress(hostAddress) {};

ConsumerThread::~ConsumerThread()
{
    if (m_host)
    {
        m_host->stopHost();
    }

    if (isThreadRunning())
    {
        signalThreadShouldExit();
        waitForThreadToExit(1000);
    }
    std::cout << "ConsumerThread | Destructor" << std::endl;
}

void ConsumerThread::run()
{
    while (!threadShouldExit())
    {
        setupHost();
        m_isConsumerConnected = validateConnection();
        return;
    }
};

void ConsumerThread::setupHost()
{
    m_host = std::make_unique<Host>(m_hostAddress);
    m_host->setupSocket();
};

void ConsumerThread::startRecievingAudio()
{
    while (!threadShouldExit())
    {
        m_host->recieveFrom(m_inputBuffer);
        m_inputRingBuffer.writeToBuffer(m_inputBuffer);
    }
};

bool ConsumerThread::validateConnection()
{
    bool recievedHandshake = m_host->waitForHandshake();
    bool connected = false;
    if (recievedHandshake)
    {
        std::cout << "ConsumerThread | Recieved Handshake" << std::endl;
        m_host->sendHandshake(m_host->getRemoteAddress());
        connected = true;
    }
    else
    {
        std::cout << "ConsumerThread | Failed to Recieve Handshake"
                  << std::endl;
        connected = false;
    }
    return connected;
};
