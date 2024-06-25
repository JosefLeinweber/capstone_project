/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "YourPluginName/PluginProcessor.h"
#include "YourPluginName/PluginEditor.h"


//==============================================================================
LowpassHighpassFilterAudioProcessor::LowpassHighpassFilterAudioProcessor()
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
      visualiser(2),
      parameters(*this,
                 nullptr,
                 juce::Identifier("LowpassAndHighpassPlugin"),
                 {std::make_unique<juce::AudioParameterFloat>(
                      "cutoff_frequency",
                      "Cutoff Frequency",
                      juce::NormalisableRange{20.f, 20000.f, 0.1f, 0.2f, false},
                      500.f),
                  std::make_unique<juce::AudioParameterBool>("highpass",
                                                             "Highpass",
                                                             false)})
#endif
{
    visualiser.setRepaintRate(30);
    visualiser.setBufferSize(256);

    cutoffFrequencyParameter =
        parameters.getRawParameterValue("cutoff_frequency");
    highpassParameter = parameters.getRawParameterValue("highpass");
    inputBufferFIFO = std::make_shared<AudioBufferFIFO>(2, 100000);
    outputBufferFIFO = std::make_shared<AudioBufferFIFO>(2, 100000);
    ConfigurationData localConfigurationData;
    localConfigurationData.set_ip("127.0.0.1");
    localConfigurationData.set_host_port(7000);
    localConfigurationData.set_consumer_port(7001);
    localConfigurationData.set_provider_port(7002);
    connectionManagerThread =
        std::make_unique<ConnectionManagerThread>(localConfigurationData,
                                                  *inputBufferFIFO,
                                                  *outputBufferFIFO,
                                                  startConnection,
                                                  stopConnection);
    connectionManagerThread->setAudioProcessor(this);
    connectionManagerThread->startThread();
}

LowpassHighpassFilterAudioProcessor::~LowpassHighpassFilterAudioProcessor()
{
    connectionManagerThread->signalThreadShouldExit();
    connectionManagerThread->waitForThreadToExit(1000);
}

//==============================================================================
const juce::String LowpassHighpassFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LowpassHighpassFilterAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool LowpassHighpassFilterAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool LowpassHighpassFilterAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double LowpassHighpassFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LowpassHighpassFilterAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int LowpassHighpassFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LowpassHighpassFilterAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String LowpassHighpassFilterAudioProcessor::getProgramName(
    int index)
{
    juce::ignoreUnused(index);
    return {};
}

void LowpassHighpassFilterAudioProcessor::changeProgramName(
    int index,
    const juce::String &newName)
{
    juce::ignoreUnused(index);
    juce::ignoreUnused(newName);
}

//==============================================================================
void LowpassHighpassFilterAudioProcessor::prepareToPlay(double sampleRate,
                                                        int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused(samplesPerBlock);

    //TODO: make the init arguments dependent on plugin parameters
}

void LowpassHighpassFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LowpassHighpassFilterAudioProcessor::isBusesLayoutSupported(
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

void LowpassHighpassFilterAudioProcessor::processBlock(
    juce::AudioBuffer<float> &buffer,
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

    // filter.processBlock(buffer, midiMessages);
    visualiser.pushBuffer(buffer);
}

//==============================================================================
bool LowpassHighpassFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *LowpassHighpassFilterAudioProcessor::createEditor()
{
    //TODO: why can I pass *connectionManagerThread here?
    return new LowpassHighpassFilterAudioProcessorEditor(*this, parameters);
}

//==============================================================================
void LowpassHighpassFilterAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData)
{
    juce::ignoreUnused(destData);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void LowpassHighpassFilterAudioProcessor::setStateInformation(const void *data,
                                                              int sizeInBytes)
{
    juce::ignoreUnused(data);
    juce::ignoreUnused(sizeInBytes);
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new LowpassHighpassFilterAudioProcessor();
}


void LowpassHighpassFilterAudioProcessor::sendToConnectionManagerThread(
    const std::string &ip,
    int port)
{
    std::cout << "Current Thread ID 1: " << std::this_thread::get_id()
              << std::endl;
    juce::MessageManager::callAsync([this, ip, port]() {
        std::cout << "Current Thread ID 2: " << std::this_thread::get_id()
                  << std::endl;
        MyCustomMessage *message = new MyCustomMessage(ip, port);
        connectionManagerThread->postMessage(message);
    });
}

void LowpassHighpassFilterAudioProcessor::sendToPluginEditor(
    const std::string &ip,
    int port)
{
    juce::MessageManager::callAsync([this, ip, port]() {
        MyCustomMessage *message = new MyCustomMessage(ip, port);
        dynamic_cast<LowpassHighpassFilterAudioProcessorEditor *>(
            getActiveEditor())
            ->postMessage(message);
    });
}
