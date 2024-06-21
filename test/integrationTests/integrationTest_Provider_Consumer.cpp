#include "AudioBuffer.h"
#include "ConsumerThread.h"
#include "ProviderThread.h"
#include "datagram.pb.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>

ConfigurationData localConfigurationData =
    setConfigurationData("127.0.0.1", 5000, 5001, 5002);

ConfigurationData remoteConfigurationData =
    setConfigurationData("127.0.0.1", 6000, 6001, 6002);


TEST_CASE("Provider & Consumer Thread | send and receive audio buffer")
{
    ConfigurationData providerConfigurationData = localConfigurationData;
    ConfigurationData consumerConfigurationData = remoteConfigurationData;

    juce::AudioBuffer<float> tempBuffer(2, 20);
    fillBuffer(tempBuffer, 0.5);

    AudioBufferFIFO outputRingBuffer(2, 80);
    outputRingBuffer.writeToInternalBufferFrom(tempBuffer);


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
