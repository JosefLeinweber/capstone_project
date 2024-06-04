/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 

#include "PluginProcessor.h"

//==============================================================================
/**
*/
class LowpassHighpassFilterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    LowpassHighpassFilterAudioProcessorEditor (LowpassHighpassFilterAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~LowpassHighpassFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LowpassHighpassFilterAudioProcessor& audioProcessor;

    juce::Slider cutoffFrequencySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        cutoffFrequencyAttachment;
    juce::Label cutoffFrequencyLabel;

    juce::ToggleButton highpassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        highpassAttachment;
    juce::Label highpassButtonLabel;

    juce::ToggleButton networkButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        networkAttachment;
    juce::Label networkButtonLabel;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowpassHighpassFilterAudioProcessorEditor)
};
