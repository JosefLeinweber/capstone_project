#include "AudioBuffer.h"
#include "ConsumerThread.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
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

TEST_CASE("ConsumerThread | Constructor", "[ConsumerThread]")
{
    ConfigurationData remoteConfigurationData;
    ConfigurationData localConfigurationData;
    AudioBufferFIFO inputRingBuffer(2, 1024);

    try
    {
        ConsumerThread consumerThread(remoteConfigurationData,
                                      localConfigurationData,
                                      inputRingBuffer);
    }
    catch (...)
    {
        FAIL("No error should be thrown");
    }
}

TEST_CASE("ConsumerThread | setupHost")
{
    ConfigurationData remoteConfigurationData;
    ConfigurationData localConfigurationData;
    localConfigurationData.set_consumer_port(5000);
    AudioBufferFIFO inputRingBuffer(2, 1024);

    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);
    REQUIRE_NOTHROW(consumerThread.setupHost());
}

TEST_CASE("ConsumerThread | writeToFIFOBuffer")
{
    ConfigurationData remoteConfigurationData;
    ConfigurationData localConfigurationData;
    AudioBufferFIFO inputRingBuffer(2, 1024);


    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);

    fillBuffer(consumerThread.m_inputBuffer, 1.0f);
    REQUIRE_NOTHROW(consumerThread.writeToFIFOBuffer());
    REQUIRE(consumerThread.m_inputRingBuffer.buffer.getSample(0, 0) == 1.0f);
}

TEST_CASE("ConsumerThread | receiveAudioFromRemoteProvider")
{
    ConfigurationData remoteConfigurationData;
    remoteConfigurationData.set_consumer_port(5001);
    remoteConfigurationData.set_ip("127.0.0.1");
    ConfigurationData localConfigurationData;
    localConfigurationData.set_consumer_port(5000);
    AudioBufferFIFO inputRingBuffer(2, 20);

    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);
    consumerThread.setupHost();
    REQUIRE_FALSE(consumerThread.receiveAudioFromRemoteProvider());
}
