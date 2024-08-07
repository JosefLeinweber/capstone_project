#include "ConnectDAWs/consumerThread.h"
#include "ConnectDAWs/ringBuffer.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <thread>

TEST_CASE("ConsumerThread | Constructor", "[ConsumerThread]")
{

    try
    {
        ConsumerThread consumerThread(remoteConfigurationData,
                                      localConfigurationData,
                                      inputRingBuffer);
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        FAIL("No error should be thrown");
    }
}

TEST_CASE("ConsumerThread | setupHost")
{


    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);
    REQUIRE_NOTHROW(consumerThread.setupHost());
}

TEST_CASE("ConsumerThread | writeToRingBuffer")
{

    printBuffer(inputRingBuffer.buffer);
    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);

    fillBuffer(consumerThread.m_inputBuffer, 1.0f);
    REQUIRE_NOTHROW(consumerThread.writeToRingBuffer());
    REQUIRE(consumerThread.m_inputRingBuffer.buffer.getSample(0, 0) == 1.0f);
}

TEST_CASE("ConsumerThread | receiveAudioFromRemoteProvider")
{


    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);
    consumerThread.setupHost();
    REQUIRE_FALSE(consumerThread.receiveAudioFromRemoteProvider(
        std::chrono::milliseconds(10)));
}

TEST_CASE("ConsumerThread | timeOut")
{

    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer);
    auto start = std::chrono::high_resolution_clock::now();

    //WHEN: timeOut is called with a timeout of 10ms THEN: it should return false as the time has not passed
    REQUIRE_FALSE(consumerThread.timeOut(std::chrono::milliseconds(10), start));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //WHEN: timeOut is called with a timeout of 0ms after waiting for 10ms from start THEN: it should return true as the time has passed
    REQUIRE(consumerThread.timeOut(std::chrono::milliseconds(0), start));
}
