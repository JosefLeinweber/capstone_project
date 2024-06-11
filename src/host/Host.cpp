#include "Host.h"
#include <chrono>
#include <iostream>


Host::Host(addressData hostAddress) : m_hostAddress(hostAddress) {};

Host::~Host()
{
    stopHost();
};

void Host::setupSocket()
{
    m_socket = std::make_unique<boost::asio::ip::udp::socket>(
        m_io_context,
        boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),
                                       m_hostAddress.port));
    m_socket->set_option(rcv_timeout_option{1500});
};

bool Host::waitForHandshake()
{

    std::size_t length = m_socket->receive_from(boost::asio::buffer(m_recv_buf),
                                                m_remote_endpoint,
                                                0,
                                                m_ignored_error);
    if (length > 0)
    {
        std::cout << "Handshake received" << std::endl;
        std::cout << length << std::endl;
        std::cout << m_recv_buf.data() << std::endl;
        m_connected = true;
    }
    else
    {
        m_connected = false;
    }
    return m_connected;
};

bool Host::isConnected()
{
    return m_connected;
};

void Host::sendHandshake(addressData remoteAddress)
{
    m_remote_endpoint = boost::asio::ip::udp::endpoint(
        boost::asio::ip::address::from_string(remoteAddress.ip),
        remoteAddress.port);
    std::array<char, 1> send_buf = {{0}};
    m_socket->send_to(boost::asio::buffer(send_buf), m_remote_endpoint);
};

void Host::sendTo(juce::AudioBuffer<float> buffer)
{
    const float *data = buffer.getReadPointer(0);
    // float* data2 = tempBuffer.getReadPointer(1);
    std::size_t length =
        buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();
    m_socket->send_to(boost::asio::buffer(data, length),
                      m_remote_endpoint,
                      0,
                      m_ignored_error);
};

void Host::recieveFrom(juce::AudioBuffer<float> &buffer)
{
    float *data = buffer.getWritePointer(0);
    // float* data2 = tempBuffer.getReadPointer(1);
    std::size_t length =
        buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();

    m_socket->receive_from(boost::asio::buffer(data, length),
                           m_remote_endpoint,
                           0,
                           m_ignored_error);
};

void Host::stopHost()
{
    if (m_socket)
    {
        m_socket->close();
    }
};

addressData Host::getRemoteAddress()
{
    addressData remoteAddress(m_remote_endpoint.address().to_string(),
                              m_remote_endpoint.port());
    return remoteAddress;
}

void Host::asyncWaitForConnection(
    std::function<void(const boost::system::error_code &error,
                       std::size_t bytes_transferred)> callback)
{
    m_socket->async_receive_from(boost::asio::buffer(m_recv_buf),
                                 m_remote_endpoint,
                                 callback);
    m_io_context.run();
};
