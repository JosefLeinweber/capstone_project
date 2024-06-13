#include "AudioBuffer.h"
#include "ConnectionManagerThread.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <thread>

TEST_CASE("ConnectionManagerThread | defaul")
{
    REQUIRE(true);
}

TEST_CASE("ConnectionManagerThread | Constructor", "[ConnectionManagerThread]")
{
    bool success = false;
    try
    {
        AudioBufferFIFO inputRingBuffer(2, 2);
        AudioBufferFIFO outputRingBuffer(2, 2);
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        success = true;
    }
    catch (...)
    {
        success = false;
    }

    REQUIRE(success);
}

TEST_CASE("ConnectionManagerThread | setupHost")
{
    AudioBufferFIFO inputRingBuffer(2, 2);
    AudioBufferFIFO outputRingBuffer(2, 2);
    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    REQUIRE(connectionManagerThread.getHost() == nullptr);

    try
    {
        addressData hostAddress("127.0.0.1", 8000);
        connectionManagerThread.setupHost(hostAddress);
        REQUIRE(connectionManagerThread.getHost() != nullptr);
    }
    catch (...)
    {
        FAIL("setupHost failed");
    }
}

TEST_CASE("ConnectionManagerThread | asyncWaitForConnection successfull")
{

    auto exampleThread = std::thread([]() {
        AudioBufferFIFO inputRingBuffer(2, 2);
        AudioBufferFIFO outputRingBuffer(2, 2);
        addressData hostAddress("127.0.0.1", 8000);
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(200));
        REQUIRE(connectionManagerThread.incommingConnection() == true);
    });

    auto providerThread = std::thread([]() {
        addressData providerAddress("127.0.0.1", 8021);
        Host host(providerAddress);
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        addressData hostAddress("127.0.0.1", 8000);
        host.sendHandshake(hostAddress);
        std::cout << "ProviderThread finishes" << std::endl;
        host.stopHost();
    });

    providerThread.join();
    exampleThread.join();
}

TEST_CASE("ConnectionManagerThread | asyncWaitForConnection unsuccessfull")
{

    auto exampleThread = std::thread([]() {
        AudioBufferFIFO inputRingBuffer(2, 2);
        AudioBufferFIFO outputRingBuffer(2, 2);
        addressData hostAddress("127.0.0.1", 8000);
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(1));
        REQUIRE(connectionManagerThread.incommingConnection() == false);
    });

    exampleThread.join();
}

TEST_CASE("ConnectionManagerThread | validateConnection successfull")
{
    auto exampleThread = std::thread([]() {
        bool success = false;
        AudioBufferFIFO inputRingBuffer(2, 2);
        AudioBufferFIFO outputRingBuffer(2, 2);
        addressData hostAddress("127.0.0.1", 8000);
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(200));
        if (connectionManagerThread.incommingConnection())
        {
            success = connectionManagerThread.validateConnection();
            REQUIRE(success == true);
        }
        else
        {
            FAIL("No incoming connection");
        }
        connectionManagerThread.getHost()->stopHost();
    });

    auto providerThread = std::thread([]() {
        addressData providerAddress("127.0.0.1", 8021);
        Host host(providerAddress);
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        addressData hostAddress("127.0.0.1", 8000);
        host.sendHandshake(hostAddress);

        bool connected = host.waitForHandshake();
        if (connected)
        {
            host.sendHandshake(hostAddress);
        }

        std::cout << "ProviderThread finishes" << std::endl;
        host.stopHost();
    });

    providerThread.join();
    exampleThread.join();
}

TEST_CASE("ConnectionManagerThread | validateConnection unsuccessfull")
{
    auto exampleThread = std::thread([]() {
        bool success = false;
        AudioBufferFIFO inputRingBuffer(2, 2);
        AudioBufferFIFO outputRingBuffer(2, 2);
        addressData hostAddress("127.0.0.1", 8000);
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(200));
        if (connectionManagerThread.incommingConnection())
        {
            std::cout << "Incoming connection" << std::endl;
            success = connectionManagerThread.validateConnection();
            std::cout << "Connection validated: " << success << std::endl;
        }
        else
        {
            FAIL("No incoming connection");
        }
        REQUIRE(success);
        connectionManagerThread.getHost()->stopHost();
    });

    auto providerThread = std::thread([]() {
        addressData providerAddress("127.0.0.1", 8021);
        Host host(providerAddress);
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        addressData hostAddress("127.0.0.1", 8000);
        host.sendHandshake(hostAddress);
        bool connected = host.waitForHandshake();
        std::cout << "ProviderThread finishes" << std::endl;
        host.stopHost();
    });

    providerThread.join();
    exampleThread.join();
}
