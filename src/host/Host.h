#pragma once

#include "AudioBuffer.h"
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>


struct addressData
{
    std::string ip;
    int port;
};

typedef boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>
    rcv_timeout_option;

class Host
{
public:
    Host(addressData hostAddress);

    ~Host();

    void setupSocket();

    void sendHandshake(addressData remoteAddress);

    bool waitForHandshake();

    void sendTo(juce::AudioBuffer<float> buffer);

    void recieveFrom(juce::AudioBuffer<float> &buffer);

    void stopHost();

    bool isConnected();

    void asyncWaitForConnection(
        std::function<void(const boost::system::error_code &error,
                           std::size_t bytes_transferred)> callback,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(1500));

    addressData getRemoteAddress();

private:
    addressData m_hostAddress;
    boost::asio::io_context m_io_context;
    std::unique_ptr<boost::asio::ip::udp::socket> m_socket;
    boost::asio::ip::udp::endpoint m_remote_endpoint;
    std::array<char, 128> m_recv_buf;
    juce::AudioBuffer<float> m_tempBuffer;
    boost::system::error_code m_ignored_error;
    std::unique_ptr<boost::asio::steady_timer> m_timer;
    bool m_connected = false;
};
