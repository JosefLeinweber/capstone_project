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
                                      inputRingBuffer,
                                      differenceBuffer);
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
                                  inputRingBuffer,
                                  differenceBuffer);
    REQUIRE_NOTHROW(consumerThread.setupHost());
}

TEST_CASE("ConsumerThread | writeToRingBuffer")
{

    printBuffer(inputRingBuffer.m_buffer);
    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  differenceBuffer);

    fillBuffer(consumerThread.m_inputBuffer, 1.0f);
    REQUIRE_NOTHROW(consumerThread.writeToRingBuffer());
    REQUIRE(consumerThread.m_inputRingBuffer.m_buffer.getSample(0, 0) == 1.0f);
}

TEST_CASE("ConsumerThread | receiveAudioFromRemoteProvider")
{


    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  differenceBuffer);
    consumerThread.setupHost();
    REQUIRE_FALSE(consumerThread.receiveAudioFromRemoteProvider(
        std::chrono::milliseconds(10)));
}

TEST_CASE("ConsumerThread | timeOut")
{

    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  differenceBuffer);
    auto start = std::chrono::high_resolution_clock::now();

    //WHEN: timeOut is called with a timeout of 10ms THEN: it should return false as the time has not passed
    REQUIRE_FALSE(consumerThread.timeOut(std::chrono::milliseconds(10), start));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //WHEN: timeOut is called with a timeout of 0ms after waiting for 10ms from start THEN: it should return true as the time has passed
    REQUIRE(consumerThread.timeOut(std::chrono::milliseconds(0), start));
}

TEST_CASE("ConsumerThread | saveLatencyToBuffer")
{

    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  differenceBuffer);
    std::uint64_t timestamp = 0;
    consumerThread.saveLatencyToBuffer(timestamp);
    REQUIRE(differenceBuffer->size() == 1);
    REQUIRE(differenceBuffer->at(0) == timestamp);
}

TEST_CASE("ConsumerThread | calculateLatency")
{
    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  differenceBuffer);
    std::uint64_t timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    std::uint64_t latency = consumerThread.calculateLatency(timestamp);
    std::cout << "Latency: " << latency << std::endl;
    REQUIRE(latency >= 0);
    REQUIRE(latency < 10);
}
