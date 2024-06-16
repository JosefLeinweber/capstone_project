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

TEST_CASE("Provider & Consumer | unsucessfully connect")
{

    addressData providerHostAddress("127.0.0.1", 8001);
    addressData providerRemoteAddress("127.0.0.1", 8022);
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

    REQUIRE(isConsumerConnected == false);
    REQUIRE(isProviderConnected == false);

    consumerThread.stopThread(1000);
    providerThread.stopThread(1000);
}

// TEST_CASE("Provider & Consumer | successfully connect on two threads")
// {

//     auto thread1 = std::thread([]() {
//         addressData providerHostAddress("127.0.0.1", 8001);
//         addressData providerRemoteAddress("127.0.0.1", 8202);
//         AudioBufferFIFO outputRingBuffer(2, 1024);
//         std::atomic<bool> isProviderConnected = false;

//         ProviderThread providerThread(providerHostAddress,
//                                       providerRemoteAddress,
//                                       outputRingBuffer,
//                                       isProviderConnected);

//         addressData consumerHostAddress("127.0.0.1", 8002);
//         AudioBufferFIFO inputRingBuffer(2, 1024);
//         std::atomic<bool> isConsumerConnected = false;

//         ConsumerThread consumerThread(consumerHostAddress,
//                                       inputRingBuffer,
//                                       isConsumerConnected);


//         providerThread.startThread();
//         consumerThread.startThread();

//         std::this_thread::sleep_for(std::chrono::seconds(1));

//         consumerThread.stopThread(1000);
//         providerThread.stopThread(1000);
//         std::cout << "1. isConsumerConnected: " << isConsumerConnected
//                   << std::endl;
//         std::cout << "1. isProviderConnected: " << isProviderConnected
//                   << std::endl;
//     });

//     auto thread2 = std::thread([]() {
//         addressData providerHostAddress("127.0.0.1", 8201);
//         addressData providerRemoteAddress("127.0.0.1", 8002);
//         AudioBufferFIFO outputRingBuffer(2, 1024);
//         std::atomic<bool> isProviderConnected = false;

//         ProviderThread providerThread(providerHostAddress,
//                                       providerRemoteAddress,
//                                       outputRingBuffer,
//                                       isProviderConnected);

//         addressData consumerHostAddress("127.0.0.1", 8202);
//         AudioBufferFIFO inputRingBuffer(2, 1024);
//         std::atomic<bool> isConsumerConnected = false;

//         ConsumerThread consumerThread(consumerHostAddress,
//                                       inputRingBuffer,
//                                       isConsumerConnected);


//         providerThread.startThread();
//         consumerThread.startThread();

//         std::this_thread::sleep_for(std::chrono::seconds(1));

//         consumerThread.stopThread(1000);
//         providerThread.stopThread(1000);

//         std::cout << "2. isConsumerConnected: " << isConsumerConnected
//                   << std::endl;
//         std::cout << "2. isProviderConnected: " << isProviderConnected
//                   << std::endl;
//     });

//     std::this_thread::sleep_for(std::chrono::seconds(2));
//     thread1.join();
//     thread2.join();
// }
