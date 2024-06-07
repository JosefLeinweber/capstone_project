#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 
#include "AudioBuffer.h"

class Client{
    public:
        Client(AudioBufferFIFO& audioBufferFIFOArg);

        void connectToServer(std::string ip, std::string port);

        void sendDataToServer();

        void receiveDataFromServer();

        void stopClient();

    private:
        AudioBufferFIFO& m_audioBufferFIFO;
        boost::asio::io_context m_io_context;
        boost::asio::ip::udp::socket m_socket;
        boost::asio::ip::udp::endpoint m_remote_endpoint;
        std::array<char, 128> m_recv_buf;
        juce::AudioBuffer<float> m_tempBuffer;
        boost::system::error_code m_ignored_error;
}