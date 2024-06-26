#include "ConnectDAWs/consumerThread.h"
#include <iostream>
#include <thread>


ConsumerThread::ConsumerThread(ConfigurationData remoteConfigurationData,
                               ConfigurationData localConfigurationData,
                               AudioBufferFIFO &inputRingBuffer,
                               const std::string threadName)

    : juce::Thread(threadName),
      m_remoteConfigurationData(remoteConfigurationData),
      m_localConfigurationData(localConfigurationData),
      m_inputRingBuffer(inputRingBuffer)
{
    m_inputBuffer.setSize(m_inputRingBuffer.buffer.getNumChannels(),
                          m_inputRingBuffer.buffer.getNumSamples() / 4);
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
        m_host = std::make_unique<Host>();
        m_host->setupSocket(m_ioContext,
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
    std::cout << "ConsumerThread | receiveAudioFromRemoteProvider" << std::endl;
    try
    {
        bool success = m_host->receiveAudioBuffer(m_inputBuffer);
        if (!success)
        {
            std::cout << "ConsumerThread | receiveAudioFromRemoteProvider | "
                         "Failed to receive audio buffer"
                      << std::endl;
            return false;
        }
        std::cout << "ConsumerThread | receiveAudioFromRemoteProvider | "
                     "Received audio buffer"
                  << std::endl;
        return success;
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
