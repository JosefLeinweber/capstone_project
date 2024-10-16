#include "ConnectDAWs/udpHost.h"
#include "sharedValues.h"
#include <boost/asio.hpp>
#include <catch2/catch_test_macros.hpp>
#include <thread>

TEST_CASE("UdpHost Constructor")
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

TEST_CASE("UdpHost SetupSocket successfull")

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

TEST_CASE("UdpHost | getTimestamp")
{
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    uint64_t timestamp = udpHost.getTimestamp();
    std::cout << "Timestamp: " << timestamp << std::endl;
    REQUIRE(std::to_string(timestamp).length() == 13);
}

TEST_CASE("UdpHost | timestamptToBytes")
{
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    uint64_t timestamp = udpHost.getTimestamp();
    std::vector<uint8_t> bytes = udpHost.timestampToBytes(timestamp);
    std::cout << "Timestamp: " << timestamp << std::endl;
    std::cout << "Bytes: ";
    for (auto byte : bytes)
    {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
    REQUIRE(bytes.size() == sizeof(timestamp));
    uint64_t newTimestamp;
    std::memcpy(&newTimestamp, bytes.data(), sizeof(timestamp));
    REQUIRE(newTimestamp == timestamp);
}

TEST_CASE("UdpHost | audioBufferToBytes")
{
    juce::AudioBuffer<float> buffer(2, 10);
    fillBuffer(buffer, 0.5);
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    std::vector<uint8_t> bytes = udpHost.audioBufferToBytes(buffer);
    std::cout << "Bytes: ";
    for (auto byte : bytes)
    {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
    REQUIRE(bytes.size() ==
            buffer.getNumChannels() * buffer.getNumSamples() * sizeof(float));
    juce::AudioBuffer<float> newBuffer(2, 10);
    newBuffer.clear();
    std::memcpy(newBuffer.getWritePointer(0), bytes.data(), bytes.size());
    printBuffer(newBuffer);
    REQUIRE(buffer == newBuffer);
}

TEST_CASE("UdpHost | concatenateBytes")
{
    std::vector<uint8_t> a = {0x01, 0x02, 0x03};
    std::vector<uint8_t> b = {0x04, 0x05, 0x06};
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    std::vector<uint8_t> result = udpHost.concatenateBytes(a, b);
    std::cout << "Result: ";
    for (auto byte : result)
    {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
    REQUIRE(result.size() == a.size() + b.size());
    REQUIRE(result[0] == 0x01);
    REQUIRE(result[1] == 0x02);
    REQUIRE(result[2] == 0x03);
    REQUIRE(result[3] == 0x04);
    REQUIRE(result[4] == 0x05);
    REQUIRE(result[5] == 0x06);
}

TEST_CASE("UdpHost | timestampToBytes, audioBufferToBytes and concatenateBytes")
{
    juce::AudioBuffer<float> buffer(2, 10);
    fillBuffer(buffer, 0.5);
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    uint64_t timestamp = udpHost.getTimestamp();
    std::vector<uint8_t> timestampBytes = udpHost.timestampToBytes(timestamp);
    std::vector<uint8_t> audioBufferBytes = udpHost.audioBufferToBytes(buffer);
    std::vector<uint8_t> result =
        udpHost.concatenateBytes(timestampBytes, audioBufferBytes);
    std::cout << "Result: ";
    for (auto byte : result)
    {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
    REQUIRE(result.size() == timestampBytes.size() + audioBufferBytes.size());
    uint64_t newTimestamp;
    std::memcpy(&newTimestamp, result.data(), sizeof(timestamp));
    REQUIRE(newTimestamp == timestamp);
    juce::AudioBuffer<float> newBuffer(2, 10);
    newBuffer.clear();
    std::memcpy(newBuffer.getWritePointer(0),
                result.data() + sizeof(timestamp),
                audioBufferBytes.size());
    printBuffer(buffer);
    printBuffer(newBuffer);
    REQUIRE(buffer == newBuffer);
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
            boost::asio::ip::address::from_string("::1"),
            8002);
        udpHost.sendAudioBuffer(buffer, remoteEndpoint);
    });

    juce::AudioBuffer<float> buffer(2, 10);
    buffer.clear();
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    udpHost.setupSocket(ioContext, 8002);
    bool success = false;
    auto receiveCallback = [&success](const boost::system::error_code &error,
                                      std::size_t bytes_transferred,
                                      std::uint64_t timestamp) {
        std::cout << "Something received" << std::endl;
        std::cout << "Timestamp: " << timestamp << std::endl;
        if (error)
        {
            std::cout << "Error: " << error.message() << std::endl;
            success = false;
        }
        else
        {
            success = true;
        }
    };
    printBuffer(buffer);

    udpHost.asyncReceiveAudioBuffer(buffer,
                                    std::bind(receiveCallback,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3));
    ioContext.run();
    printBuffer(buffer);
    REQUIRE(success);
    REQUIRE(buffer.getSample(0, 0) == 0.5);
    remoteThread.join();
}

TEST_CASE("UdpHost | sendAudioBuffer and handle timestamp in receive")
{
    std::jthread remoteThread([]() {
        boost::asio::io_context ioContext;
        UdpHost udpHost;
        udpHost.setupSocket(ioContext, 8001);
        juce::AudioBuffer<float> buffer(2, 10);
        fillBuffer(buffer, 0.5);
        boost::asio::ip::udp::endpoint remoteEndpoint(
            boost::asio::ip::address::from_string("::1"),
            8002);
        udpHost.sendAudioBuffer(buffer, remoteEndpoint);
    });

    juce::AudioBuffer<float> buffer(2, 10);
    buffer.clear();
    boost::asio::io_context ioContext;
    UdpHost udpHost;
    udpHost.setupSocket(ioContext, 8002);
    bool success = false;
    auto receiveCallback = [&success](const boost::system::error_code &error,
                                      std::size_t bytes_transferred,
                                      std::uint64_t timestamp) {
        std::cout << "Something received" << std::endl;
        std::cout << "Timestamp: " << timestamp << std::endl;
        if (error)
        {
            success = false;
        }
        else
        {
            success = true;
        }
    };
    printBuffer(buffer);

    std::cout << "Before receive" << std::endl;
    udpHost.asyncReceiveAudioBuffer(buffer,
                                    std::bind(receiveCallback,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3));
    ioContext.run();
    std::cout << "After receive" << std::endl;
    printBuffer(buffer);
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
        boost::asio::ip::address::from_string("::1"),
        8002);
    REQUIRE_NOTHROW(udpHost.sendAudioBuffer(buffer, remoteEndpoint));
}

// TEST_CASE("UdpHost | recieveAudioBuffer timeout")
// {
//     boost::asio::io_context ioContext;
//     UdpHost udpHost;
//     udpHost.setupSocket(ioContext, 8001);
//     juce::AudioBuffer<float> buffer(2, 10);
//     bool success = true;

//     udpHost.receiveAudioBuffer(buffer,
//                                [](const boost::system::error_code &error,
//                                   std::size_t bytes_transferred) {
//                                    if (error)
//                                    {
//                                        std::cout << "erro" << std::endl;
//                                        FAIL("Error receiving data");
//                                    }
//                                    else
//                                    {
//                                        std::cout << "data" << std::endl;
//                                        FAIL("Data received");
//                                    }
//                                });

//     std::this_thread::sleep_for(std::chrono::milliseconds(200));
//     // udpHost.cancelReceive();
//     std::cout << "end" << std::endl;
//     FAIL("Timeout");
// }
