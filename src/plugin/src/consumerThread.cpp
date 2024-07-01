#include "ConnectDAWs/consumerThread.h"


ConsumerThread::ConsumerThread(ConfigurationData remoteConfigurationData,
                               ConfigurationData localConfigurationData,
                               AudioBufferFIFO &inputRingBuffer,
                               const std::string threadName)

    : juce::Thread(threadName),
      m_remoteConfigurationData(remoteConfigurationData),
      m_localConfigurationData(localConfigurationData),
      m_inputRingBuffer(inputRingBuffer)
{
    m_inputBuffer.setSize(localConfigurationData.num_input_channels(),
                          localConfigurationData.samples_per_block());
    m_inputBuffer.clear();
};

ConsumerThread::~ConsumerThread()
{
    if (isThreadRunning())
    {
        signalThreadShouldExit();
        waitForThreadToExit(1000);
    }
    std::cout << "ConsumerThread | Destructor" << std::endl;
}

void ConsumerThread::run()
{
    setupHost(std::chrono::milliseconds(2000));
    while (!threadShouldExit())
    {
        if (receiveAudioFromRemoteProvider())
        {
            writeToFIFOBuffer();
        }
        else
        {
            signalThreadShouldExit();
        }
    }
};

void ConsumerThread::setupHost(std::chrono::milliseconds timeout)
{
    try
    {
        m_udpHost = std::make_unique<UdpHost>();
        m_udpHost->setupSocket(m_ioContext,
                               m_localConfigurationData.consumer_port(),
                               timeout.count());
    }
    catch (std::exception &e)
    {
        std::cout << "ConsumerThread | setupHost | Exception: " << e.what()
                  << std::endl;
        signalThreadShouldExit();
    }
};

bool ConsumerThread::receiveAudioFromRemoteProvider()
{
    try
    {
        if (m_udpHost->receiveAudioBuffer(m_inputBuffer))
        {
            return true;
        }
        else
        {
            std::cout << "ConsumerThread | receiveAudioFromRemoteProvider | "
                         "Failed to receive audio buffer"
                      << std::endl;
            return false;
        }
    }
    catch (std::exception &e)
    {
        std::cout << "ConsumerThread | receiveAudioFromRemoteProvider | "
                     "Exception: "
                  << e.what() << std::endl;
        return false;
    }
};

void ConsumerThread::writeToFIFOBuffer()
{
    return m_inputRingBuffer.writeToInternalBufferFrom(m_inputBuffer);
};
