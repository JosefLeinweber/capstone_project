#include "Host.h"
#include <boost/asio.hpp>
#include <catch2/catch_test_macros.hpp>
#include <thread>

void fillBuffer(juce::AudioBuffer<float> &buffer, float value)
{
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            buffer.setSample(channel, i, value);
        }
    }
}

TEST_CASE("Host | Constructor")
{
    bool sucess = false;
    try
    {
        boost::asio::io_context ioContext;
        Host host;
        sucess = true;
    }
    catch (...)
    {
        sucess = false;
    }

    REQUIRE(sucess == true);
}

TEST_CASE("Host | SetupSocket successfull")

{
    boost::asio::io_context ioContext;
    Host host;
    REQUIRE_NOTHROW(host.setupSocket(ioContext, 8001));
}

TEST_CASE("Host | SetupSocket with invalid port")
{
    boost::asio::io_context ioContext;
    Host host;
    REQUIRE_THROWS(host.setupSocket(ioContext, 0));
}

TEST_CASE("Host | sendAudioBuffer and receive")
{
    std::jthread remoteThread([]() {
        boost::asio::io_context ioContext;
        Host host;
        host.setupSocket(ioContext, 8001);
        juce::AudioBuffer<float> buffer(2, 10);
        fillBuffer(buffer, 0.5);
        boost::asio::ip::udp::endpoint remoteEndpoint(
            boost::asio::ip::address::from_string("127.0.0.1"),
            8002);
        host.sendAudioBuffer(buffer, remoteEndpoint);
    });

    juce::AudioBuffer<float> buffer(2, 10);
    boost::asio::io_context ioContext;
    Host host;
    host.setupSocket(ioContext, 8002);
    REQUIRE(host.receiveAudioBuffer(buffer));
    REQUIRE(buffer.getSample(0, 0) == 0.5);
    remoteThread.join();
}

TEST_CASE("Host | sendAudioBuffer to invalid endpoint still successfull")
{
    boost::asio::io_context ioContext;
    Host host;
    host.setupSocket(ioContext, 8001);
    juce::AudioBuffer<float> buffer(2, 10);
    fillBuffer(buffer, 0.5);
    boost::asio::ip::udp::endpoint remoteEndpoint(
        boost::asio::ip::address::from_string("127.0.0.1"),
        8002);
    REQUIRE_NOTHROW(host.sendAudioBuffer(buffer, remoteEndpoint));
}

TEST_CASE("Host | recieveAudioBuffer timeout")
{
    boost::asio::io_context ioContext;
    Host host;
    host.setupSocket(ioContext, 8001, 50);
    juce::AudioBuffer<float> buffer(2, 10);
    REQUIRE_FALSE(host.receiveAudioBuffer(buffer));
}
