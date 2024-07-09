#include "ConnectDAWs/providerThread.h"


ProviderThread::ProviderThread(ConfigurationData remoteConfigurationData,
                               ConfigurationData localConfigurationData,
                               AudioBufferFIFO &outputRingBuffer,
                               std::chrono::milliseconds timeout,
                               const std::string threadName)
    : juce::Thread(threadName), m_timeout(timeout),
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
        if (readFromFIFOBuffer(m_timeout))
        {
            sendAudioToRemoteConsumer();
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
            return false;
        }
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start) > timeout)
        {
            signalThreadShouldExit();
            return false;
        }
    }
    m_outputRingBuffer.write(m_outputBuffer);
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
    catch (...)
    {
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
    catch (...)
    {
        signalThreadShouldExit();
    }
}
