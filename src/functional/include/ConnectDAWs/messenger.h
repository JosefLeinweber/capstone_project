#pragma once
#include <functional>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

class MessageToGUI : public juce::Message
{
public:
    MessageToGUI(const std::string &messageType, const std::string &message);

    std::string m_messageType;
    std::string m_message;
};

class MessageToCMT : public juce::Message
{
public:
    MessageToCMT(const std::string &messageType,
                 const std::string &ipAddress,
                 int port);

    std::string m_messageType;
    std::string m_ipAddress;
    int m_port;
};

class MessageToCMT : public juce::Message
{
public:
    MessageToCMT(const std::string &messageType,
                 const std::string &ipAddress,
                 int port);

    std::string m_messageType;
    std::string m_message
};


class Messenger : public juce::MessageListener
{
public:
    Messenger(std::function<void(const juce::Message &message)>
                  handleMessageFunction);
    ~Messenger();

    void handleMessage(const juce::Message &message) override;

    void sendMessage();

private:
    std::function<void(const juce::Message &message)> m_handleMessageFunction;
};
