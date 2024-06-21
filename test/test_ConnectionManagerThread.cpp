#include "AudioBuffer.h"
#include "ConnectionManagerThread.h"
#include "ConsumerThread.h"
#include "ProviderThread.h"
#include "TcpHost.h"
#include "datagram.pb.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <thread>


ConfigurationData localConfigurationData =
    setConfigurationData("127.0.0.1", 5000, 5001, 5002);

ConfigurationData remoteConfigurationData =
    setConfigurationData("127.0.0.1", 6000, 6001, 6002);


AudioBufferFIFO inputRingBuffer(2, 1024);
AudioBufferFIFO outputRingBuffer(2, 1024);

AudioBufferFIFO remoteInputRingBuffer(2, 1024);
AudioBufferFIFO remoteOutputRingBuffer(2, 1024);

std::atomic<bool> startConnection = false;
std::atomic<bool> stopConnection = false;

TEST_CASE("ConnectionManagerThread | Constructor")
{
    bool success = false;
    try
    {
        ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                        inputRingBuffer,
                                                        outputRingBuffer,
                                                        startConnection,
                                                        stopConnection);
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
    // GIVEN: a ConnectionManagerThread
    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);
    // WHEN: setupHost is called
    // THEN: no exception should be thrown
    REQUIRE_NOTHROW(connectionManagerThread.setupHost());
}

TEST_CASE("ConnectionManagerThread | asyncWaitForConnection successfull")
{
    std::atomic<bool> connectionEstablished = false;
    REQUIRE_FALSE(startConnection);
    REQUIRE_FALSE(stopConnection);
    //GIVEN: a ConnectionManagerThread which is waiting for a connection
    auto waitingThread = std::jthread([&]() {
        ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                        inputRingBuffer,
                                                        outputRingBuffer,
                                                        startConnection,
                                                        stopConnection);
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        connectionEstablished = connectionManagerThread.incomingConnection();
        std::cout << "waitingThread finishes" << std::endl;
    });
    //WHEN: a remote host tries to connect
    auto connectionInitializerThread = std::jthread([]() {
        boost::asio::io_context ioContext;
        TcpHost host(ioContext, remoteConfigurationData.host_port());
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        host.initializeConnection(localConfigurationData.ip(),
                                  localConfigurationData.host_port());
        std::cout << "connectionInitializerThread finishes" << std::endl;
    });

    connectionInitializerThread.join();
    waitingThread.join();
    REQUIRE(connectionEstablished);
}

TEST_CASE("ConnectionManagerThread | asyncWaitForConnection unsuccessfull")
{
    std::atomic<bool> connectionEstablished = true;
    auto waitingThread = std::jthread([&]() {
        ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                        inputRingBuffer,
                                                        outputRingBuffer,
                                                        startConnection,
                                                        stopConnection);
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(5));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        connectionEstablished = connectionManagerThread.incomingConnection();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    waitingThread.join();
    REQUIRE_FALSE(connectionEstablished);
}

TEST_CASE("ConnectionManagerThread | initializeConnection successfull")
{
    //GIVEN: a remote host waits for a connection
    auto connectionInitializerThread = std::jthread([]() {
        ConnectionManagerThread connectionManagerThread(remoteConfigurationData,
                                                        inputRingBuffer,
                                                        outputRingBuffer,
                                                        startConnection,
                                                        stopConnection);
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection();
        connectionManagerThread.m_ioContext.run();
    });

    //WHEN: a ConnectionManagerThread tries to connect to the remote host
    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);
    connectionManagerThread.setupHost();
    //THEN: no exception should be thrown
    REQUIRE_NOTHROW(
        connectionManagerThread.initializeConnection(remoteConfigurationData));
    connectionInitializerThread.join();
}

TEST_CASE("ConnectionManagerThread | initializeConnection unsuccessfully")
{
    //GIVEN: the remote host is not waiting for a connection

    //WHEN: a ConnectionManagerThread tries to connect to the remote host
    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);
    connectionManagerThread.setupHost();
    //THEN: exception should be thrown
    try
    {
        connectionManagerThread.initializeConnection(remoteConfigurationData);
    }
    catch (std::exception &e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
        REQUIRE(true);
    }
}


TEST_CASE("ConnectionManagerThread | receiveConfigurationData successfull")
{
    std::atomic<bool> configurationDataReceived = false;
    //GIVEN: a ConnectionManagerThread which is connected to a remote host and waits for a configuration data
    auto connectionInitializerThread = std::jthread([]() {
        boost::asio::io_context ioContext;
        TcpHost host(ioContext, remoteConfigurationData.host_port());
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        host.initializeConnection(localConfigurationData.ip(),
                                  localConfigurationData.host_port());
        std::string serializedData =
            host.serializeConfigurationData(remoteConfigurationData);
        host.send(serializedData);
        std::cout << "connectionInitializerThread finishes" << std::endl;
    });


    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);
    connectionManagerThread.setupHost();
    connectionManagerThread.asyncWaitForConnection();
    connectionManagerThread.m_ioContext.run();

    // WHEN: the configuration data is received
    configurationDataReceived =
        connectionManagerThread.receiveConfigurationData();

    // THEN: the configuration data should be received successfully and the data should be correct
    REQUIRE(configurationDataReceived);
    REQUIRE(connectionManagerThread.getRemoteConfigurationData().ip() ==
            remoteConfigurationData.ip());
    REQUIRE(
        connectionManagerThread.getRemoteConfigurationData().provider_port() ==
        remoteConfigurationData.provider_port());
    REQUIRE(
        connectionManagerThread.getRemoteConfigurationData().consumer_port() ==
        remoteConfigurationData.consumer_port());

    connectionInitializerThread.join();
}

TEST_CASE("ConnectionManagerThread | receiveConfigurationData unsuccessfull")
{
    //GIVEN: a ConnectionManagerThread which is not connected to a remote host
    std::atomic<bool> configurationDataReceived = true;

    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);

    connectionManagerThread.setupHost();
    //WHEN: the ConnectionManagerThread tries to receive configuration data
    configurationDataReceived =
        connectionManagerThread.receiveConfigurationData();
    //THEN: the configuration data should not be received
    REQUIRE_FALSE(configurationDataReceived);
}

TEST_CASE("ConnectionManagerThread | receiveConfigurationData receiver is "
          "initiated after sender")
{
    std::atomic<bool> configurationDataReceived = false;
    //GIVEN: a ConnectionManagerThread which is connected to a remote host and waits
    auto waitingThread = std::jthread([&]() {
        ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                        inputRingBuffer,
                                                        outputRingBuffer,
                                                        startConnection,
                                                        stopConnection);
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection();
        connectionManagerThread.m_ioContext.run();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        configurationDataReceived =
            connectionManagerThread.receiveConfigurationData();
    });
    //WHEN: a remote host tries to connect
    auto connectionInitializerThread = std::jthread([]() {
        boost::asio::io_context ioContext;
        TcpHost host(ioContext, remoteConfigurationData.host_port());
        host.setupSocket();
        host.initializeConnection(localConfigurationData.ip(),
                                  localConfigurationData.host_port());
        std::string serializedData =
            host.serializeConfigurationData(remoteConfigurationData);
        host.send(serializedData);

        std::cout << "connectionInitializerThread finishes" << std::endl;
    });

    connectionInitializerThread.join();
    waitingThread.join();
    REQUIRE(configurationDataReceived);
}


TEST_CASE("ConnectionManagerThread | sendConfigurationData successfull")
{
    std::atomic<bool> configurationDataSent = false;
    //GIVEN: a ConnectionManagerThread which is connected to a remote host
    auto remoteThread = std::jthread([&]() {
        ConnectionManagerThread connectionManagerThread(remoteConfigurationData,
                                                        inputRingBuffer,
                                                        outputRingBuffer,
                                                        startConnection,
                                                        stopConnection);
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(1000));
        connectionManagerThread.m_ioContext.run();

        //WHEN: the ConnectionManagerThread sends configuration data
        configurationDataSent = connectionManagerThread.sendConfigurationData(
            remoteConfigurationData);
    });
    boost::asio::io_context ioContext;
    TcpHost host(ioContext, localConfigurationData.host_port());
    host.setupSocket();
    host.initializeConnection(remoteConfigurationData.ip(),
                              remoteConfigurationData.host_port());
    std::string msg = host.receiveConfiguration();
    ConfigurationData receivedConfigurationData;
    receivedConfigurationData = host.deserializeConfigurationData(msg);

    remoteThread.join();
    //THEN: the configuration data should be sent successfully and the data should be correct on the receiving end

    std::cout << "Provider IP: " << receivedConfigurationData.ip()
              << " Provider Port: " << receivedConfigurationData.provider_port()
              << std::endl;

    REQUIRE(configurationDataSent);
    REQUIRE(receivedConfigurationData.ip() == remoteConfigurationData.ip());
    REQUIRE(receivedConfigurationData.provider_port() ==
            remoteConfigurationData.provider_port());
    REQUIRE(receivedConfigurationData.consumer_port() ==
            remoteConfigurationData.consumer_port());
}

TEST_CASE("ConnectionManagerThread | sendConfigurationData to a not "
          "connected remote")
{
    //GIVEN: a ConnectionManagerThread which is not connected to a remote host
    std::atomic<bool> configurationDataSent = true;
    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);
    connectionManagerThread.setupHost();
    //WHEN: the ConnectionManagerThread tries to send configuration data
    configurationDataSent =
        connectionManagerThread.sendConfigurationData(localConfigurationData);
    //THEN: the configuration data should not be sent
    REQUIRE_FALSE(configurationDataSent);
}

TEST_CASE("ConnectionManagerThread | sendConfiguration and "
          "receiveConfiguration successfull")
{
    //GIVEN: a ConnectionManagerThread which is connected to a remote host
    auto sendingThread = std::jthread([]() {
        ConnectionManagerThread connectionManagerThread(remoteConfigurationData,
                                                        inputRingBuffer,
                                                        outputRingBuffer,
                                                        startConnection,
                                                        stopConnection);
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(1000));
        connectionManagerThread.m_ioContext.run();
        connectionManagerThread.sendConfigurationData(remoteConfigurationData);
    });

    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);
    connectionManagerThread.setupHost();
    connectionManagerThread.initializeConnection(remoteConfigurationData);

    //WHEN: the ConnectionManagerThread tries to receive configuration data
    bool received = connectionManagerThread.receiveConfigurationData();

    //THEN: the configuration data should be received successfully and the data should be correct
    REQUIRE(received);

    ConfigurationData receivedConfigurationData =
        connectionManagerThread.getRemoteConfigurationData();

    std::cout << "Provider IP: " << receivedConfigurationData.ip()
              << " Provider Port: " << receivedConfigurationData.provider_port()
              << std::endl;
    REQUIRE(receivedConfigurationData.ip() == remoteConfigurationData.ip());
    REQUIRE(receivedConfigurationData.provider_port() ==
            remoteConfigurationData.provider_port());
    REQUIRE(receivedConfigurationData.consumer_port() ==
            remoteConfigurationData.consumer_port());
    sendingThread.join();
}

TEST_CASE("ConnectionManagerThread  | exchangeConfigurationDataWithRemote "
          "successfully")
{
    //GIVEN: a ConnectionManagerThread which is connected to a remote host
    auto remoteThread = std::jthread([]() {
        ConnectionManagerThread connectionManagerThread(remoteConfigurationData,
                                                        inputRingBuffer,
                                                        outputRingBuffer,
                                                        startConnection,
                                                        stopConnection);
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(1000));
        connectionManagerThread.m_ioContext.run();
        connectionManagerThread.exchangeConfigurationDataWithRemote(
            remoteConfigurationData);
    });

    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);
    connectionManagerThread.setupHost();
    connectionManagerThread.initializeConnection(remoteConfigurationData);

    //WHEN: the ConnectionManagerThread tries to receive configuration data
    REQUIRE_NOTHROW(connectionManagerThread.exchangeConfigurationDataWithRemote(
        localConfigurationData));

    //THEN: the configuration data should be received successfully and the data should be correct
    ConfigurationData receivedConfigurationData =
        connectionManagerThread.getRemoteConfigurationData();

    std::cout << "Provider IP: " << receivedConfigurationData.ip()
              << " Provider Port: " << receivedConfigurationData.provider_port()
              << std::endl;
    REQUIRE(receivedConfigurationData.ip() == remoteConfigurationData.ip());
    REQUIRE(receivedConfigurationData.provider_port() ==
            remoteConfigurationData.provider_port());
    REQUIRE(receivedConfigurationData.consumer_port() ==
            remoteConfigurationData.consumer_port());
    remoteThread.join();
}

TEST_CASE("ConnectionManagerThread | exchangeConfigurationDataWithRemote "
          "unsuccessfull")
{
    //GIVEN: a ConnectionManagerThread which is not connected to a remote host
    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);
    connectionManagerThread.setupHost();

    //WHEN: the ConnectionManagerThread calls exchangeConfigurationDataWithRemote
    //THEN: the configuration data should not be exchanged
    bool success = true;
    try
    {

        success = connectionManagerThread.exchangeConfigurationDataWithRemote(
            localConfigurationData);
    }
    catch (std::exception &e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
        REQUIRE(true);
    }
    REQUIRE_FALSE(success);
}


TEST_CASE("ConnectionManagerThread | startUpProviderAndConsumerThreads "
          "successfull start up of both threads")
{

    //GIVEN: a ConnectionManagerThread
    ConnectionManagerThread connectionManagerThread(localConfigurationData,
                                                    inputRingBuffer,
                                                    outputRingBuffer,
                                                    startConnection,
                                                    stopConnection);

    bool success = false;
    try
    {
        success = connectionManagerThread.startUpProviderAndConsumerThreads(
            localConfigurationData,
            remoteConfigurationData,
            std::chrono::milliseconds(10));
    }
    catch (...)
    {
        FAIL("Exception thrown");
    }
    REQUIRE(success);
}

// TEST_CASE("ConnectionManagerThread | startUpProviderAndConsumerThreads send "
//           "and receive audio then quit")
// {


// }


// TEST_CASE("ConnectionManagerThread | startUpProviderAndConsumerThreads only "
//           "successfull isConsumerConnected")
// {
//     auto startUpProviderAndConsumerThread = std::thread([]() {
//         ConnectionManagerThread connectionManagerThread(inputRingBuffer,
//                                                         outputRingBuffer);
//         connectionManagerThread.startUpProviderAndConsumerThreads(
//             addressDataCollection.providerAddress,
//             addressDataCollection.consumerAddress,
//             remoteAddressDataCollection.consumerAddress);
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//     });

//     std::atomic<bool> isConsumerConnected = false;


//     ConsumerThread consumerThread(remoteAddressDataCollection.consumerAddress,
//                                   remoteInputRingBuffer,
//                                   isConsumerConnected,
//                                   "LonlyConsumerThread");

//     consumerThread.startThread();

//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//     consumerThread.signalThreadShouldExit();
//     while (consumerThread.isThreadRunning())
//     {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//     startUpProviderAndConsumerThread.join();
//     std::cout << " CC " << isConsumerConnected << std::endl;
//     REQUIRE(isConsumerConnected == true);
// }

// TEST_CASE("ConnectionManagerThread | startUpProviderAndConsumerThreads only "
//           "successfull isProviderConnected")
// {
//     auto startUpProviderAndConsumerThread = std::thread([]() {
//         ConnectionManagerThread connectionManagerThread(inputRingBuffer,
//                                                         outputRingBuffer);
//         // connectionManagerThread.setupHost(); //!! WHY DOES THIS CAUSE THE TEST TO FAIL???
//         connectionManagerThread.onlyStartConsumerThread(
//             addressDataCollection.consumerAddress);
//     });

//     std::atomic<bool> isProviderConnected = false;

//     ProviderThread providerThread(remoteAddressDataCollection.providerAddress,
//                                   addressDataCollection.consumerAddress,
//                                   remoteOutputRingBuffer,
//                                   isProviderConnected);

//     providerThread.startThread();

//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//     providerThread.signalThreadShouldExit();
//     while (providerThread.isThreadRunning())
//     {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     startUpProviderAndConsumerThread.join();
//     // providerThread.join();
//     REQUIRE(isProviderConnected == true);
//     // FAIL("This test is not implemented yet");
// }


// TEST_CASE(
//     "ConnectionManagerThread | startUpProviderAndConsumerThreads successfull")
// {
//     auto startUpProviderAndConsumerThread = std::thread([]() {
//         bool success = false;
//         ConnectionManagerThread connectionManagerThread(inputRingBuffer,
//                                                         outputRingBuffer);
//         // connectionManagerThread.setupHost();
//         success = connectionManagerThread.startUpProviderAndConsumerThreads(
//             addressDataCollection.providerAddress,
//             addressDataCollection.consumerAddress,
//             remoteAddressDataCollection.consumerAddress);
//         // connectionManagerThread.getHost()->stopHost();
//     });

//     std::atomic<bool> isProviderConnected = false;
//     ProviderThread providerThread(remoteAddressDataCollection.providerAddress,
//                                   addressDataCollection.consumerAddress,
//                                   remoteOutputRingBuffer,
//                                   isProviderConnected);

//     std::atomic<bool> isConsumerConnected = false;
//     ConsumerThread consumerThread(remoteAddressDataCollection.consumerAddress,
//                                   remoteInputRingBuffer,
//                                   isConsumerConnected);

//     providerThread.startThread();
//     consumerThread.startThread();

//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//     std::cout << " PC " << isProviderConnected << " CC " << isConsumerConnected
//               << std::endl;


//     startUpProviderAndConsumerThread.join();
//     providerThread.stopThread(1000);
//     consumerThread.stopThread(1000);
//     CHECK(isProviderConnected == true);
//     CHECK(isConsumerConnected == true);
//     REQUIRE(isProviderConnected == true);
//     REQUIRE(isConsumerConnected == true);
// }
