#include "Host.h"
#include "TcpHost.h"
#include <catch2/catch_test_macros.hpp>
#include <thread>
addressData remoteAddress("127.0.0.1", 8002);
addressData hostAddress("127.0.0.1", 8002);

void callbackFunction(const boost::system::error_code &error)
{
    std::cout << "callback function called" << std::endl;
}


SCENARIO("TcpHost | Constructor")
{
    GIVEN("A io_context and a port")
    {
        boost::asio::io_context ioContext;
        unsigned short port = 8001;
        WHEN("A TcpHost is created")
        {
            THEN("The TcpHost is created without error")
            {
                REQUIRE_NOTHROW(TcpHost(ioContext, port));
            }
        }
    }
}

SCENARIO("TcpHost | SetupSocket")
{
    GIVEN("A tcpHost object")
    {
        boost::asio::io_context ioContext;
        unsigned short port = 8001;
        TcpHost tcpHost(ioContext, port);
        WHEN("setupSocket is called")
        {
            THEN("The function is called without error")
            {
                REQUIRE_NOTHROW(tcpHost.setupSocket());
            }
        }
    }
}

SCENARIO("TcpHost | asyncWaitForConnection")
{
    GIVEN("A tcpHost object")
    {
        boost::asio::io_context ioContext;
        unsigned short port = 8001;
        TcpHost tcpHost(ioContext, port);
        tcpHost.setupSocket();
        WHEN("asyncWaitForConnection is called")
        {
            THEN("The function is called without error")
            {
                try
                {

                    tcpHost.asyncWaitForConnection(callbackFunction);
                }
                catch (std::exception &e)
                {
                    std::cout << "something went wrong:" << std::endl;
                    std::cout << e.what() << std::endl;
                    FAIL(e.what());
                }
            }
        }
    }
}

SCENARIO("TcpHost | initializeConnection with unreachable remote host")
{
    GIVEN("A tcpHost object with setupHost called and a addressData object")
    {
        boost::asio::io_context ioContext;
        unsigned short port = 8001;
        TcpHost tcpHost(ioContext, port);
        tcpHost.setupSocket();

        WHEN("initializeConnection is called")
        {
            THEN("The function fails because the remote host is not reachable")
            {
                REQUIRE_THROWS(tcpHost.initializeConnection(remoteAddress),
                               "Failed to connect to remote host");
            }
        }
    }
}

SCENARIO("TcpHost | initializeConnection with reachable remote host")
{
    GIVEN("A tcpHost object with setupHost called and a remote tcpHost running "
          "on a different thread")
    {
        boost::asio::io_context ioContext;
        unsigned short port = 8001;
        TcpHost tcpHost(ioContext, port);
        tcpHost.setupSocket();
        std::jthread remoteThread([]() {
            boost::asio::io_context ioContext;
            TcpHost remoteTcpHost(ioContext, remoteAddress.port);
            remoteTcpHost.setupSocket();
            remoteTcpHost.asyncWaitForConnection(callbackFunction);
            ioContext.run();
        });

        WHEN("initializeConnection is called")
        {
            THEN("The function connects to the remote host without error")
            {
                REQUIRE_NOTHROW(tcpHost.initializeConnection(remoteAddress));
            }
        }
        remoteThread.join();
    }
}

SCENARIO("TcpHost | sendConfiguration without receiving remote host")
{
    GIVEN("A tcpHost connected to a remote tcpHost running "
          "on a different thread")
    {
        std::jthread remoteThread([]() {
            std::cout << "from remote thread" << std::endl;
            boost::asio::io_context ioContext;
            TcpHost remoteTcpHost(ioContext, remoteAddress.port);
            remoteTcpHost.setupSocket();
            remoteTcpHost.asyncWaitForConnection(callbackFunction);
            ioContext.run();
            std::cout << "end of remote thread" << std::endl;
        });
        boost::asio::io_context ioContext;
        unsigned short port = 8001;
        TcpHost tcpHost(ioContext, port);
        tcpHost.setupSocket();
        tcpHost.initializeConnection(remoteAddress);


        WHEN("sendConfiguration is called")
        {
            THEN("The function sends the configuration to the remote host "
                 "without error")
            {
                ConfigurationDataStruct configurationData;
                configurationData.providerAddress = remoteAddress;
                configurationData.consumerAddress = remoteAddress;
                std::cout << "sending configuration" << std::endl;
                try
                {
                    std::cout << "inside try block" << std::endl;
                    std::string serializedString =
                        tcpHost.serializeConfigurationData(
                            configurationData.toPb());
                    tcpHost.send(serializedString);
                    std::cout << "sent configuration" << std::endl;
                }
                catch (std::exception &e)
                {
                    std::cout << "something went wrong:" << std::endl;
                    std::cout << e.what() << std::endl;
                    FAIL(e.what());
                }
            }
        }
        remoteThread.join();
    }
}

TEST_CASE("TcpHost | sendConfiguration with receiving remote host")
{
    // GIVEN: A tcpHost connected to a remote tcpHost running
    std::jthread remoteThread([]() {
        std::cout << "from remote thread" << std::endl;
        boost::asio::io_context ioContext;
        TcpHost remoteTcpHost(ioContext, remoteAddress.port);
        remoteTcpHost.setupSocket();
        remoteTcpHost.initializeConnection(hostAddress);
        std::string message = "Hello from remote host";
        remoteTcpHost.send(message);
        std::cout << "after send configuration" << std::endl;
    });


    boost::asio::io_context ioContext;
    TcpHost tcpHost(ioContext, hostAddress.port);
    tcpHost.setupSocket();
    tcpHost.asyncWaitForConnection(callbackFunction);
    ioContext.run();
    // WHEN: tcpHost receives the data
    std::string received_string = tcpHost.receiveConfiguration();

    // THEN: the received data is equal to the sent data
    std::cout << "received string: " << received_string << std::endl;
    REQUIRE(received_string == "Hello from remote host");
    remoteThread.join();
}


SCENARIO("TcpHost | sendConfiguration, receiveConfiguration, "
         "deseraializeConfiguration")
{
    std::cout << "1" << std::endl;
    GIVEN("tcpHost connected to a remote tcpHost who "
          "sends a configuration ")
    {
        std::cout << "2" << std::endl;
        std::jthread remoteThread([]() {
            std::cout << "from remote thread" << std::endl;
            boost::asio::io_context ioContext;
            TcpHost remoteTcpHost(ioContext, remoteAddress.port);
            remoteTcpHost.setupSocket();
            remoteTcpHost.initializeConnection(hostAddress);
            std::cout << "after init connection" << std::endl;
            addressData addressData1("125.0.0.1", 5555);
            addressData addressData2("125.0.0.2", 7777);

            ConfigurationDataStruct configurationData;
            configurationData.providerAddress = addressData1;
            configurationData.consumerAddress = addressData2;
            std::string serializedString =
                remoteTcpHost.serializeConfigurationData(
                    configurationData.toPb());
            remoteTcpHost.send(serializedString);
            std::cout << "after send configuration" << std::endl;
        });


        WHEN("tcpHost calls recieveConfiguration")
        {
            std::cout << "3" << std::endl;
            boost::asio::io_context ioContext;
            TcpHost tcpHost(ioContext, hostAddress.port);
            tcpHost.setupSocket();
            tcpHost.asyncWaitForConnection(callbackFunction);
            ioContext.run();
            std::string configurationDataString =
                tcpHost.receiveConfiguration();

            std::cout << "len of recieved string: "
                      << configurationDataString.size() << std::endl;
            THEN("the recieved data can be successfully deseralized")
            {
                std::cout << "4" << std::endl;
                ConfigurationDataStruct configurationData =
                    tcpHost.deserializeConfigurationData(
                        configurationDataString);
                REQUIRE(configurationData.providerAddress.port == 5555);
                REQUIRE(configurationData.providerAddress.ip == "125.0.0.1");
                REQUIRE(configurationData.consumerAddress.ip == "125.0.0.2");
                REQUIRE(configurationData.consumerAddress.port == 7777);
            }
        }
        remoteThread.join();
    }
}

SCENARIO("TcpHost | serializer and deserializer")
{

    GIVEN("Some data to serialize")
    {
        boost::asio::io_context ioContext;
        TcpHost tcpHost(ioContext, hostAddress.port);

        addressData addressData1("125.0.0.1", 5555);
        addressData addressData2("125.0.0.2", 7777);

        ConfigurationDataStruct configurationData;
        configurationData.providerAddress = addressData1;
        configurationData.consumerAddress = addressData2;

        WHEN("data gets serialized and deserialized")
        {

            std::string serializedData =
                tcpHost.serializeConfigurationData(configurationData.toPb());
            ConfigurationDataStruct deseralizedconfigurationData =
                tcpHost.deserializeConfigurationData(serializedData);
            THEN("the data is still equal to the original")
            {

                REQUIRE(deseralizedconfigurationData.providerAddress.port ==
                        configurationData.providerAddress.port);
                REQUIRE(deseralizedconfigurationData.providerAddress.ip ==
                        configurationData.providerAddress.ip);
                REQUIRE(deseralizedconfigurationData.consumerAddress.ip ==
                        configurationData.consumerAddress.ip);
                REQUIRE(deseralizedconfigurationData.consumerAddress.port ==
                        configurationData.consumerAddress.port);
            }
        }
    }
}
