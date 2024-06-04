/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "YourPluginName/PluginProcessor.h"
#include "YourPluginName/PluginEditor.h"




//==============================================================================
LowpassHighpassFilterAudioProcessorEditor::LowpassHighpassFilterAudioProcessorEditor (LowpassHighpassFilterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    constexpr auto HEIGHT = 500;
    constexpr auto WIDTH = 500;

    addAndMakeVisible(cutoffFrequencySlider);
    cutoffFrequencySlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    cutoffFrequencyAttachment.reset(
        new juce::AudioProcessorValueTreeState::SliderAttachment(
            vts, "cutoff_frequency", cutoffFrequencySlider));
    
    addAndMakeVisible(cutoffFrequencyLabel);
    cutoffFrequencyLabel.setText("RANDOM SHIT", juce::dontSendNotification);

    addAndMakeVisible(highpassButton);
    highpassAttachment.reset(
        new juce::AudioProcessorValueTreeState::ButtonAttachment(
            vts, "highpass", highpassButton));

    addAndMakeVisible(highpassButtonLabel);
    highpassButtonLabel.setText("Highpass", juce::dontSendNotification);

    addAndMakeVisible(networkButton);
    networkAttachment.reset(
        new juce::AudioProcessorValueTreeState::ButtonAttachment(
            vts, "network", networkButton));
    
    addAndMakeVisible(networkButtonLabel);
    networkButtonLabel.setText("Network Button", juce::dontSendNotification);

    addAndMakeVisible(audioProcessor.visualiser);
    audioProcessor.visualiser.setColours(juce::Colours::black, juce::Colours::whitesmoke.withAlpha(0.5f));

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (WIDTH, HEIGHT);

}

LowpassHighpassFilterAudioProcessorEditor::~LowpassHighpassFilterAudioProcessorEditor()
{
}

//==============================================================================
void LowpassHighpassFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    cutoffFrequencySlider.setBounds({ 15, 35, 100, 300 });
    cutoffFrequencyLabel.setBounds({ cutoffFrequencySlider.getX() + 30, cutoffFrequencySlider.getY() - 30, 
        200, 50 });
    highpassButton.setBounds({ cutoffFrequencySlider.getX(), 
        cutoffFrequencySlider.getY() + cutoffFrequencySlider.getHeight() + 15, 30, 50 });
    highpassButtonLabel.setBounds({ cutoffFrequencySlider.getX() + highpassButton.getWidth() + 15, highpassButton.getY(), 
        cutoffFrequencySlider.getWidth() - highpassButton.getWidth(), 
        highpassButton.getHeight() });

    audioProcessor.visualiser.setBounds(getLocalBounds().withSizeKeepingCentre(250, 250));
}

void LowpassHighpassFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
