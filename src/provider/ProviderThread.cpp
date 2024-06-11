#include "ProviderThread.h"
#include <chrono>
#include <iostream>

ProviderThread::ProviderThread(addressData &hostAddress,
                               addressData &remoteAddress,
                               AudioBufferFIFO &outputRingBuffer,
                               std::atomic<bool> &isProviderConnected)
    : juce::Thread("Provider Thread"), m_hostAddress(hostAddress),
      m_remoteAddress(remoteAddress), m_outputRingBuffer(outputRingBuffer),
      m_isProviderConnected(isProviderConnected) {};

void ProviderThread::run()
{
    setupHost();
    m_isProviderConnected = validateConnection();
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
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    std::cout << "Provider sends handshake at: " << currentTime.count()
              << " seconds" << std::endl;
    bool connected = m_host->waitForHandshake();
    std::cout << "Provider recieved handshake == " << connected << std::endl;
    if (connected)
    {
        auto currentTime = std::chrono::system_clock::now().time_since_epoch();
        std::cout << "Provider recieved handshake at: " << currentTime.count()
                  << " seconds" << std::endl;
    }
    return connected;
}
