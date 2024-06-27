#include "ConnectDAWs/providerThread.h"
#include <chrono>
#include <iostream>


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
    std::cout << "m_outputBzffer.getNumSamples(): "
              << m_outputBuffer.getNumSamples() << std::endl;
    std::cout << "m_outputRingBuffer.getNumReady(): "
              << m_outputRingBuffer.getNumReady() << std::endl;
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
                         "timeout, not enough ready samples in buffer"
                      << std::endl;
            return false;
        }
    }
    std::cout << "ProviderThread | readFromFIFOBuffer | numReady: "
              << m_outputRingBuffer.getNumReady() << std::endl;
    m_outputRingBuffer.readFromInternalBufferTo(m_outputBuffer);
    return true;
}

bool ProviderThread::sendAudioToRemoteConsumer()
{
    std::cout << "ProviderThread | sendAudioToRemoteConsumer" << std::endl;

    try
    {
        std::cout << "ProviderThread | sendAudioToRemoteConsumer | "
                     "send the following buffer: "
                  << std::endl;
        m_host->sendAudioBuffer(m_outputBuffer,
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
        m_host = std::make_unique<Host>();
        m_host->setupSocket(m_ioContext,
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
