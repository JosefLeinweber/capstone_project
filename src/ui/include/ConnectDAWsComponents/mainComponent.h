#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ConnectDAWs/messenger.h"

class MainComponent : public juce::Component, public juce::Button::Listener
{
public:
    MainComponent(std::shared_ptr<Messenger> &guiMessenger,
                  std::shared_ptr<Messenger> &cmtMessenger);
    ~MainComponent();

    void paint(juce::Graphics &) override;
    void resized() override;

    void buttonClicked(juce::Button *button) override;
    void handleMessage(const juce::Message &message);
    void initGUIMessenger();

private:
    std::shared_ptr<Messenger> &m_guiMessenger;
    std::shared_ptr<Messenger> &m_cmtMessenger;

    juce::TextButton stopButton{"Stop"};
    juce::TextEditor ipEditor;
    juce::TextEditor portEditor;
    juce::TextButton sendButton{"Send IP and Port"};
    juce::Label ipLabel{"IP", "IP Address:"};
    juce::Label portLabel{"Port", "Port:"};
    juce::Label statusLabel{"Status", "Status:"};
    juce::Label localIpAndPortLabel{"Local", "Local IP and Port:"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
