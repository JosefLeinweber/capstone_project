#include "AudioBuffer.h"
#include "ConsumerThread.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <thread>

ConfigurationData localConfigurationData =
    setConfigurationData("127.0.0.1", 5000, 5001, 5002);

ConfigurationData remoteConfigurationData =
    setConfigurationData("127.0.0.1", 6000, 6001, 6002);


TEST_CASE("ConsumerThread | Constructor", "[ConsumerThread]")
{
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

    AudioBufferFIFO inputRingBuffer(2, 1024);

    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);
    REQUIRE_NOTHROW(consumerThread.setupHost());
}

TEST_CASE("ConsumerThread | writeToFIFOBuffer")
{

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

    AudioBufferFIFO inputRingBuffer(2, 20);

    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);
    consumerThread.setupHost();
    REQUIRE_FALSE(consumerThread.receiveAudioFromRemoteProvider());
}
