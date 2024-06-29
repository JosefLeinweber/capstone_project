/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "ConnectDAWs/mainPluginProcessor.h"
#include "ConnectDAWs/mainPluginEditor.h"

//==============================================================================
MainAudioProcessor::MainAudioProcessor()
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
      visualiser(2), outputVisualiser(2),
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
    visualiser.setBufferSize(448);

    outputVisualiser.setRepaintRate(30);
    outputVisualiser.setBufferSize(448);

    cutoffFrequencyParameter =
        parameters.getRawParameterValue("cutoff_frequency");
    highpassParameter = parameters.getRawParameterValue("highpass");
}

MainAudioProcessor::~MainAudioProcessor()
{
    // connectionManagerThread->signalThreadShouldExit();
    // connectionManagerThread->waitForThreadToExit(1000);
}

//==============================================================================
const juce::String MainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MainAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool MainAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool MainAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double MainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MainAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int MainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MainAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String MainAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void MainAudioProcessor::changeProgramName(int index,
                                           const juce::String &newName)
{
    juce::ignoreUnused(index);
    juce::ignoreUnused(newName);
}

//==============================================================================
void MainAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int numInputChannels = getTotalNumInputChannels();
    int numOutputChannels = getTotalNumOutputChannels();
    connectDAWs.prepareToPlay(sampleRate,
                              samplesPerBlock,
                              numInputChannels,
                              numOutputChannels);

    std::cout << "Sample Rate: " << sampleRate << std::endl;
}

void MainAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    connectDAWs.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MainAudioProcessor::isBusesLayoutSupported(
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

void MainAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
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
    // for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    //     buffer.clear(i, 0, buffer.getNumSamples());

    // filter.processBlock(buffer, midiMessages);
    visualiser.pushBuffer(buffer);

    connectDAWs.processBlock(buffer);

    outputVisualiser.pushBuffer(buffer);
}

//==============================================================================
bool MainAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *MainAudioProcessor::createEditor()
{

    return new MainAudioProcessorEditor(*this,
                                        parameters,
                                        connectDAWs.m_guiMessenger,
                                        connectDAWs.m_cmtMessenger);
}

//==============================================================================
void MainAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::ignoreUnused(destData);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MainAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
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
    return new MainAudioProcessor();
}


// void MainAudioProcessor::sendToConnectionManagerThread(const std::string &ip,
//                                                        int port)
// {
//     std::cout << "Current Thread ID 1: " << std::this_thread::get_id()
//               << std::endl;
//     juce::MessageManager::callAsync([this, ip, port]() {
//         std::cout << "Current Thread ID 2: " << std::this_thread::get_id()
//                   << std::endl;
//         MyCustomMessage *message = new MyCustomMessage(ip, port);
//         connectionManagerThread->postMessage(message);
//     });
// }

// void MainAudioProcessor::sendToPluginEditor(const std::string &ip, int port)
// {
//     juce::MessageManager::callAsync([this, ip, port]() {
//         MyCustomMessage *message = new MyCustomMessage(ip, port);
//         dynamic_cast<MainAudioProcessorEditor *>(getActiveEditor())
//             ->postMessage(message);
//     });
// }
