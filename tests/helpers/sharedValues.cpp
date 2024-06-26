#include "sharedValues.h"

ConfigurationData setConfigurationData(const std::string ip,
                                       int hostPort,
                                       int providerPort,
                                       int consumerPort)
{
    ConfigurationData configurationData;
    configurationData.set_ip(ip);
    configurationData.set_provider_port(providerPort);
    configurationData.set_consumer_port(consumerPort);
    configurationData.set_host_port(hostPort);
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
    setConfigurationData("127.0.0.1", 5000, 5001, 5002);

ConfigurationData remoteConfigurationData =
    setConfigurationData("127.0.0.1", 6000, 6001, 6002);
