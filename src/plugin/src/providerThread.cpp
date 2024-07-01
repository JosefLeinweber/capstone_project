#include "ConnectDAWs/providerThread.h"


ProviderThread::ProviderThread(ConfigurationData remoteConfigurationData,
                               ConfigurationData localConfigurationData,
                               AudioBufferFIFO &outputRingBuffer,

                               const std::string threadName)
    : juce::Thread(threadName),
      m_remoteConfigurationData(remoteConfigurationData),
      m_localConfigurationData(localConfigurationData),
      m_outputRingBuffer(outputRingBuffer)
{
    m_outputBuffer.setSize(m_localConfigurationData.num_output_channels(),
                           m_localConfigurationData.samples_per_block());
    m_outputBuffer.clear();
};

ProviderThread::~ProviderThread()
{

    if (isThreadRunning())
    {
        signalThreadShouldExit();
        waitForThreadToExit(1000);
    }
    std::cout << "ProviderThread | Destructor" << std::endl;
}


void ProviderThread::run()
{
    setupHost();
    while (!threadShouldExit())
    {
        if (readFromFIFOBuffer(std::chrono::milliseconds(2000)))
        {
            sendAudioToRemoteConsumer();
        }
        else
        {
            signalThreadShouldExit();
        }
    }
};

bool ProviderThread::readFromFIFOBuffer(std::chrono::milliseconds timeout)
{
    auto start = std::chrono::high_resolution_clock::now();
    //TODO: change 0 to not be hardcoded
    // Only read from
    while (m_outputRingBuffer.getNumReady() < m_outputBuffer.getNumSamples())

    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (threadShouldExit())
        {
            std::cout << "ProviderThread | readFromFIFOBuffer | "
                         "threadShouldExit"
                      << std::endl;
            return false;
        }
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start) > timeout)
        {
            std::cout << "ProviderThread | readFromFIFOBuffer | "
                         "timeout, only "
                      << m_outputRingBuffer.getNumReady()
                      << " samples ready to be read" << std::endl;
            return false;
        }
    }
    m_outputRingBuffer.readFromInternalBufferTo(m_outputBuffer);
    return true;
}

bool ProviderThread::sendAudioToRemoteConsumer()
{
    try
    {
        m_udpHost->sendAudioBuffer(
            m_outputBuffer,
            boost::asio::ip::udp::endpoint(
                boost::asio::ip::address::from_string(
                    m_remoteConfigurationData.ip()),
                m_remoteConfigurationData.consumer_port()));
        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << "ProviderThread | sendAudioToRemoteConsumer | "
                     "Failed to send data: "
                  << e.what() << std::endl;
        return false;
    }
};

void ProviderThread::setupHost()
{
    try
    {
        m_udpHost = std::make_unique<UdpHost>();
        m_udpHost->setupSocket(m_ioContext,
                               m_localConfigurationData.provider_port());
    }
    catch (const std::exception &e)
    {
        std::cout << "ProviderThread | setupHost | "
                     "Failed to setup host: "
                  << e.what() << std::endl;
        signalThreadShouldExit();
    }
}
