#include "ConnectDAWs/udpHost.h"
#include "sharedValues.h"
#include <boost/asio.hpp>
#include <catch2/catch_test_macros.hpp>
#include <thread>

TEST_CASE("UdpHost | Constructor")
{
    bool sucess = false;
    try
    {
        boost::asio::io_context ioContext;
        UdpHost udpHost;
        sucess = true;
    }
    catch (...)
    {
        sucess = false;
    }

    REQUIRE(sucess == true);
}

TEST_CASE("UdpHost | SetupSocket successfull")

{
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    REQUIRE_NOTHROW(udpHost.setupSocket(ioContext, 8001));
}

TEST_CASE("UdpHost | SetupSocket with invalid port")
{
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    REQUIRE_THROWS(udpHost.setupSocket(ioContext, 0));
}

TEST_CASE("UdpHost | sendAudioBuffer and receive")
{
    std::jthread remoteThread([]() {
        boost::asio::io_context ioContext;
        UdpHost udpHost;
        udpHost.setupSocket(ioContext, 8001);
        juce::AudioBuffer<float> buffer(2, 10);
        fillBuffer(buffer, 0.5);
        boost::asio::ip::udp::endpoint remoteEndpoint(
            boost::asio::ip::address::from_string("127.0.0.1"),
            8002);
        udpHost.sendAudioBuffer(buffer, remoteEndpoint);
    });

    juce::AudioBuffer<float> buffer(2, 10);
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    udpHost.setupSocket(ioContext, 8002);
    bool success = false;
    auto receiveCallback = [&success](const boost::system::error_code &error,
                                      std::size_t bytes_transferred) {
        if (error)
        {
            success = false;
        }
        else
        {
            success = true;
        }
    };

    udpHost.receiveAudioBuffer(buffer,
                               std::bind(receiveCallback,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
    REQUIRE(success);
    REQUIRE(buffer.getSample(0, 0) == 0.5);
    remoteThread.join();
}

TEST_CASE("UdpHost | sendAudioBuffer to invalid endpoint still successfull")
{
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    udpHost.setupSocket(ioContext, 8001);
    juce::AudioBuffer<float> buffer(2, 10);
    fillBuffer(buffer, 0.5);
    boost::asio::ip::udp::endpoint remoteEndpoint(
        boost::asio::ip::address::from_string("127.0.0.1"),
        8002);
    REQUIRE_NOTHROW(udpHost.sendAudioBuffer(buffer, remoteEndpoint));
}

TEST_CASE("UdpHost | recieveAudioBuffer timeout")
{
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    udpHost.setupSocket(ioContext, 8001);
    juce::AudioBuffer<float> buffer(2, 10);
    bool success = true;
    auto receiveCallback = [&success](const boost::system::error_code &error,
                                      std::size_t bytes_transferred) {
        if (error)
        {
            FAIL("Error receiving data");
        }
        else
        {
            FAIL("Data received");
        }
    };

    udpHost.receiveAudioBuffer(buffer,
                               [](const boost::system::error_code &error,
                                  std::size_t bytes_transferred) {
                                   if (error)
                                   {
                                       std::cout << "erro" << std::endl;
                                       FAIL("Error receiving data");
                                   }
                                   else
                                   {
                                       std::cout << "data" << std::endl;
                                       FAIL("Data received");
                                   }
                               });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // udpHost.cancelReceive();
    std::cout << "end" << std::endl;
    FAIL("Timeout");
}
