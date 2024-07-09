/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ConnectDAWsAudioProcessor::ConnectDAWsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      m_visualiser(2), m_outputVisualiser(2)
#endif
{
    m_visualiser.setRepaintRate(30);
    m_visualiser.setBufferSize(448);

    m_outputVisualiser.setRepaintRate(30);
    m_outputVisualiser.setBufferSize(448);
}

ConnectDAWsAudioProcessor::~ConnectDAWsAudioProcessor()
{
    m_connectDAWs.releaseResources();
}

//==============================================================================
const juce::String ConnectDAWsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ConnectDAWsAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ConnectDAWsAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ConnectDAWsAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ConnectDAWsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ConnectDAWsAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int ConnectDAWsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ConnectDAWsAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String ConnectDAWsAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void ConnectDAWsAudioProcessor::changeProgramName(int index,
                                                  const juce::String &newName)
{
    juce::ignoreUnused(index);
    juce::ignoreUnused(newName);
}

//==============================================================================
void ConnectDAWsAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock)
{
    int numInputChannels = getTotalNumInputChannels();
    int numOutputChannels = getTotalNumOutputChannels();
    //TODO: change to start or update
    if (m_connectDAWs.m_connectionManagerThread == nullptr)
    {
        m_connectDAWs.startUpConnectionManagerThread(sampleRate,
                                                     samplesPerBlock,
                                                     numInputChannels,
                                                     numOutputChannels);
    }
}

void ConnectDAWsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ConnectDAWsAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void ConnectDAWsAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    m_visualiser.pushBuffer(buffer);

    m_connectDAWs.processBlock(buffer);

    m_outputVisualiser.pushBuffer(buffer);
}

//==============================================================================
bool ConnectDAWsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *ConnectDAWsAudioProcessor::createEditor()
{

    return new ConnectDAWsAudioProcessorEditor(*this,
                                               m_connectDAWs.m_guiMessenger,
                                               m_connectDAWs.m_cmtMessenger);
}

//==============================================================================
void ConnectDAWsAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::ignoreUnused(destData);
    // You should use this method to store your m_parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ConnectDAWsAudioProcessor::setStateInformation(const void *data,
                                                    int sizeInBytes)
{
    juce::ignoreUnused(data);
    juce::ignoreUnused(sizeInBytes);
    // You should use this method to restore your m_parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new ConnectDAWsAudioProcessor();
}
