#include "ConnectDAWs/messenger.h"


Messenger::Messenger(
    std::function<void(const juce::Message &message)> handleMessageFunction)
    : m_handleMessageFunction(handleMessageFunction)
{
}

Messenger::~Messenger()
{
}

void Messenger::handleMessage(const juce::Message &message)
{
    m_handleMessageFunction(message);
}


void Messenger::sendMessage()
{
    //TODO: Implement this function to reduce doublication of code
    throw std::runtime_error("Not implemented");
}
