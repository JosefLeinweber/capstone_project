#include "Host.h"
#include <chrono>
#include <iostream>

void printBuffer(auto &buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        std::cout << "Channel " << channel << ": ";
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            std::cout << buffer.getSample(channel, i) << " ";
        }
        std::cout << std::endl;
    }
}


Host::Host() {};

Host::~Host()
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

void Host::setupSocket(boost::asio::io_context &ioContext,
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


void Host::sendAudioBuffer(juce::AudioBuffer<float> buffer,
                           boost::asio::ip::udp::endpoint remoteEndpoint)
{
    const float *data = buffer.getReadPointer(0);
    // float* data2 = tempBuffer.getReadPointer(1);
    std::size_t length =
        buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();

    try
    {
        std::cout << "Sending data..." << std::endl;
        printBuffer(buffer);
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
        std::cout << "Sent " << len << " bytes" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to send data: " << e.what() << std::endl;
        throw e;
    }
};

bool Host::receiveAudioBuffer(juce::AudioBuffer<float> &buffer)
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
        std::cout << "Received " << len << " bytes" << std::endl;
        std::cout << "Received data..." << std::endl;
        printBuffer(buffer);
        return true;
    }
    else
    {
        std::cout << "Failed to receive data" << std::endl;
        return false;
    }
};
