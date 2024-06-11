#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <iostream>
#include "ConsumerThread.h"
#include "AudioBuffer.h"

TEST_CASE("ConsumerThread | Constructor", "[ConsumerThread]") {
    bool success = false;
    try{
        AudioBufferFIFO inputRingBuffer(2, 2);
        std::atomic<bool> isConsumerConnected = false;
        addressData hostAddress("127.0.0.1", 8001);
        ConsumerThread consumerThread(inputRingBuffer, isConsumerConnected, hostAddress);
        success = true;
    }
    catch(...){
        success = false;
    }

    REQUIRE(success);
}

TEST_CASE("ConsumerThread | setupHost"){
    AudioBufferFIFO inputRingBuffer(2, 2);
    std::atomic<bool> isConsumerConnected = false;
    addressData hostAddress("127.0.0.1", 8001);

    ConsumerThread consumerThread(inputRingBuffer, isConsumerConnected, hostAddress);
    REQUIRE(consumerThread.getHost() == nullptr);

    try{
        consumerThread.setupHost();
    }
    catch(...){
        FAIL("setupHost failed");
    }
    REQUIRE(consumerThread.getHost() != nullptr);
    REQUIRE(true);

}

TEST_CASE("ConsumerThread | validateConnection successfully"){
    AudioBufferFIFO inputRingBuffer(2, 2);
    std::atomic<bool> isConsumerConnected = false;
    addressData hostAddress("127.0.0.1", 8001);

    ConsumerThread consumerThread(inputRingBuffer, isConsumerConnected, hostAddress);
    consumerThread.startThread();

    auto providerThread = std::thread([&](){
        addressData providerAddress("127.0.0.1", 8022);
        Host host(providerAddress);
        host.sendHandshake(hostAddress);
        host.waitForHandshake();
        std::cout << "ProviderThread finishes" << std::endl;
    });


    providerThread.join();
    consumerThread.stopThread(1000);
    REQUIRE(isConsumerConnected == true);
}

TEST_CASE("ConsumerThread | validateConnection unsuccessfully") {
    AudioBufferFIFO inputRingBuffer(2, 2);
    std::atomic<bool> isConsumerConnected = true;
    addressData hostAddress("127.0.0.1", 8001);
    ConsumerThread consumerThread(inputRingBuffer, isConsumerConnected, hostAddress);
    std::cout << "Consumer thread created" << std::endl;
    consumerThread.startThread();
    std::cout << "Consumer thread started" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    REQUIRE(isConsumerConnected == false);
    consumerThread.stopThread(1500);
}

TEST_CASE("ConsumerThread | sendHandshake before remote waits for handshake") {
    AudioBufferFIFO inputRingBuffer(2, 2);
    std::atomic<bool> isConsumerConnected = false;
    addressData hostAddress("127.0.0.1", 8001);
    ConsumerThread consumerThread(inputRingBuffer, isConsumerConnected, hostAddress);
    consumerThread.startThread();
    auto providerThread = std::thread([&]() {
        addressData providerAddress("127.0.0.1", 8022);
        Host host(providerAddress);
        // Send handshake before the consumer thread waits for it
        host.sendHandshake(hostAddress);
        std::cout << "ProviderThread finishes" << std::endl;
    });
    providerThread.join();
    consumerThread.stopThread(1000);
    REQUIRE(isConsumerConnected == true);
}
