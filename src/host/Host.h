#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 
#include "AudioBuffer.h"

struct addressData {
  std::string ip;
  int port;
};

typedef boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO> rcv_timeout_option;

class Host{
public:
    Host(addressData hostAddress);

    void sendHandshake(addressData remoteAddress);

    bool waitForHandshake();

    void sendTo(juce::AudioBuffer<float> buffer);

    void recieveFrom(juce::AudioBuffer<float>& buffer);

    void stopHost();

    bool isConnected();

    addressData getRemoteAddress();

private:
    addressData m_hostAddress;
    boost::asio::io_context m_io_context;
    std::unique_ptr<boost::asio::ip::udp::socket> m_socket;
    boost::asio::ip::udp::endpoint m_remote_endpoint;
    std::array<char, 128> m_recv_buf;
    juce::AudioBuffer<float> m_tempBuffer;
    boost::system::error_code m_ignored_error;
    bool m_connected = false;
};