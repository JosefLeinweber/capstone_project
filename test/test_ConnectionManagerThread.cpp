#include "AudioBuffer.h"
#include "ConnectionManagerThread.h"
#include "ConsumerThread.h"
#include "ProviderThread.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <thread>

struct AddressDataCollection
{
    addressData hostAddress;
    addressData consumerAddress;
    addressData providerAddress;
};

struct RemoteAddressDataCollection
{
    addressData consumerAddress;
    addressData providerAddress;
};

AddressDataCollection addressDataCollection({"127.0.0.1", 8000},
                                            {"127.0.0.1", 8001},
                                            {"127.0.0.1", 8002});

RemoteAddressDataCollection remoteAddressDataCollection({"127.0.0.1", 8771},
                                                        {"127.0.0.1", 8772});

AudioBufferFIFO inputRingBuffer(2, 1024);
AudioBufferFIFO outputRingBuffer(2, 1024);

AudioBufferFIFO remoteInputRingBuffer(2, 1024);
AudioBufferFIFO remoteOutputRingBuffer(2, 1024);

TEST_CASE("ConnectionManagerThread | Constructor")
{
    bool success = false;
    try
    {
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
    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    REQUIRE(connectionManagerThread.getHost() == nullptr);

    try
    {
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        REQUIRE(connectionManagerThread.getHost() != nullptr);
    }
    catch (...)
    {
        FAIL("setupHost failed");
    }
}

TEST_CASE("ConnectionManagerThread | asyncWaitForConnection successfull")
{

    auto waitingThread = std::thread([]() {
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(200));
        REQUIRE(connectionManagerThread.incommingConnection() == true);
    });

    auto connectionInitializerThread = std::thread([]() {
        Host host(remoteAddressDataCollection.providerAddress);
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        host.sendHandshake(addressDataCollection.hostAddress);
        std::cout << "connectionInitializerThread finishes" << std::endl;
        host.stopHost();
    });

    connectionInitializerThread.join();
    waitingThread.join();
}

TEST_CASE("ConnectionManagerThread | asyncWaitForConnection unsuccessfull")
{

    auto waitingThread = std::thread([]() {
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(1));
        REQUIRE(connectionManagerThread.incommingConnection() == false);
    });

    waitingThread.join();
}

TEST_CASE("ConnectionManagerThread | validateConnection successfull")
{
    auto validateConnectionThread = std::thread([]() {
        bool success = false;
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
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

    auto connectionInitializerThread = std::thread([]() {
        Host host(remoteAddressDataCollection.providerAddress);
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        host.sendHandshake(addressDataCollection.hostAddress);

        bool connected = host.waitForHandshake();
        if (connected)
        {
            host.sendHandshake(addressDataCollection.hostAddress);
        }

        std::cout << "connectionInitializerThread finishes" << std::endl;
        host.stopHost();
    });

    connectionInitializerThread.join();
    validateConnectionThread.join();
}

TEST_CASE("ConnectionManagerThread | validateConnection unsuccessfull after "
          "successfull asyncWaitForConnection")
{
    //FIXME: when this tests fails, it crashes or throws a debug error. This needs to be fixed!
    auto connectionValidationThread = std::thread([]() {
        bool success = false;
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
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
        REQUIRE(success == false);
    });

    auto connectionInitializerThread = std::thread([]() {
        Host host(remoteAddressDataCollection.providerAddress);
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        host.sendHandshake(addressDataCollection.hostAddress);
        std::cout << "connectionInitializerThread finishes" << std::endl;
        host.stopHost();
    });

    connectionInitializerThread.join();
    connectionValidationThread.join();
}

TEST_CASE("ConnectionManagerThread | validateConnection unsuccessfull after "
          "unsuccessfull asyncWaitForConnection")
{
    auto connectionValidationThread = std::thread([]() {
        bool success = false;

        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(1));
        REQUIRE(connectionManagerThread.incommingConnection() == false);
    });

    connectionValidationThread.join();
}

TEST_CASE("ConnectionManagerThread | startUpProviderAndConsumerThreads only "
          "successfull isConsumerConnected")
{
    auto startUpProviderAndConsumerThread = std::thread([]() {
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.startUpProviderAndConsumerThreads(
            addressDataCollection.providerAddress,
            addressDataCollection.consumerAddress,
            remoteAddressDataCollection.consumerAddress);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });

    std::atomic<bool> isConsumerConnected = false;


    ConsumerThread consumerThread(remoteAddressDataCollection.consumerAddress,
                                  remoteInputRingBuffer,
                                  isConsumerConnected,
                                  "LonlyConsumerThread");

    consumerThread.startThread();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    consumerThread.signalThreadShouldExit();
    while (consumerThread.isThreadRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    startUpProviderAndConsumerThread.join();
    std::cout << " CC " << isConsumerConnected << std::endl;
    REQUIRE(isConsumerConnected == true);
}

TEST_CASE("ConnectionManagerThread | startUpProviderAndConsumerThreads only "
          "successfull isProviderConnected")
{
    auto startUpProviderAndConsumerThread = std::thread([]() {
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        // connectionManagerThread.setupHost(addressDataCollection.hostAddress); //!! WHY DOES THIS CAUSE THE TEST TO FAIL???
        connectionManagerThread.onlyStartConsumerThread(
            addressDataCollection.consumerAddress);
    });

    std::atomic<bool> isProviderConnected = false;

    ProviderThread providerThread(remoteAddressDataCollection.providerAddress,
                                  addressDataCollection.consumerAddress,
                                  remoteOutputRingBuffer,
                                  isProviderConnected);

    providerThread.startThread();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    providerThread.signalThreadShouldExit();
    while (providerThread.isThreadRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    startUpProviderAndConsumerThread.join();
    // providerThread.join();
    REQUIRE(isProviderConnected == true);
    // FAIL("This test is not implemented yet");
}


TEST_CASE(
    "ConnectionManagerThread | startUpProviderAndConsumerThreads successfull")
{
    auto startUpProviderAndConsumerThread = std::thread([]() {
        bool success = false;
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        // connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        success = connectionManagerThread.startUpProviderAndConsumerThreads(
            addressDataCollection.providerAddress,
            addressDataCollection.consumerAddress,
            remoteAddressDataCollection.consumerAddress);
        // connectionManagerThread.getHost()->stopHost();
    });

    std::atomic<bool> isProviderConnected = false;
    ProviderThread providerThread(remoteAddressDataCollection.providerAddress,
                                  addressDataCollection.consumerAddress,
                                  remoteOutputRingBuffer,
                                  isProviderConnected);

    std::atomic<bool> isConsumerConnected = false;
    ConsumerThread consumerThread(remoteAddressDataCollection.consumerAddress,
                                  remoteInputRingBuffer,
                                  isConsumerConnected);

    providerThread.startThread();
    consumerThread.startThread();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << " PC " << isProviderConnected << " CC " << isConsumerConnected
              << std::endl;


    startUpProviderAndConsumerThread.join();
    providerThread.stopThread(1000);
    consumerThread.stopThread(1000);
    CHECK(isProviderConnected == true);
    CHECK(isConsumerConnected == true);
    REQUIRE(isProviderConnected == true);
    REQUIRE(isConsumerConnected == true);
}
