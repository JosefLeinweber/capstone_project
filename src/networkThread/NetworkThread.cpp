#include "NetworkThread.h"

NetworkThread::NetworkThread(addressData hostAddress) : 
    juce::Thread("Consumer Thread"), m_hostAddress(hostAddress) {};

void NetworkThread::setupHost() {
    std::make_unique<Host> m_host(m_hostAddress);
};

void NetworkThread::receiveHandshake() {
    m_host->waitForHandshake();
};

void NetworkThread::sendHandshake(addressData remoteAddress) {
    m_host->sendHandshake(remoteAddress);
}