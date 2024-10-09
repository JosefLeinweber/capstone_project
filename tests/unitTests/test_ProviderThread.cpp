#include "ConnectDAWs/providerThread.h"
#include "ConnectDAWs/ringBuffer.h"
#include "ConnectDAWs/udpHost.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>


TEST_CASE("ProviderThread | Constructor")
{


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


    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);
    REQUIRE_NOTHROW(providerThread.setupHost());
}

TEST_CASE("ProviderThread | readFromRingBuffer")
{


    auto fillThread = std::jthread([&]() {
        juce::AudioBuffer<float> tempBuffer(2, 10);
        fillBuffer(tempBuffer, 0.5);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        outputRingBuffer.copyFrom(tempBuffer);
    });

    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);
    REQUIRE_NOTHROW(providerThread.readFromRingBuffer());
    REQUIRE(outputRingBuffer.m_buffer.getSample(0, 0) == 0.5);
    printBuffer(outputRingBuffer.m_buffer);
    fillThread.join();
}

TEST_CASE("ProviderThread | readFromRingBuffer timeout")
{


    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);
    REQUIRE_FALSE(providerThread.readFromRingBuffer());
    printBuffer(outputRingBuffer.m_buffer);
}

TEST_CASE("ProviderThread | sendAudioToRemoteConsumer")
{


    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);

    providerThread.setupHost();
    REQUIRE(providerThread.sendAudioToRemoteConsumer());
    printBuffer(outputRingBuffer.m_buffer);
}
