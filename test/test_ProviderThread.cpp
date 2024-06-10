#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <iostream>
#include <juce_core/juce_core.h>
#include "ProviderThread.h"
#include "Host.h"
#include "AudioBuffer.h"


TEST_CASE("ProviderThread | Constructor") {
    bool sucess = false;
    try {
        addressData hostAddress("127.0.0.1", 8001);
        addressData remoteAddress("127.0.0.1", 8022);
        AudioBufferFIFO outputRingBuffer(2, 1024);
        std::atomic<bool> isProviderConnected;

        ProviderThread providerThread(hostAddress, remoteAddress, outputRingBuffer, isProviderConnected);
        sucess = true;
    } catch (std::exception& e) {
        FAIL(e.what());
    }

    REQUIRE(sucess);
}

TEST_CASE("ProviderThread | setupHost") {
    addressData hostAddress("127.0.0.1", 8001);
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected;

    ProviderThread providerThread(hostAddress, remoteAddress, outputRingBuffer, isProviderConnected);
    try {
        providerThread.setupHost();
    } catch (std::exception& e) {
        FAIL(e.what());
    }
    REQUIRE(true);
}

TEST_CASE("ProviderThread | validateConnection successfully") {
    addressData hostAddress("127.0.0.1", 8001);    
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected = false;

    ProviderThread providerThread(hostAddress, remoteAddress, outputRingBuffer, isProviderConnected);

    auto consumerThread = std::thread([&](){
        addressData consumerAddress("127.0.0.1", 8022);
        Host host(consumerAddress);
        bool gotHandshake = host.waitForHandshake();
        if (gotHandshake) {
            host.sendHandshake(hostAddress);
        }
        std::cout << "ConsumerThread finishes" << std::endl;
    });

    providerThread.startThread();
    consumerThread.join();
    providerThread.stopThread(1000);
    REQUIRE(isProviderConnected == true);
}

TEST_CASE("ProviderThread | validateConnection unsuccessfully") {
    addressData hostAddress("127.0.0.1", 8001);    
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected = true;

    ProviderThread providerThread(hostAddress, remoteAddress, outputRingBuffer, isProviderConnected);

    auto consumerThread = std::thread([&](){
        addressData consumerAddress("127.0.0.1", 8002);
        Host host(consumerAddress);
        bool gotHandshake = host.waitForHandshake();
        if (gotHandshake) {
            host.sendHandshake(hostAddress);
        }
        std::cout << "ConsumerThread finishes" << std::endl;
    });

    providerThread.startThread();
    consumerThread.join();
    providerThread.stopThread(1000);
    REQUIRE(isProviderConnected == false);
}


TEST_CASE("ProviderThread | sendHandshake before remote waits for handshake") {
    addressData hostAddress("127.0.0.1", 8001);    
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected = false;

    ProviderThread providerThread(hostAddress, remoteAddress, outputRingBuffer, isProviderConnected);
    providerThread.startThread();

    auto consumerThread = std::thread([&](){
        addressData consumerAddress("127.0.0.1", 8022);
        Host host(consumerAddress);
        bool gotHandshake = host.waitForHandshake();
        if (gotHandshake) {
            host.sendHandshake(hostAddress);
        }
        std::cout << "ConsumerThread finishes" << std::endl;
    });

    consumerThread.join();
    providerThread.stopThread(1000);
    REQUIRE(isProviderConnected == true);
}