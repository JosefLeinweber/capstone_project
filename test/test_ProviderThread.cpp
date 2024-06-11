#include "AudioBuffer.h"
#include "Host.h"
#include "ProviderThread.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>


TEST_CASE("ProviderThread | Constructor")
{
    bool sucess = false;
    try
    {
        addressData hostAddress("127.0.0.1", 8001);
        addressData remoteAddress("127.0.0.1", 8022);
        AudioBufferFIFO outputRingBuffer(2, 1024);
        std::atomic<bool> isProviderConnected;

        ProviderThread providerThread(hostAddress,
                                      remoteAddress,
                                      outputRingBuffer,
                                      isProviderConnected);
        sucess = true;
    }
    catch (std::exception &e)
    {
        FAIL(e.what());
    }

    REQUIRE(sucess);
}

TEST_CASE("ProviderThread | setupHost")
{
    addressData hostAddress("127.0.0.1", 8001);
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected;

    ProviderThread providerThread(hostAddress,
                                  remoteAddress,
                                  outputRingBuffer,
                                  isProviderConnected);
    try
    {
        providerThread.setupHost();
    }
    catch (std::exception &e)
    {
        FAIL(e.what());
    }
    REQUIRE(true);
}

TEST_CASE("ProviderThread | validateConnection successfully")
{
    addressData hostAddress("127.0.0.1", 8001);
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected = false;

    ProviderThread providerThread(hostAddress,
                                  remoteAddress,
                                  outputRingBuffer,
                                  isProviderConnected);

    auto consumerThread = std::thread([&]() {
        addressData consumerAddress("127.0.0.1", 8022);
        Host host(consumerAddress);
        host.setupSocket();
        bool gotHandshake = host.waitForHandshake();
        if (gotHandshake)
        {
            host.sendHandshake(hostAddress);
        }
        std::cout << "ConsumerThread finishes" << std::endl;
    });

    providerThread.startThread();
    consumerThread.join();
    providerThread.stopThread(1000);
    REQUIRE(isProviderConnected == true);
}

TEST_CASE("ProviderThread | validateConnection unsuccessfully")
{
    addressData hostAddress("127.0.0.1", 8001);
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected = true;

    ProviderThread providerThread(hostAddress,
                                  remoteAddress,
                                  outputRingBuffer,
                                  isProviderConnected);

    auto consumerThread = std::thread([&]() {
        addressData consumerAddress("127.0.0.1", 8002);
        Host host(consumerAddress);
        host.setupSocket();
        bool gotHandshake = host.waitForHandshake();
        if (gotHandshake)
        {
            host.sendHandshake(hostAddress);
        }
        std::cout << "ConsumerThread finishes" << std::endl;
    });

    providerThread.startThread();
    consumerThread.join();
    providerThread.stopThread(1000);
    REQUIRE(isProviderConnected == false);
}


TEST_CASE("ProviderThread | sendHandshake before remote waits for handshake")
{
    addressData hostAddress("127.0.0.1", 8001);
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected = false;

    ProviderThread providerThread(hostAddress,
                                  remoteAddress,
                                  outputRingBuffer,
                                  isProviderConnected);
    providerThread.startThread();

    auto consumerThread = std::thread([&]() {
        addressData consumerAddress("127.0.0.1", 8022);
        Host host(consumerAddress);
        host.setupSocket();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        bool gotHandshake = host.waitForHandshake();
        auto currentTime = std::chrono::system_clock::now().time_since_epoch();
        if (gotHandshake)
        {

            std::cout << "ConsumerThread recieved handshake at: "
                      << currentTime.count() << " seconds" << std::endl;
            host.sendHandshake(hostAddress);
            auto currentTime =
                std::chrono::system_clock::now().time_since_epoch();
            std::cout << "ConsumerThread sends handshake at: "
                      << currentTime.count() << " seconds" << std::endl;

            REQUIRE(true);
        }
        std::cout << "ConsumerThread finishes" << std::endl;
    });

    consumerThread.join();
    providerThread.stopThread(1000);
    REQUIRE(isProviderConnected == true);
}

TEST_CASE(
    "ProviderThread | too much time between sendHandshake and waitForHandshake")
{
    addressData hostAddress("127.0.0.1", 8001);
    addressData remoteAddress("127.0.0.1", 8022);
    AudioBufferFIFO outputRingBuffer(2, 1024);
    std::atomic<bool> isProviderConnected = true;

    ProviderThread providerThread(hostAddress,
                                  remoteAddress,
                                  outputRingBuffer,
                                  isProviderConnected);
    providerThread.startThread();

    auto consumerThread = std::thread([&]() {
        addressData consumerAddress("127.0.0.1", 8022);
        Host host(consumerAddress);
        host.setupSocket();

        bool gotHandshake = host.waitForHandshake();
        auto currentTime = std::chrono::system_clock::now().time_since_epoch();
        if (gotHandshake)
        {

            std::cout << "ConsumerThread recieved handshake at: "
                      << currentTime.count() << " seconds" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            host.sendHandshake(hostAddress);
            auto currentTime =
                std::chrono::system_clock::now().time_since_epoch();
            std::cout << "ConsumerThread sends handshake at: "
                      << currentTime.count() << " seconds" << std::endl;
        }
    });

    consumerThread.join();
    providerThread.stopThread(2000);
    REQUIRE(isProviderConnected == false);
}
