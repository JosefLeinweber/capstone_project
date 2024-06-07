#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 
#include "AudioBuffer.h"

struct datagram {
    std::vector<float> buffer1;
    std::vector<float> buffer2;
};


class Server{
public:
    Server(std::string ip, int port);

    void inititalizeConnection(std::string ip, int port);

    void waitForConnection();

    void sendTo(juce::AudioBuffer<float> buffer);

    void recieveFrom(juce::AudioBuffer<float>& buffer);

    void stopServer();

    bool isConnected();

private:

    

    std::string m_ip;
    int m_port;
    boost::asio::io_context m_io_context;
    std::unique_ptr<boost::asio::ip::udp::socket> m_socket;
    boost::asio::ip::udp::endpoint m_remote_endpoint;
    std::array<char, 128> m_recv_buf;
    juce::AudioBuffer<float> m_tempBuffer;
    boost::system::error_code m_ignored_error;
    bool m_connected = false;
};