#include "ProviderThread.h"
#include <chrono>
#include <iostream>

ProviderThread::ProviderThread(addressData &hostAddress,
                               addressData &remoteAddress,
                               AudioBufferFIFO &outputRingBuffer,
                               std::atomic<bool> &isProviderConnected)
    : juce::Thread("Provider Thread"), m_hostAddress(hostAddress),
      m_remoteAddress(remoteAddress), m_outputRingBuffer(outputRingBuffer),
      m_isProviderConnected(isProviderConnected)
{
    std::cout << "hostAddress: " << hostAddress.ip << ":" << hostAddress.port
              << "  m_hostAddress: " << m_hostAddress.ip << ":"
              << m_hostAddress.port << std::endl;
    std::cout << remoteAddress.ip << ":" << remoteAddress.port
              << "  m_remoteAddress: " << m_remoteAddress.ip << ":"
              << m_remoteAddress.port << std::endl;
};

ProviderThread::~ProviderThread()
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
    std::cout << "ProviderThread | Destructor" << std::endl;
}


void ProviderThread::run()
{
    setupHost();
    m_isProviderConnected = validateConnection();
    std::cout << "Provider connected: " << m_isProviderConnected << std::endl;
    return;
};

void ProviderThread::startSendingAudio()
{
    while (!threadShouldExit())
    {
        m_outputRingBuffer.readFromBuffer(m_outputBuffer);
        m_host->sendTo(m_outputBuffer);
    }
};

void ProviderThread::setupHost()
{
    m_host = std::make_unique<Host>(m_hostAddress);
    m_host->setupSocket();
}

bool ProviderThread::validateConnection()
{
    m_host->sendHandshake(m_remoteAddress);
    bool connected = m_host->waitForHandshake();
    return connected;
}
