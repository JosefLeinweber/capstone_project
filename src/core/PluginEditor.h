/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "ConnectDAWs/connectDAWS.h"
#include "ConnectDAWsComponents/mainComponent.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class MainAudioProcessorEditor : public juce::AudioProcessorEditor
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

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MainAudioProcessor &audioProcessor;
    MainComponent m_mainComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainAudioProcessorEditor)
};
