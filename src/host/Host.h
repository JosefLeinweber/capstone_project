#pragma once

#include "AudioBuffer.h"
#include "datagram.pb.h"
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <juce_core/juce_core.h>

typedef boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>
    rcv_timeout_option;

class Host
{
public:
    Host();

    ~Host();

    void setupSocket(boost::asio::io_context &ioContext,
                     unsigned short port,
                     unsigned short rec_timeout = 1000);

    void sendAudioBuffer(juce::AudioBuffer<float> buffer,
                         boost::asio::ip::udp::endpoint remoteEndpoint);

    bool receiveAudioBuffer(juce::AudioBuffer<float> &buffer);

private:
    ConfigurationData m_configurationData;

    boost::asio::io_context m_ioContext;
    boost::system::error_code m_ignoredError;
    boost::asio::ip::udp::endpoint m_remoteEndpoint;

    std::unique_ptr<boost::asio::ip::udp::socket> m_socket;
    std::unique_ptr<boost::asio::steady_timer> m_timer;

    juce::AudioBuffer<float> m_tempBuffer;
};
