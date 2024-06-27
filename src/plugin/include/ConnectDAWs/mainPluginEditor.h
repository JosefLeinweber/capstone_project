/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>

#include "mainPluginProcessor.h"
#include "messenger.h"

//==============================================================================
/**
*/
class MainAudioProcessorEditor : public juce::AudioProcessorEditor,
                                 public juce::Button::Listener
{
public:
    MainAudioProcessorEditor(MainAudioProcessor &,
                             juce::AudioProcessorValueTreeState &vts,
                             std::shared_ptr<Messenger> &guiMessenger,
                             std::shared_ptr<Messenger> &cmtMessenger);
    ~MainAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;
    void buttonClicked(juce::Button *button) override;
    void handleMessage(const juce::Message &message);
    void initGUIMessenger();

private:
    void sendMessageToCMT(juce::Message *message);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MainAudioProcessor &audioProcessor;
    std::shared_ptr<Messenger> &m_guiMessenger;
    std::shared_ptr<Messenger> &m_cmtMessenger;

    juce::TextEditor ipEditor;
    juce::TextEditor portEditor;
    juce::TextButton sendButton{"Send IP and Port"};
    juce::Label ipLabel{"IP", "IP Address:"};
    juce::Label portLabel{"Port", "Port:"};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainAudioProcessorEditor)
};
