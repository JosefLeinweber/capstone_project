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

void UdpHost::setupSocket(boost::asio::io_context &ioContext, int32_t port)
{
    try
    {
        if (port == 0)
        {
            throw std::runtime_error("Invalid port number");
        }
        m_socket = std::make_unique<boost::asio::ip::udp::socket>(
            ioContext,
            boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v6(),
                                           static_cast<unsigned short>(port)));
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
    std::size_t length =
        buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();

    // Create a buffer to hold the timestamp and the audio data
    std::vector<uint8_t> sendBuffer(sizeof(std::uint64_t) + length);

    // Get the current timestamp
    std::uint64_t timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();

    // Copy the timestamp into the send buffer
    std::memcpy(sendBuffer.data(), &timestamp, sizeof(std::uint64_t));

    // Copy the audio data into the send buffer
    std::memcpy(sendBuffer.data() + sizeof(std::uint64_t), data, length);

    try
    {
        std::size_t len = m_socket->send_to(boost::asio::buffer(sendBuffer),
                                            remoteEndpoint,
                                            0,
                                            m_ignoredError);
        if (m_ignoredError)
        {
            throw std::runtime_error("Error sending data");
        }
        if (len <= 0 || len != sendBuffer.size())
        {
            throw std::runtime_error("Failed to send all data");
        }
    }
    catch (const std::exception &e)
    {
        throw e;
    }
};


//TODO: change name to asyncReceiveAudioBuffer
void UdpHost::asyncReceiveAudioBuffer(
    juce::AudioBuffer<float> &buffer,
    std::function<void(const boost::system::error_code &error,
                       std::size_t bytes_transferred,
                       std::uint64_t timestamp)> handler)
{
    // Calculate the length of the audio data
    std::size_t length =
        buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();

    // Create a buffer to hold the timestamp and the audio data
    std::vector<uint8_t> recvBuffer(sizeof(std::uint64_t) + length);

    m_socket->async_receive_from(
        boost::asio::buffer(recvBuffer),
        m_remoteEndpoint,
        [this, &buffer, handler, recvBuffer](
            const boost::system::error_code &error,
            std::size_t bytes_transferred) {
            if (!error && bytes_transferred >= sizeof(std::uint64_t))
            {
                // Extract the timestamp from the received buffer
                std::uint64_t timestamp;
                std::memcpy(&timestamp,
                            recvBuffer.data(),
                            sizeof(std::uint64_t));

                // Extract the audio data from the received buffer
                float *data = buffer.getWritePointer(0);
                std::memcpy(data,
                            recvBuffer.data() + sizeof(std::uint64_t),
                            bytes_transferred - sizeof(std::uint64_t));

                // Call the handler with the timestamp
                handler(error,
                        bytes_transferred - sizeof(std::uint64_t),
                        timestamp);
            }
            else
            {
                // Call the handler with an error
                handler(error, 0, 0);
            }
        });
}

void UdpHost::cancelReceive()
{
    m_socket->cancel();
};
