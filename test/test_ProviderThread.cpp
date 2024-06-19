#include "AudioBuffer.h"
#include "Host.h"
#include "ProviderThread.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>

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

void printBuffer(auto &buffer)
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


TEST_CASE("ProviderThread | Constructor")
{
    ConfigurationData remoteConfigurationData;
    ConfigurationData localConfigurationData;
    AudioBufferFIFO outputRingBuffer(2, 1024);

    try
    {
        ProviderThread providerThread(remoteConfigurationData,
                                      localConfigurationData,
                                      outputRingBuffer);
    }
    catch (...)
    {
        FAIL("No error should be thrown");
    }
}

TEST_CASE("ProviderThread | setupHost")
{
    ConfigurationData remoteConfigurationData;
    ConfigurationData localConfigurationData;
    localConfigurationData.set_provider_port(5000);
    AudioBufferFIFO outputRingBuffer(2, 1024);

    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);
    REQUIRE_NOTHROW(providerThread.setupHost());
}

TEST_CASE("ProviderThread | readFromFIFOBuffer")
{
    ConfigurationData remoteConfigurationData;
    ConfigurationData localConfigurationData;
    AudioBufferFIFO outputRingBuffer(2, 20);

    auto fillThread = std::jthread([&outputRingBuffer]() {
        juce::AudioBuffer<float> tempBuffer(2, 10);
        fillBuffer(tempBuffer, 0.5);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        outputRingBuffer.writeToInternalBufferFrom(tempBuffer);
    });

    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);
    REQUIRE_NOTHROW(providerThread.readFromFIFOBuffer());
    REQUIRE(outputRingBuffer.buffer.getSample(0, 0) == 0.5);
    printBuffer(outputRingBuffer.buffer);
    fillThread.join();
}

TEST_CASE("ProviderThread | readFromFIFOBuffer timeout")
{
    ConfigurationData remoteConfigurationData;
    ConfigurationData localConfigurationData;
    AudioBufferFIFO outputRingBuffer(2, 20);

    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);
    REQUIRE_FALSE(providerThread.readFromFIFOBuffer());
    printBuffer(outputRingBuffer.buffer);
}

TEST_CASE("ProviderThread | sendAudioToRemoteConsumer")
{
    ConfigurationData remoteConfigurationData;
    remoteConfigurationData.set_consumer_port(5001);
    remoteConfigurationData.set_ip("127.0.0.1");
    ConfigurationData localConfigurationData;
    localConfigurationData.set_provider_port(5000);
    AudioBufferFIFO outputRingBuffer(2, 20);


    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);

    providerThread.setupHost();
    REQUIRE(providerThread.sendAudioToRemoteConsumer());
    printBuffer(outputRingBuffer.buffer);
}
