#include "ConnectDAWs/providerThread.h"


ProviderThread::ProviderThread(ConfigurationData remoteConfigurationData,
                               ConfigurationData localConfigurationData,
                               RingBuffer &outputRingBuffer,
                               std::shared_ptr<Benchmark> &benchmark,
                               std::chrono::milliseconds timeout,
                               const std::string threadName)
    : juce::Thread(threadName), m_timeout(timeout),
      m_remoteConfigurationData(remoteConfigurationData),
      m_localConfigurationData(localConfigurationData),
      m_outputRingBuffer(outputRingBuffer), m_benchmark(benchmark)
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
        if (readFromRingBuffer(m_timeout))
        {
            sendAudioToRemoteConsumer();
        }
    }
};

bool ProviderThread::readFromRingBuffer(std::chrono::milliseconds timeout)
{
    auto start = std::chrono::high_resolution_clock::now();
    //TODO: change 0 to not be hardcoded
    // Only read from
    while (m_outputRingBuffer.getNumReadyToRead() <
           m_outputBuffer.getNumSamples())

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
    m_outputRingBuffer.copyTo(m_outputBuffer);
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
                static_cast<unsigned short>(
                    m_remoteConfigurationData.consumer_port())));
        // adding approximatly +250ms delay
        m_benchmark->addDelay(38);
        m_benchmark->m_pluginOutgoingBenchmark.recordEndTimestamp();
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
