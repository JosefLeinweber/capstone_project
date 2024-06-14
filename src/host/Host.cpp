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
};

bool Host::waitForHandshake()
{
    m_socket->set_option(rcv_timeout_option{3000});
    std::size_t length = m_socket->receive_from(boost::asio::buffer(m_recv_buf),
                                                m_remote_endpoint,
                                                0,
                                                m_ignored_error);
    if (length > 0)
    {
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
        try
        {
            m_socket->cancel();
        }
        catch (...)
        {
            std::cout << "No async call to cancel" << std::endl;
        }


        m_socket->close();
    }
    m_io_context.stop();
};

addressData Host::getRemoteAddress()
{
    addressData remoteAddress(m_remote_endpoint.address().to_string(),
                              m_remote_endpoint.port());
    return remoteAddress;
}


void Host::asyncWaitForConnection(
    std::function<void(const boost::system::error_code &error,
                       std::size_t bytes_transferred)> callback,
    std::chrono::milliseconds timeout)
{
    m_timer =
        std::make_unique<boost::asio::steady_timer>(m_io_context, timeout);
    m_socket->async_receive_from(
        boost::asio::buffer(m_recv_buf),
        m_remote_endpoint,
        [this, callback](const boost::system::error_code &error,
                         std::size_t bytes_transferred) {
            std::cout << "callback executed at: "
                      << std::chrono::system_clock::now() << std::endl;
            callback(error, bytes_transferred);
            this->m_timer->cancel();
        });


    m_timer->async_wait([this](const boost::system::error_code &error) {
        if (!error)
        {
            std::cout << "Timer expired" << std::endl;
            m_socket->cancel();
        }
    });

    m_io_context.run();
    m_io_context.restart();
};
