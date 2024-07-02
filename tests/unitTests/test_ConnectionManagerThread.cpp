#include "ConnectDAWs/audioBuffer.h"
#include "ConnectDAWs/connectionManagerThread.h"
#include "ConnectDAWs/consumerThread.h"
#include "ConnectDAWs/providerThread.h"
#include "ConnectDAWs/tcpHost.h"
#include "datagram.pb.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <thread>

TEST_CASE("ConnectionManagerThread | setupHost")
{
    REQUIRE_NOTHROW(connectionManagerThread.setupHost());
}

TEST_CASE("ConnectionManagerThread | asyncWaitForConnection successfull")
{
    std::atomic<bool> connectionEstablished = false;
    REQUIRE_FALSE(startConnection);
    REQUIRE_FALSE(stopConnection);
    //GIVEN: a ConnectionManagerThread which is waiting for a connection
    auto waitingThread = std::jthread([&]() {
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        connectionEstablished = connectionManagerThread.isConnected();
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
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(5));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        connectionEstablished = connectionManagerThread.isConnected();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    waitingThread.join();
    REQUIRE_FALSE(connectionEstablished);
}

TEST_CASE("ConnectionManagerThread | stopAsyncWaitForConnection")
{
    connectionManagerThread.setupHost();
    connectionManagerThread.asyncWaitForConnection(
        std::chrono::milliseconds(5000));
    REQUIRE_NOTHROW(connectionManagerThread.stopAsyncWaitForConnection());
}

TEST_CASE("ConnectionManagerThread | initializeConnection successfull")
{
    //GIVEN: a remote host waits for a connection
    auto connectionInitializerThread = std::jthread([]() {
        remoteConnectionManagerThread.setupHost();
        remoteConnectionManagerThread.asyncWaitForConnection();
        remoteConnectionManagerThread.m_ioContext.run();
    });

    //WHEN: a ConnectionManagerThread tries to connect to the remote host
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
        host.initializeConnection(localConfigurationData.ip(),
                                  localConfigurationData.host_port());
        std::string serializedData =
            host.serializeConfigurationData(remoteConfigurationData);
        host.send(serializedData);
        std::cout << "connectionInitializerThread finishes" << std::endl;
    });


    connectionManagerThread.setupHost();
    connectionManagerThread.asyncWaitForConnection();
    connectionManagerThread.startIOContextInDifferentThread();

    // WHEN: the configuration data is received
    configurationDataReceived =
        connectionManagerThread.receiveConfigurationData();

    // THEN: the configuration data should be received successfully and the data should be correct
    REQUIRE(configurationDataReceived);
    REQUIRE(connectionManagerThread.m_remoteConfigurationData.ip() ==
            remoteConfigurationData.ip());
    REQUIRE(connectionManagerThread.m_remoteConfigurationData.provider_port() ==
            remoteConfigurationData.provider_port());
    REQUIRE(connectionManagerThread.m_remoteConfigurationData.consumer_port() ==
            remoteConfigurationData.consumer_port());

    connectionInitializerThread.join();
}

TEST_CASE("ConnectionManagerThread | receiveConfigurationData unsuccessfull")
{
    //GIVEN: a ConnectionManagerThread which is not connected to a remote host
    std::atomic<bool> configurationDataReceived = true;


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
        connectionManagerThread.setupHost();
        connectionManagerThread.asyncWaitForConnection();
        connectionManagerThread.startIOContextInDifferentThread();
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
        remoteConnectionManagerThread.setupHost();
        remoteConnectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(1000));
        remoteConnectionManagerThread.startIOContextInDifferentThread();

        //WHEN: the ConnectionManagerThread sends configuration data
        configurationDataSent =
            remoteConnectionManagerThread.sendConfigurationData(
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

    connectionManagerThread.setupHost();
    //WHEN: the ConnectionManagerThread tries to send configuration data
    configurationDataSent =
        connectionManagerThread.sendConfigurationData(localConfigurationData);
    //THEN: the configuration data should not be sent
    REQUIRE_FALSE(configurationDataSent);
}

TEST_CASE("ConnectionManagerThread | exchangeConfigurationDataWithRemote "
          "unsuccessfull")
{
    //GIVEN: a ConnectionManagerThread which is not connected to a remote host
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
