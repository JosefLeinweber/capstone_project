#include "AudioBuffer.h"
#include "ConnectionManagerThread.h"
#include "ConsumerThread.h"
#include "ProviderThread.h"
#include "TcpHost.h"
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
    // GIVEN: a ConnectionManagerThread
    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    // WHEN: setupHost is called
    // THEN: no exception should be thrown
    REQUIRE_NOTHROW(
        connectionManagerThread.setupHost(addressDataCollection.hostAddress));
}

TEST_CASE("ConnectionManagerThread | asyncWaitForConnection successfull")
{
    std::atomic<bool> connectionEstablished = false;
    //GIVEN: a ConnectionManagerThread which is waiting for a connection
    auto waitingThread = std::jthread([&]() {
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        connectionManagerThread.asyncWaitForConnection();
        connectionManagerThread.m_ioContext.run();
        connectionEstablished = connectionManagerThread.incomingConnection();
    });
    //WHEN: a remote host tries to connect
    auto connectionInitializerThread = std::jthread([]() {
        boost::asio::io_context ioContext;
        TcpHost host(ioContext,
                     remoteAddressDataCollection.providerAddress.port);
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        host.initializeConnection(addressDataCollection.hostAddress);
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
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(5));
        connectionManagerThread.m_ioContext.run();
        connectionEstablished = connectionManagerThread.incomingConnection();
    });

    waitingThread.join();
    REQUIRE_FALSE(connectionEstablished);
}

TEST_CASE("ConnectionManagerThread | initializeConnection successfull")
{
    //GIVEN: a remote host waits for a connection
    auto connectionInitializerThread = std::jthread([]() {
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(
            remoteAddressDataCollection.providerAddress);
        connectionManagerThread.asyncWaitForConnection();
        connectionManagerThread.m_ioContext.run();
    });

    //WHEN: a ConnectionManagerThread tries to connect to the remote host
    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    connectionManagerThread.setupHost(addressDataCollection.hostAddress);
    //THEN: no exception should be thrown
    REQUIRE_NOTHROW(connectionManagerThread.initializeConnection(
        remoteAddressDataCollection.providerAddress));
    connectionInitializerThread.join();
}

TEST_CASE("ConnectionManagerThread | initializeConnection unsuccessfully")
{
    //GIVEN: the remote host is not waiting for a connection

    //WHEN: a ConnectionManagerThread tries to connect to the remote host
    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    connectionManagerThread.setupHost(addressDataCollection.hostAddress);
    //THEN: exception should be thrown
    try
    {
        connectionManagerThread.initializeConnection(
            remoteAddressDataCollection.providerAddress);
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
        ConfigurationDataStruct configurationData;
        configurationData.providerAddress =
            addressDataCollection.providerAddress;
        configurationData.consumerAddress =
            addressDataCollection.consumerAddress;
        boost::asio::io_context ioContext;
        TcpHost host(ioContext,
                     remoteAddressDataCollection.providerAddress.port);
        host.setupSocket();
        // Send handshake before the consumer thread waits for it
        host.initializeConnection(addressDataCollection.hostAddress);
        std::string serializedData =
            host.serializeConfigurationData(configurationData.toPb());
        host.send(serializedData);
        std::cout << "connectionInitializerThread finishes" << std::endl;
    });


    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    connectionManagerThread.setupHost(addressDataCollection.hostAddress);
    connectionManagerThread.asyncWaitForConnection();
    connectionManagerThread.m_ioContext.run();

    // WHEN: the configuration data is received
    configurationDataReceived =
        connectionManagerThread.receiveConfigurationData();

    std::cout
        << "Provider IP: "
        << connectionManagerThread.getConfigurationData().providerAddress.ip
        << " Provider Port: "
        << connectionManagerThread.getConfigurationData().providerAddress.port
        << std::endl;

    // THEN: the configuration data should be received successfully and the data should be correct
    REQUIRE(configurationDataReceived);
    REQUIRE(connectionManagerThread.getConfigurationData().providerAddress.ip ==
            addressDataCollection.providerAddress.ip);
    REQUIRE(
        connectionManagerThread.getConfigurationData().providerAddress.port ==
        addressDataCollection.providerAddress.port);
    REQUIRE(connectionManagerThread.getConfigurationData().consumerAddress.ip ==
            addressDataCollection.consumerAddress.ip);
    REQUIRE(
        connectionManagerThread.getConfigurationData().consumerAddress.port ==
        addressDataCollection.consumerAddress.port);

    connectionInitializerThread.join();
}

TEST_CASE("ConnectionManagerThread | receiveConfigurationData unsuccessfull")
{
    //GIVEN: a ConnectionManagerThread which is not connected to a remote host
    std::atomic<bool> configurationDataReceived = true;

    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);

    connectionManagerThread.setupHost(addressDataCollection.hostAddress);
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
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        connectionManagerThread.asyncWaitForConnection();
        connectionManagerThread.m_ioContext.run();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        configurationDataReceived =
            connectionManagerThread.receiveConfigurationData();
    });
    //WHEN: a remote host tries to connect
    auto connectionInitializerThread = std::jthread([]() {
        ConfigurationDataStruct configurationData;
        configurationData.providerAddress =
            addressDataCollection.providerAddress;
        configurationData.consumerAddress =
            addressDataCollection.consumerAddress;
        boost::asio::io_context ioContext;
        TcpHost host(ioContext,
                     remoteAddressDataCollection.providerAddress.port);
        host.setupSocket();
        host.initializeConnection(addressDataCollection.hostAddress);
        std::string serializedData =
            host.serializeConfigurationData(configurationData.toPb());
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
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(addressDataCollection.hostAddress);
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(1000));
        connectionManagerThread.m_ioContext.run();
        ConfigurationDataStruct configurationData;
        configurationData.providerAddress =
            addressDataCollection.providerAddress;
        configurationData.consumerAddress =
            addressDataCollection.consumerAddress;
        //WHEN: the ConnectionManagerThread sends configuration data
        configurationDataSent =
            connectionManagerThread.sendConfigurationData(configurationData);
    });
    boost::asio::io_context ioContext;
    TcpHost host(ioContext, remoteAddressDataCollection.providerAddress.port);
    host.setupSocket();
    host.initializeConnection(addressDataCollection.hostAddress);
    std::string msg = host.receiveConfiguration();
    ConfigurationDataStruct receivedConfigurationData;
    receivedConfigurationData = host.deserializeConfigurationData(msg);

    remoteThread.join();
    //THEN: the configuration data should be sent successfully and the data should be correct on the receiving end

    std::cout << "Provider IP: " << receivedConfigurationData.providerAddress.ip
              << " Provider Port: "
              << receivedConfigurationData.providerAddress.port << std::endl;
    std::cout << "Consumer IP: " << receivedConfigurationData.consumerAddress.ip
              << " Consumer Port: "
              << receivedConfigurationData.consumerAddress.port << std::endl;

    REQUIRE(configurationDataSent);
    REQUIRE(receivedConfigurationData.providerAddress.ip ==
            addressDataCollection.providerAddress.ip);
    REQUIRE(receivedConfigurationData.providerAddress.port ==
            addressDataCollection.providerAddress.port);
    REQUIRE(receivedConfigurationData.consumerAddress.ip ==
            addressDataCollection.consumerAddress.ip);
    REQUIRE(receivedConfigurationData.consumerAddress.port ==
            addressDataCollection.consumerAddress.port);
}

TEST_CASE(
    "ConnectionManagerThread | sendConfigurationData to a not connected remote")
{
    //GIVEN: a ConnectionManagerThread which is not connected to a remote host
    std::atomic<bool> configurationDataSent = true;
    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    connectionManagerThread.setupHost(addressDataCollection.hostAddress);
    ConfigurationDataStruct configurationData;
    configurationData.providerAddress = addressDataCollection.providerAddress;
    configurationData.consumerAddress = addressDataCollection.consumerAddress;
    //WHEN: the ConnectionManagerThread tries to send configuration data
    configurationDataSent =
        connectionManagerThread.sendConfigurationData(configurationData);
    //THEN: the configuration data should not be sent
    REQUIRE_FALSE(configurationDataSent);
}

TEST_CASE("ConnectionManagerThread | sendConfiguration and "
          "receiveConfiguration successfull")
{
    //GIVEN: a ConnectionManagerThread which is connected to a remote host
    auto sendingThread = std::jthread([]() {
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(
            remoteAddressDataCollection.providerAddress);
        connectionManagerThread.asyncWaitForConnection();
        connectionManagerThread.m_ioContext.run();
        ConfigurationDataStruct configurationData;
        configurationData.providerAddress =
            addressDataCollection.providerAddress;
        configurationData.consumerAddress =
            addressDataCollection.consumerAddress;
        connectionManagerThread.sendConfigurationData(configurationData);
    });

    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    connectionManagerThread.setupHost(addressDataCollection.hostAddress);
    connectionManagerThread.initializeConnection(
        remoteAddressDataCollection.providerAddress);

    //WHEN: the ConnectionManagerThread tries to receive configuration data
    bool received = connectionManagerThread.receiveConfigurationData();

    //THEN: the configuration data should be received successfully and the data should be correct
    REQUIRE(received);

    ConfigurationDataStruct receivedConfigurationData =
        connectionManagerThread.getConfigurationData();

    std::cout << "Provider IP: " << receivedConfigurationData.providerAddress.ip
              << " Provider Port: "
              << receivedConfigurationData.providerAddress.port << std::endl;
    REQUIRE(receivedConfigurationData.providerAddress.ip ==
            addressDataCollection.providerAddress.ip);
    REQUIRE(receivedConfigurationData.providerAddress.port ==
            addressDataCollection.providerAddress.port);
    REQUIRE(receivedConfigurationData.consumerAddress.ip ==
            addressDataCollection.consumerAddress.ip);
    REQUIRE(receivedConfigurationData.consumerAddress.port ==
            addressDataCollection.consumerAddress.port);
    sendingThread.join();
}

TEST_CASE("ConnectionManagerThread  | exchangeConfigurationDataWithRemote "
          "successfully")
{
    //GIVEN: a ConnectionManagerThread which is connected to a remote host
    auto remoteThread = std::jthread([]() {
        ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                        outputRingBuffer);
        connectionManagerThread.setupHost(
            remoteAddressDataCollection.providerAddress);
        connectionManagerThread.asyncWaitForConnection();
        connectionManagerThread.m_ioContext.run();
        ConfigurationDataStruct configurationData;
        configurationData.providerAddress =
            addressDataCollection.providerAddress;
        configurationData.consumerAddress =
            addressDataCollection.consumerAddress;
        connectionManagerThread.exchangeConfigurationDataWithRemote(
            configurationData);
    });

    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    connectionManagerThread.setupHost(addressDataCollection.hostAddress);
    connectionManagerThread.initializeConnection(
        remoteAddressDataCollection.providerAddress);
    ConfigurationDataStruct configurationData;
    configurationData.providerAddress = addressDataCollection.providerAddress;
    configurationData.consumerAddress = addressDataCollection.consumerAddress;
    //WHEN: the ConnectionManagerThread calls exchangeConfigurationDataWithRemote
    //THEN: the configuration data should be exchanged successfully
    bool success;
    REQUIRE_NOTHROW(
        success = connectionManagerThread.exchangeConfigurationDataWithRemote(
            configurationData));
    REQUIRE(success);

    REQUIRE(connectionManagerThread.getConfigurationData().providerAddress.ip ==
            addressDataCollection.providerAddress.ip);
    REQUIRE(
        connectionManagerThread.getConfigurationData().providerAddress.port ==
        addressDataCollection.providerAddress.port);
    REQUIRE(connectionManagerThread.getConfigurationData().consumerAddress.ip ==
            addressDataCollection.consumerAddress.ip);
    REQUIRE(
        connectionManagerThread.getConfigurationData().consumerAddress.port ==
        addressDataCollection.consumerAddress.port);
    remoteThread.join();
}

TEST_CASE("ConnectionManagerThread | exchangeConfigurationDataWithRemote "
          "unsuccessfull")
{
    //GIVEN: a ConnectionManagerThread which is not connected to a remote host
    ConnectionManagerThread connectionManagerThread(inputRingBuffer,
                                                    outputRingBuffer);
    connectionManagerThread.setupHost(addressDataCollection.hostAddress);
    ConfigurationDataStruct configurationData;
    configurationData.providerAddress = addressDataCollection.providerAddress;
    configurationData.consumerAddress = addressDataCollection.consumerAddress;
    //WHEN: the ConnectionManagerThread calls exchangeConfigurationDataWithRemote
    //THEN: the configuration data should not be exchanged
    bool success = true;
    try
    {

        success = connectionManagerThread.exchangeConfigurationDataWithRemote(
            configurationData);
    }
    catch (std::exception &e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
        REQUIRE(true);
        REQUIRE_FALSE(success);
    }
}


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
//         // connectionManagerThread.setupHost(addressDataCollection.hostAddress); //!! WHY DOES THIS CAUSE THE TEST TO FAIL???
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
//         // connectionManagerThread.setupHost(addressDataCollection.hostAddress);
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
