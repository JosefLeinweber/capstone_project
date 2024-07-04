#include "ConnectDAWs/connectDAWs.h"

ConnectDAWs::ConnectDAWs()
{
}

ConnectDAWs::~ConnectDAWs()
{
}


std::string ConnectDAWs::getIp()
{
    //TODO: is this save?
    try
    {
        // std::string public_ip;
        // using namespace boost::asio::ip;
        // boost::asio::io_context ioContext;
        // boost::asio::streambuf request_buffer;

        // // Send the request
        // boost::asio::ip::tcp::resolver resolver(ioContext);
        // boost::asio::ip::tcp::resolver::query query("api.ipify.org", "http");
        // boost::asio::ip::tcp::socket socket(ioContext);

        // boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
        //     resolver.resolve(query);
        // boost::asio::connect(socket, endpoint_iterator);

        // // Receive the response
        // boost::asio::streambuf response_buffer;
        // boost::asio::read_until(socket, response_buffer, "\r\n\r\n");

        // // Extract the IP address from the response (assuming it's the entire body)
        // std::istream response_stream(&response_buffer);
        // std::getline(response_stream, public_ip);

        // // Close the socket
        // socket.close();
        // return public_ip;
        return "will be implemented in the future";
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        std::string();
    }
}


void ConnectDAWs::setLocalConfigurationData(double sampleRate,
                                            int samplesPerBlock,
                                            int numInputChannels,
                                            int numOutputChannels)
{
    m_localConfigurationData.set_ip(getIp());
    m_localConfigurationData.set_host_port(7000);
    m_localConfigurationData.set_consumer_port(7001);
    m_localConfigurationData.set_provider_port(7002);
    m_localConfigurationData.set_samples_per_block(samplesPerBlock);
    m_localConfigurationData.set_sample_rate(sampleRate);
    m_localConfigurationData.set_num_input_channels(numInputChannels);
    m_localConfigurationData.set_num_output_channels(numOutputChannels);
}

void ConnectDAWs::initFIFOBuffers(int numInputChannels,
                                  int numOutputChannels,
                                  int samplesPerBlock)
{
    //TODO: remove magic number (10) determin what the best value is and set a constant
    int bufferSize = samplesPerBlock * 10;
    m_inputBufferFIFO =
        std::make_shared<AudioBufferFIFO>(numInputChannels, bufferSize);
    m_outputBufferFIFO =
        std::make_shared<AudioBufferFIFO>(numOutputChannels, bufferSize);
}

void ConnectDAWs::prepareToPlay(double sampleRate,
                                int samplesPerBlock,
                                int numInputChannels,
                                int numOutputChannels)
{
    initFIFOBuffers(numInputChannels, numOutputChannels, samplesPerBlock);

    setLocalConfigurationData(sampleRate,
                              samplesPerBlock,
                              numInputChannels,
                              numOutputChannels);

    m_connectionManagerThread =
        std::make_unique<ConnectionManagerThread>(m_guiMessenger,
                                                  m_cmtMessenger,
                                                  m_localConfigurationData,
                                                  *m_inputBufferFIFO,
                                                  *m_outputBufferFIFO,
                                                  m_startConnection,
                                                  m_stopConnection);
    m_connectionManagerThread->startThread(juce::Thread::Priority::high);
}


void ConnectDAWs::releaseResources()
{
    if (m_connectionManagerThread != nullptr)
    {
        m_connectionManagerThread->signalThreadShouldExit();
        m_connectionManagerThread->waitForThreadToExit(1000);
    }
}

void ConnectDAWs::processBlock(juce::AudioBuffer<float> &buffer)
{
    m_outputBufferFIFO->writeToInternalBufferFrom(buffer);

    buffer.clear();

    m_inputBufferFIFO->readFromInternalBufferTo(buffer);
}
