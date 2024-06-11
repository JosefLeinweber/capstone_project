#include "ConsumerThread.h"
#include "ProviderThread.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>

TEST_CASE("Provider & Consumer | successfully connect")
{

    addressData providerHostAddress("127.0.0.1", 8001);
    addressData providerRemoteAddress("127.0.0.1", 8021);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected = false;

    ProviderThread providerThread(providerHostAddress,
                                  providerRemoteAddress,
                                  outputRingBuffer,
                                  isProviderConnected);

    addressData consumerHostAddress("127.0.0.1", 8021);
    AudioBufferFIFO inputRingBuffer(2, 1024);
    std::atomic<bool> isConsumerConnected = false;

    ConsumerThread consumerThread(consumerHostAddress,
                                  inputRingBuffer,
                                  isConsumerConnected);


    providerThread.startThread();
    consumerThread.startThread();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    REQUIRE(isConsumerConnected == true);
    REQUIRE(isProviderConnected == true);

    consumerThread.stopThread(1000);
    providerThread.stopThread(1000);
}
