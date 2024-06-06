#include "Server.h"



Server::Server(std::string ip, int port) : m_ip(ip), m_port(port){};

void Server::waitForConnection(){
    m_socket = std::make_unique<boost::asio::ip::udp::socket>(m_io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), m_port));
    m_socket->receive_from(boost::asio::buffer(m_recv_buf), m_remote_endpoint);
    m_connected = true;
};

bool Server::isConnected(){
    return m_connected;
};

void Server::inititalizeConnection(std::string ip, int port){
    m_socket = std::make_unique<boost::asio::ip::udp::socket>(m_io_context);

    m_remote_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(ip), port);
    m_socket->open(boost::asio::ip::udp::v4());
    std::array<char, 1> send_buf  = {{ 0 }};
    m_socket->send_to(boost::asio::buffer(send_buf), m_remote_endpoint);
};

void Server::sendTo(juce::AudioBuffer<float> buffer){
        const float* data = buffer.getReadPointer(0);
        // float* data2 = tempBuffer.getReadPointer(1);
        std::size_t length = buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();
        m_socket->send_to(boost::asio::buffer(data, length), m_remote_endpoint, 0, m_ignored_error);

};

void Server::recieveFrom(juce::AudioBuffer<float>& buffer){
    float* data = buffer.getWritePointer(0);
    // float* data2 = tempBuffer.getReadPointer(1);
    std::size_t length = buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();

    m_socket->receive_from(boost::asio::buffer(data, length), m_remote_endpoint, 0, m_ignored_error);

};


