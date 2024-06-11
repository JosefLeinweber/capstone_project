#include "ConsumerThread.h"
#include <iostream>

ConsumerThread::ConsumerThread(
  AudioBufferFIFO& inputRingBuffer,
  std::atomic<bool>& isConsumerConnected,
  addressData& hostAddress
) : juce::Thread("Consumer Thread"),
    m_inputRingBuffer(inputRingBuffer),
    m_isConsumerConnected(isConsumerConnected),
    m_hostAddress(hostAddress) {};

void ConsumerThread::run() {
    setupHost();

    m_isConsumerConnected = validateConnection();

};

void ConsumerThread::setupHost() {
    m_host = std::make_unique<Host>(m_hostAddress);
};

void ConsumerThread::startRecievingAudio() {
    while (!threadShouldExit()) {
        m_host->recieveFrom(m_inputBuffer);
        m_inputRingBuffer.writeToBuffer(m_inputBuffer);

    }
};

bool ConsumerThread::validateConnection() {
    bool recievedHandshake = m_host->waitForHandshake();
    bool connected = false;
    if (recievedHandshake){
        m_host->sendHandshake(m_host->getRemoteAddress());
        connected = true;
    }
    else {
        connected = false;
    }
    return connected;
};
