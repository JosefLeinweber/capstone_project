#include "ConnectDAWs/audioBuffer.h"
#include "ConnectDAWs/consumerThread.h"
#include "ConnectDAWs/providerThread.h"
#include "datagram.pb.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>


TEST_CASE("Provider & Consumer Thread | send and receive audio buffer")
{
    ConfigurationData providerConfigurationData = localConfigurationData;
    ConfigurationData consumerConfigurationData = remoteConfigurationData;

    juce::AudioBuffer<float> tempBuffer(2, 512);
    fillBuffer(tempBuffer, 0.5);

    outputRingBuffer.writeToInternalBufferFrom(tempBuffer);


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