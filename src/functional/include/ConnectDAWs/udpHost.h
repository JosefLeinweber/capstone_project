#pragma once

#include "datagram.pb.h"
#include <boost/asio.hpp>
#include <juce_audio_basics/juce_audio_basics.h>

typedef boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>
    receive_timeout_option;


class UdpHost
{
public:
    UdpHost();

    ~UdpHost();

    void setupSocket(boost::asio::io_context &ioContext, int32_t port);

    void sendAudioBuffer(juce::AudioBuffer<float> buffer,
                         boost::asio::ip::udp::endpoint remoteEndpoint);

    void asyncReceiveAudioBuffer(
        juce::AudioBuffer<float> &buffer,
        std::function<void(const boost::system::error_code &error,
                           std::size_t bytes_transferred,
                           std::uint64_t timestamp)> handler);

    void cancelReceive();

    uint64_t getTimestamp();

    std::vector<uint8_t> concatenateBytes(const std::vector<uint8_t> &a,
                                          const std::vector<uint8_t> &b);

    std::vector<uint8_t> audioBufferToBytes(
        const juce::AudioBuffer<float> &buffer);

    std::vector<uint8_t> timestampToBytes(uint64_t timestamp);

private:
    ConfigurationData m_configurationData;

    boost::asio::io_context m_ioContext;
    boost::system::error_code m_ignoredError;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;

    std::unique_ptr<boost::asio::ip::udp::socket> m_socket;
    std::unique_ptr<boost::asio::steady_timer> m_timer;
    juce::AudioBuffer<float> m_tempBuffer;
};
