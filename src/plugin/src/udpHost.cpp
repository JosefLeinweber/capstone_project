#include "ConnectDAWs/UdpHost.h"

UdpHost::UdpHost() {};

UdpHost::~UdpHost()
{
    if (m_socket)
    {
        try
        {
            m_socket->close();
        }
        catch (...)
        {
            std::cout << "Socket already closed" << std::endl;
        }
    }
};

void UdpHost::setupSocket(boost::asio::io_context &ioContext,
                          unsigned short port,
                          unsigned short rec_timeout)
{
    try
    {
        if (port == 0)
        {
            throw std::runtime_error("Invalid port number");
        }
        m_socket = std::make_unique<boost::asio::ip::udp::socket>(
            ioContext,
            boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
        m_socket->set_option(rcv_timeout_option{rec_timeout});
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to create socket: " << e.what() << std::endl;
        throw e;
    }
};


void UdpHost::sendAudioBuffer(juce::AudioBuffer<float> buffer,
                              boost::asio::ip::udp::endpoint remoteEndpoint)
{
    const float *data = buffer.getReadPointer(0);
    // float* data2 = tempBuffer.getReadPointer(1);
    std::size_t length =
        buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();

    try
    {

        std::size_t len = m_socket->send_to(boost::asio::buffer(data, length),
                                            remoteEndpoint,
                                            0,
                                            m_ignoredError);
        if (m_ignoredError)
        {
            throw std::runtime_error("Error sending data");
        }
        if (len <= 0 || len != length)
        {
            throw std::runtime_error("Failed to send all data");
        }
    }
    catch (const std::exception &e)
    {
        throw e;
    }
};

bool UdpHost::receiveAudioBuffer(juce::AudioBuffer<float> &buffer)
{

    float *data = buffer.getWritePointer(0);
    // float* data2 = tempBuffer.getReadPointer(1);
    std::size_t length =
        buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();


    std::size_t len = m_socket->receive_from(boost::asio::buffer(data, length),
                                             m_remoteEndpoint,
                                             0,
                                             m_ignoredError);

    if (!m_ignoredError && len > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
};
