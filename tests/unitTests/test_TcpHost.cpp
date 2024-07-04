
#include "ConnectDAWs/tcpHost.h"
#include "datagram.pb.h"
#include <catch2/catch_test_macros.hpp>
#include <thread>


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
                try
                {
                    tcpHost.initializeConnection("::1",
                                                 8002,
                                                 std::chrono::milliseconds(1));
                    ioContext.run();
                    FAIL("No exception thrown");
                }
                catch (...)
                {
                    REQUIRE(true);
                }
            }
        }
    }
}

SCENARIO("TcpHost | initializeConnection with reachable remote host")
{
    GIVEN("A tcpHost object with setupHost called and a remote tcpHost running "
          "on a different thread")
    {
        AddressData remoteAddress;
        remoteAddress.set_ip("::1");
        remoteAddress.set_port(8001);

        std::jthread remoteThread([&]() {
            boost::asio::io_context ioContext;
            TcpHost remoteTcpHost(ioContext, remoteAddress.port());
            remoteTcpHost.setupSocket();
            remoteTcpHost.asyncWaitForConnection(callbackFunction);
            ioContext.run();
        });

        boost::asio::io_context ioContext;
        unsigned short port = 8001;
        TcpHost tcpHost(ioContext, port);
        tcpHost.setupSocket();
        WHEN("initializeConnection is called")
        {
            THEN("The function connects to the remote host without error")
            {
                try
                {
                    tcpHost.initializeConnection(
                        remoteAddress.ip(),
                        remoteAddress.port(),
                        std::chrono::milliseconds(1000));
                    ioContext.run();
                    REQUIRE(true);
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

SCENARIO("TcpHost | sendConfiguration without receiving remote host")
{
    GIVEN("A tcpHost connected to a remote tcpHost running "
          "on a different thread")
    {
        AddressData remoteAddress;
        remoteAddress.set_ip("::1");
        remoteAddress.set_port(8001);

        std::jthread remoteThread([&]() {
            std::cout << "from remote thread" << std::endl;
            boost::asio::io_context ioContext;
            TcpHost remoteTcpHost(ioContext, remoteAddress.port());
            remoteTcpHost.setupSocket();
            remoteTcpHost.asyncWaitForConnection(callbackFunction);
            ioContext.run();
            std::cout << "end of remote thread" << std::endl;
        });
        boost::asio::io_context ioContext;
        unsigned short port = 8001;
        TcpHost tcpHost(ioContext, port);
        tcpHost.setupSocket();
        tcpHost.initializeConnection(remoteAddress.ip(), remoteAddress.port());


        WHEN("sendConfiguration is called")
        {
            THEN("The function sends the configuration to the remote host "
                 "without error")
            {
                ConfigurationData configurationData;
                configurationData.set_ip(remoteAddress.ip());
                configurationData.set_provider_port(remoteAddress.port());
                configurationData.set_consumer_port(remoteAddress.port());
                std::cout << "sending configuration" << std::endl;
                try
                {
                    std::cout << "inside try block" << std::endl;
                    std::string serializedString =
                        tcpHost.serializeConfigurationData(configurationData);
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
    AddressData remoteAddress;
    remoteAddress.set_ip("::1");
    remoteAddress.set_port(8001);
    AddressData hostAddress;
    hostAddress.set_ip("::1");
    hostAddress.set_port(8002);

    // GIVEN: A tcpHost connected to a remote tcpHost running
    std::jthread remoteThread([&]() {
        std::cout << "from remote thread" << std::endl;
        boost::asio::io_context ioContext;
        TcpHost remoteTcpHost(ioContext, remoteAddress.port());
        remoteTcpHost.setupSocket();
        remoteTcpHost.initializeConnection(hostAddress.ip(),
                                           hostAddress.port());
        std::string message = "Hello from remote host";
        remoteTcpHost.send(message);
        std::cout << "after send configuration" << std::endl;
    });


    boost::asio::io_context ioContext;
    TcpHost tcpHost(ioContext, hostAddress.port());
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
    AddressData remoteAddress;
    remoteAddress.set_ip("::1");
    remoteAddress.set_port(8001);
    AddressData hostAddress;
    hostAddress.set_ip("::1");
    hostAddress.set_port(8002);

    GIVEN("tcpHost connected to a remote tcpHost who "
          "sends a configuration ")
    {

        std::jthread remoteThread([&]() {
            std::cout << "from remote thread" << std::endl;
            boost::asio::io_context ioContext;
            TcpHost remoteTcpHost(ioContext, remoteAddress.port());
            remoteTcpHost.setupSocket();
            remoteTcpHost.initializeConnection(hostAddress.ip(),
                                               hostAddress.port());
            std::cout << "after init connection" << std::endl;
            ConfigurationData configurationData;
            configurationData.set_ip(remoteAddress.ip());
            configurationData.set_provider_port(remoteAddress.port());
            configurationData.set_consumer_port(hostAddress.port());


            std::string serializedString =
                remoteTcpHost.serializeConfigurationData(configurationData);
            remoteTcpHost.send(serializedString);
            std::cout << "after send configuration" << std::endl;
        });


        WHEN("tcpHost calls recieveConfiguration")
        {
            boost::asio::io_context ioContext;
            TcpHost tcpHost(ioContext, hostAddress.port());
            tcpHost.setupSocket();
            tcpHost.asyncWaitForConnection(callbackFunction);
            ioContext.run();
            std::string configurationDataString =
                tcpHost.receiveConfiguration();

            std::cout << "len of recieved string: "
                      << configurationDataString.size() << std::endl;
            THEN("the recieved data can be successfully deseralized")
            {
                ConfigurationData configurationData =
                    tcpHost.deserializeConfigurationData(
                        configurationDataString);
                REQUIRE(configurationData.provider_port() ==
                        remoteAddress.port());
                REQUIRE(configurationData.ip() == remoteAddress.ip());
                REQUIRE(configurationData.consumer_port() ==
                        hostAddress.port());
            }
        }
        remoteThread.join();
    }
}

SCENARIO("TcpHost | serializer and deserializer")
{
    AddressData remoteAddress;
    remoteAddress.set_ip("::1");
    remoteAddress.set_port(8001);
    AddressData hostAddress;
    hostAddress.set_ip("::1");
    hostAddress.set_port(8002);

    GIVEN("Some data to serialize")
    {
        boost::asio::io_context ioContext;
        TcpHost tcpHost(ioContext, hostAddress.port());

        ConfigurationData configurationData;
        configurationData.set_ip(remoteAddress.ip());
        configurationData.set_provider_port(remoteAddress.port());
        configurationData.set_consumer_port(hostAddress.port());

        WHEN("data gets serialized and deserialized")
        {

            std::string serializedData =
                tcpHost.serializeConfigurationData(configurationData);
            ConfigurationData deseralizedconfigurationData =
                tcpHost.deserializeConfigurationData(serializedData);
            THEN("the data is still equal to the original")
            {

                REQUIRE(deseralizedconfigurationData.provider_port() ==
                        configurationData.provider_port());
                REQUIRE(deseralizedconfigurationData.ip() ==
                        configurationData.ip());
                REQUIRE(deseralizedconfigurationData.ip() ==
                        configurationData.ip());
                REQUIRE(deseralizedconfigurationData.consumer_port() ==
                        configurationData.consumer_port());
            }
        }
    }
}
