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

#include "ConnectionManagerThread.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class LowpassHighpassFilterAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      public juce::Button::Listener
{
public:
    LowpassHighpassFilterAudioProcessorEditor(
        LowpassHighpassFilterAudioProcessor &,
        juce::AudioProcessorValueTreeState &vts);
    ~LowpassHighpassFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;
    void buttonClicked(juce::Button *button) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LowpassHighpassFilterAudioProcessor &audioProcessor;

    juce::TextEditor ipEditor;
    juce::TextEditor portEditor;
    juce::TextButton sendButton{"Send IP and Port"};
    juce::Label ipLabel{"IP", "IP Address:"};
    juce::Label portLabel{"Port", "Port:"};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        LowpassHighpassFilterAudioProcessorEditor)
};
