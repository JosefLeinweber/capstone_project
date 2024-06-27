/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once


#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include "audioBuffer.h"
#include "connectionManagerThread.h"

//==============================================================================
/**
*/
class LowpassHighpassFilterAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    LowpassHighpassFilterAudioProcessor();
    ~LowpassHighpassFilterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    // void connectToClient();
    juce::AudioVisualiserComponent visualiser;
    void sendToConnectionManagerThread(const std::string &ip, int port);
    void sendToPluginEditor(const std::string &ip, int port);

private:
    std::atomic<bool> startConnection = false;
    std::atomic<bool> stopConnection = false;
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> *cutoffFrequencyParameter = nullptr;
    std::atomic<float> *highpassParameter = nullptr;
    std::unique_ptr<ConnectionManagerThread> connectionManagerThread;
    std::shared_ptr<AudioBufferFIFO> outputBufferFIFO;
    std::shared_ptr<AudioBufferFIFO> inputBufferFIFO;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        LowpassHighpassFilterAudioProcessor)
};
