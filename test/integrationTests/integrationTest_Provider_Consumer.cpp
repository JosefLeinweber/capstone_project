#include "AudioBuffer.h"
#include "ConsumerThread.h"
#include "ProviderThread.h"
#include "datagram.pb.h"
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

TEST_CASE("Provider & Consumer Thread | send and receive audio buffer")
{
    juce::AudioBuffer<float> tempBuffer(2, 20);
    fillBuffer(tempBuffer, 0.5);

    ConfigurationData providerConfigurationData;
    providerConfigurationData.set_provider_port(8001);
    providerConfigurationData.set_consumer_port(8002);
    providerConfigurationData.set_ip("127.0.0.1");
    AudioBufferFIFO outputRingBuffer(2, 80);
    outputRingBuffer.writeToInternalBufferFrom(tempBuffer);


    ConfigurationData consumerConfigurationData;
    consumerConfigurationData.set_provider_port(8003);
    consumerConfigurationData.set_consumer_port(8004);
    consumerConfigurationData.set_ip("127.0.0.1");
    AudioBufferFIFO inputRingBuffer(2, 80);


    ProviderThread providerThread(consumerConfigurationData,
                                  providerConfigurationData,
                                  outputRingBuffer);


    ConsumerThread consumerThread(providerConfigurationData,
                                  consumerConfigurationData,
                                  inputRingBuffer);

    providerThread.startThread();
    consumerThread.startThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    providerThread.signalThreadShouldExit();
    consumerThread.signalThreadShouldExit();
    providerThread.waitForThreadToExit(1000);
    consumerThread.waitForThreadToExit(1000);
    std::cout << "After threads closed\n";
    std::cout << "providerThread.m_outputBuffer\n";
    printBuffer(providerThread.m_outputBuffer);
    std::cout << "consumerThread.m_inputBuffer\n";
    printBuffer(consumerThread.m_inputBuffer);
    std::cout << "providerThread.m_outputRingBuffer.buffer\n";
    printBuffer(providerThread.m_outputRingBuffer.buffer);
    std::cout << "consumerThread.m_inputRingBuffer.buffer\n";
    printBuffer(consumerThread.m_inputRingBuffer.buffer);
    bool sendAndReceivedDataIsEqual = false;
    for (int channel = 0; channel < tempBuffer.getNumChannels(); channel++)
    {
        for (int i = 0; i < tempBuffer.getNumSamples(); i++)
        {
            if (tempBuffer.getSample(channel, i) !=
                consumerThread.m_inputBuffer.getSample(channel, i))
            {
                sendAndReceivedDataIsEqual = false;
                break;
            }
            sendAndReceivedDataIsEqual = true;
        }
    }
    REQUIRE(sendAndReceivedDataIsEqual);
}
