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
                                      benchmark1);
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
                                  benchmark1);
    REQUIRE_NOTHROW(consumerThread.setupHost());
}

TEST_CASE("ConsumerThread | writeToRingBuffer")
{

    printBuffer(inputRingBuffer.m_buffer);
    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  benchmark1);

    fillBuffer(consumerThread.m_inputBuffer, 1.0f);
    REQUIRE_NOTHROW(consumerThread.writeToRingBuffer());
    REQUIRE(consumerThread.m_inputRingBuffer.m_buffer.getSample(0, 0) == 1.0f);
}

TEST_CASE("ConsumerThread | receiveAudioFromRemoteProvider")
{


    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  benchmark1);
    consumerThread.setupHost();
    REQUIRE_FALSE(consumerThread.receiveAudioFromRemoteProvider(
        std::chrono::milliseconds(10)));
}

TEST_CASE("ConsumerThread | timeOut")
{

    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  benchmark1);
    auto start = std::chrono::high_resolution_clock::now();

    //WHEN: timeOut is called with a timeout of 10ms THEN: it should return false as the time has not passed
    REQUIRE_FALSE(consumerThread.timeOut(std::chrono::milliseconds(10), start));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //WHEN: timeOut is called with a timeout of 0ms after waiting for 10ms from start THEN: it should return true as the time has passed
    REQUIRE(consumerThread.timeOut(std::chrono::milliseconds(0), start));
}

TEST_CASE("ConsumerThread | saveTimestamps")
{
    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  benchmark1);
    consumerThread.saveTimestamps(1);
    REQUIRE(benchmark1->m_networkLatencyBenchmarkData.startTimestamps.size() ==
            1);
    REQUIRE(benchmark1->m_networkLatencyBenchmarkData.endTimestamps.size() ==
            1);
    REQUIRE(benchmark1->m_networkLatencyBenchmarkData.startTimestamps[0] == 1);
}

TEST_CASE("ConsumerThread | benchmark finished")
{
    ConsumerThread consumerThread(remoteConfigurationData,
                                  localConfigurationData,
                                  inputRingBuffer,
                                  benchmark1);
    for (int i = 0; i < 100; i++)
    {
        benchmark1->m_networkLatencyBenchmarkData.startTimestamps.push_back(1);
        benchmark1->m_networkLatencyBenchmarkData.endTimestamps.push_back(2);
    }
    REQUIRE(benchmark1->finished());
}
