#pragma once

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ConnectDAWs/messenger.h"
#include "ConnectDAWsComponents/inConnectionComponent.h"
#include "ConnectDAWsComponents/startConnectionComponent.h"

class ConnectDAWsComponent : public juce::Component
{
public:
    ConnectDAWsComponent(std::shared_ptr<Messenger> &guiMessenger,
                         std::shared_ptr<Messenger> &cmtMessenger,
                         std::function<void()> forceResizeCallback);
    ~ConnectDAWsComponent();

    void paint(juce::Graphics &) override;
    void resized() override;

    void buttonClickedCallback(juce::Button *button, bool success);
    void handleMessage(const juce::Message &message);
    void initGUIMessenger();
    bool m_isConnected = false;

private:
    void sendStatusMessageToCMT(std::string type, std::string message);
    void sendAddressMessageToCMT(std::string ip, int port);
    void updateComponentVisibility(bool isConnected);

    std::shared_ptr<Messenger> &m_guiMessenger;
    std::shared_ptr<Messenger> &m_cmtMessenger;
    std::function<void()> m_forceResizeCallback;
    StartConnectionComponent m_startConnectionComponent{
        [this](juce::Button *button, bool success) {
            buttonClickedCallback(button, success);
        }};

    InConnectionComponent m_inConnectionComponent{
        [this](juce::Button *button, bool success) {
            buttonClickedCallback(button, success);
        }};
    juce::Label m_statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectDAWsComponent)
};
