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

int64_t UdpHost::getTimestamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
};

std::vector<uint8_t> UdpHost::timestampToBytes(int64_t timestamp)
{
    std::vector<uint8_t> bytes(sizeof(timestamp));
    std::memcpy(bytes.data(), &timestamp, sizeof(timestamp));
    return bytes;
}

// Function to convert an AudioBuffer to a byte array
std::vector<uint8_t> UdpHost::audioBufferToBytes(
    const juce::AudioBuffer<float> &buffer)
{
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    std::vector<uint8_t> bytes(numChannels * numSamples * sizeof(float));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        std::memcpy(bytes.data() + ch * numSamples * sizeof(float),
                    buffer.getReadPointer(ch),
                    numSamples * sizeof(float));
    }

    return bytes;
}

std::vector<uint8_t> UdpHost::concatenateBytes(const std::vector<uint8_t> &a,
                                               const std::vector<uint8_t> &b)
{
    std::vector<uint8_t> result;
    result.reserve(a.size() + b.size());
    result.insert(result.end(), a.begin(), a.end());
    result.insert(result.end(), b.begin(), b.end());
    return result;
}


//TODO: extraxt timestamp things and move them to a separate class ?
void UdpHost::sendAudioBuffer(juce::AudioBuffer<float> buffer,
                              boost::asio::ip::udp::endpoint remoteEndpoint)
{
    const float *data = buffer.getReadPointer(0);
    std::size_t length =
        buffer.getNumSamples() * sizeof(float) * buffer.getNumChannels();

    std::int64_t timestamp = getTimestamp();
    std::vector<uint8_t> timestampBytes = timestampToBytes(timestamp);
    std::vector<uint8_t> audioBufferBytes = audioBufferToBytes(buffer);
    std::vector<uint8_t> sendBuffer =
        concatenateBytes(timestampBytes, audioBufferBytes);


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


void UdpHost::asyncReceiveAudioBuffer(
    std::vector<uint8_t> &recvBuffer,
    std::function<void(const boost::system::error_code &error,
                       std::size_t bytes_transferred)> handler)
{
    m_socket->async_receive_from(
        boost::asio::buffer(recvBuffer.data(), recvBuffer.size()),
        m_remoteEndpoint,
        std::bind(handler, std::placeholders::_1, std::placeholders::_2));
}

void UdpHost::cancelReceive()
{
    m_socket->cancel();
};
