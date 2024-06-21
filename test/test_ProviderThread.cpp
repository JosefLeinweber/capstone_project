#include "AudioBuffer.h"
#include "Host.h"
#include "ProviderThread.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>

ConfigurationData localConfigurationData =
    setConfigurationData("127.0.0.1", 5000, 5001, 5002);

ConfigurationData remoteConfigurationData =
    setConfigurationData("127.0.0.1", 6000, 6001, 6002);


TEST_CASE("ProviderThread | Constructor")
{

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

    AudioBufferFIFO outputRingBuffer(2, 1024);

    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);
    REQUIRE_NOTHROW(providerThread.setupHost());
}

TEST_CASE("ProviderThread | readFromFIFOBuffer")
{

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

    AudioBufferFIFO outputRingBuffer(2, 20);

    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);
    REQUIRE_FALSE(providerThread.readFromFIFOBuffer());
    printBuffer(outputRingBuffer.buffer);
}

TEST_CASE("ProviderThread | sendAudioToRemoteConsumer")
{
    AudioBufferFIFO outputRingBuffer(2, 20);


    ProviderThread providerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  outputRingBuffer);

    providerThread.setupHost();
    REQUIRE(providerThread.sendAudioToRemoteConsumer());
    printBuffer(outputRingBuffer.buffer);
}
