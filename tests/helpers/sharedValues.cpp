#include "sharedValues.h"

ConfigurationData setConfigurationData(const std::string ip,
                                       int hostPort,
                                       int providerPort,
                                       int consumerPort,
                                       int sampleRate,
                                       int samplesPerBlock,
                                       int numInputChannels,
                                       int numOutputChannels)
{
    ConfigurationData configurationData;
    configurationData.set_ip(ip);
    configurationData.set_provider_port(providerPort);
    configurationData.set_consumer_port(consumerPort);
    configurationData.set_host_port(hostPort);
    configurationData.set_samples_per_block(samplesPerBlock);
    configurationData.set_sample_rate(sampleRate);
    configurationData.set_num_input_channels(numInputChannels);
    configurationData.set_num_output_channels(numOutputChannels);

    return configurationData;
}

void fillBuffer(juce::AudioBuffer<float> &buffer, float value)
{
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            buffer.setSample(channel, i, value);
        }
    }
}
void printBuffer(juce::AudioBuffer<float> &buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        std::cout << "Channel " << channel << ": ";
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            std::cout << buffer.getSample(channel, i) << " ";
        }
        std::cout << std::endl;
    }
}

ConfigurationData localConfigurationData =
    setConfigurationData("::1", 5000, 5001, 5002, 44100, 512, 2, 2);

ConfigurationData remoteConfigurationData =
    setConfigurationData("::1", 6000, 6001, 6002, 44100, 512, 2, 2);

RingBuffer inputRingBuffer(2, 1024);
RingBuffer outputRingBuffer(2, 1024);

RingBuffer remoteInputRingBuffer(2, 1024);
RingBuffer remoteOutputRingBuffer(2, 1024);

std::atomic<bool> startConnection = false;
std::atomic<bool> stopConnection = false;

auto placeHolderHandler = [](const juce::Message &message) {
    juce::ignoreUnused(message);
};

std::shared_ptr<Messenger> guiMessenger1 =
    std::make_shared<Messenger>(placeHolderHandler);
std::shared_ptr<Messenger> cmtMessenger1;

std::shared_ptr<Messenger> guiMessenger2 =
    std::make_shared<Messenger>(placeHolderHandler);
std::shared_ptr<Messenger> cmtMessenger2;

std::shared_ptr<Benchmark> benchmark1 = std::make_shared<Benchmark>();
std::shared_ptr<Benchmark> benchmark2 = std::make_shared<Benchmark>();


ConnectionManagerThread remoteConnectionManagerThread(guiMessenger1,
                                                      cmtMessenger1,
                                                      remoteConfigurationData,
                                                      remoteInputRingBuffer,
                                                      remoteOutputRingBuffer,
                                                      startConnection,
                                                      stopConnection,
                                                      benchmark1,
                                                      "RemoteCMT");

ConnectionManagerThread connectionManagerThread(guiMessenger2,
                                                cmtMessenger2,
                                                localConfigurationData,
                                                inputRingBuffer,
                                                outputRingBuffer,
                                                startConnection,
                                                stopConnection,
                                                benchmark2,
                                                "LocalCMT");

std::shared_ptr<std::vector<std::int64_t>> differenceBuffer =
    std::make_shared<std::vector<std::int64_t>>();
