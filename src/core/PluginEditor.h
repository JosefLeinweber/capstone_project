/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include "ConnectDAWs/connectDAWS.h"
#include "ConnectDAWsComponents/connectDAWsComponent.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ConnectDAWsAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ConnectDAWsAudioProcessorEditor(ConnectDAWsAudioProcessor &,
                                    std::shared_ptr<Messenger> &guiMessenger,
                                    std::shared_ptr<Messenger> &cmtMessenger);
    ~ConnectDAWsAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::Label m_outputVisualiserLabel{"Output Visualiser",
                                        "Output Visualiser"};
    juce::Label m_inputVisualiserLabel{"Input Visualiser", "Input Visualiser"};
    ConnectDAWsAudioProcessor &m_audioProcessor;
    ConnectDAWsComponent m_connectDAWsComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ConnectDAWsAudioProcessorEditor)
};
