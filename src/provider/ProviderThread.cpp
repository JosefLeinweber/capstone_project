#include "ProviderThread.h"

ProviderThread::ProviderThread( addressData& hostAddress, 
                                addressData& remoteAddress,
                                AudioBufferFIFO& outputRingBuffer, 
                                std::atomic<bool>& isProviderConnected) : 
                                juce::Thread("Provider Thread"), 
                                m_hostAddress(hostAddress),
                                m_remoteAddress(remoteAddress),
                                m_outputRingBuffer(outputRingBuffer), 
                                m_isProviderConnected(isProviderConnected) {};

void ProviderThread::run() {
    setupHost();

    m_isProviderConnected = validateConnection();
};

void ProviderThread::startSendingAudio() {
    while (!threadShouldExit()) {
        m_outputRingBuffer.readFromBuffer(m_outputBuffer);
        m_host->sendTo(m_outputBuffer);
    }
};

void ProviderThread::setupHost() {
    m_host = std::make_unique<Host>(m_hostAddress);
}

bool ProviderThread::validateConnection() {

    m_host->sendHandshake(m_remoteAddress);
    return m_host->waitForHandshake();

}