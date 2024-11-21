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
    // the ip will be send to the remote without encription currently
    try
    {
        // Step 1: Create an io_context object
        boost::asio::io_context io_context;

        // Step 2: Resolve the server name
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve("api6.ipify.org", "http");

        // Step 3: Create and connect the socket
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Step 4: Form the HTTP GET request
        std::string request = "GET /?format=text HTTP/1.1\r\n";
        request += "Host: api6.ipify.org\r\n";
        request += "Connection: close\r\n\r\n";

        // Step 5: Send the request
        boost::asio::write(socket, boost::asio::buffer(request));

        // Step 6: Read the response
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Step 7: Process the response
        std::string header;
        std::istream response_stream(&response);
        while (std::getline(response_stream, header) && header != "\r")
        {
            // Discard header lines
        }

        // Read the remaining data which contains the IP address
        std::string ip;
        std::getline(response_stream, ip);
        return ip;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return std::string();
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

void ConnectDAWs::initRingBuffers(int numInputChannels,
                                  int numOutputChannels,
                                  int samplesPerBlock)
{
    //TODO: remove magic number (10) determin what the best value is and set a constant
    int bufferSize = samplesPerBlock * 10;
    m_inputRingBuffer =
        std::make_shared<RingBuffer>(numInputChannels, bufferSize);
    m_outputRingBuffer =
        std::make_shared<RingBuffer>(numOutputChannels, bufferSize);
}

void ConnectDAWs::startUpConnectionManagerThread(double sampleRate,
                                                 int samplesPerBlock,
                                                 int numInputChannels,
                                                 int numOutputChannels)
{
    initRingBuffers(numInputChannels, numOutputChannels, samplesPerBlock);

    setLocalConfigurationData(sampleRate,
                              samplesPerBlock,
                              numInputChannels,
                              numOutputChannels);

    m_connectionManagerThread =
        std::make_unique<ConnectionManagerThread>(m_guiMessenger,
                                                  m_cmtMessenger,
                                                  m_localConfigurationData,
                                                  *m_inputRingBuffer,
                                                  *m_outputRingBuffer,
                                                  m_startConnection,
                                                  m_stopConnection,
                                                  m_benchmark);
    m_connectionManagerThread->startThread(juce::Thread::Priority::high);
}


void ConnectDAWs::releaseResources()
{
    if (m_connectionManagerThread != nullptr)
    {
        m_connectionManagerThread->signalThreadShouldExit();
        m_connectionManagerThread->waitForThreadToExit(1000);
        m_connectionManagerThread.reset();
    }
}

void ConnectDAWs::processBlock(juce::AudioBuffer<float> &buffer)
{
    m_outputRingBuffer->copyFrom(buffer);

    buffer.clear();

    m_inputRingBuffer->copyTo(buffer);
}
