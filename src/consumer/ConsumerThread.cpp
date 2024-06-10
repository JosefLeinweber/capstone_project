#include "ConsumerThread.h"

ConsumerThread::ConsumerThread(AudioBufferFIFO& inputRingBuffer, std::atomic<bool>& isConsumerConnected, std::string host, int port) : 
    juce::Thread("Consumer Thread"), m_inputRingBuffer(inputRingBuffer), m_isConnected(isConsumerConnected), m_hostIp(host), m_hostPort(port) {};

void ConsumerThread::run() {
    std::make_unique<Host> m_host(m_hostIp, m_hostPort);

    while (!m_host->isConnected()) {
        m_host->waitForHandshake();
        m_host->();
    }

    startRecievingAudio();
};

void ConsumerThread::startRecievingAudio() {
    while (!threadShouldExit()) {
        m_server.receiveAudioData(m_inputBuffer);
        m_inputRingBuffer.writeToBuffer(m_inputBuffer);
    }
}
