#include "ConnectDAWs/consumerThread.h"


ConsumerThread::ConsumerThread(ConfigurationData remoteConfigurationData,
                               ConfigurationData localConfigurationData,
                               AudioBufferFIFO &inputRingBuffer,
                               std::chrono::milliseconds timeout,
                               const std::string threadName)

    : juce::Thread(threadName), m_timeout(timeout),
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
    if (m_ioContextThread.joinable())
    {
        m_ioContextThread.join();
    }
    if (!m_ioContext.stopped())
    {
        m_ioContext.stop();
    }
    std::cout << "ConsumerThread | Destructor" << std::endl;
}

void ConsumerThread::run()
{
    setupHost();
    while (!threadShouldExit())
    {
        if (receiveAudioFromRemoteProvider(m_timeout))
        {
            std::cout << "ConsumerThread | run | Writing audio to FIFO buffer"
                      << std::endl;
            writeToFIFOBuffer();
        }
        else
        {
            std::cout << "ConsumerThread | run | Error receiving audio from "
                         "remote provider"
                      << std::endl;
        }
    }
};

void ConsumerThread::setupHost()
{
    try
    {
        m_udpHost = std::make_unique<UdpHost>();
        m_udpHost->setupSocket(m_ioContext,
                               m_localConfigurationData.consumer_port());
    }
    catch (std::exception &e)
    {

        signalThreadShouldExit();
    }
};

void ConsumerThread::startIOContextInDifferentThread()
{
    m_ioContextThread = std::jthread([&]() { m_ioContext.run_one(); });
};

bool ConsumerThread::receiveAudioFromRemoteProvider(
    std::chrono::milliseconds timeout)
{
    m_udpHost->receiveAudioBuffer(m_inputBuffer,
                                  std::bind(&ConsumerThread::receiveHandler,
                                            this,
                                            std::placeholders::_1,
                                            std::placeholders::_2));
    startIOContextInDifferentThread();
    auto start = std::chrono::high_resolution_clock::now();
    while (!m_receivedData)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (threadShouldExit())
        {
            m_udpHost->cancelReceive();
            return false;
        }
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start)
                .count() > timeout.count())
        {
            m_udpHost->cancelReceive();
            return false;
        }
    }

    m_receivedData = false;
    return true;
};

void ConsumerThread::receiveHandler(const boost::system::error_code &error,
                                    std::size_t bytes_transferred)
{
    if (!error && bytes_transferred > 0)
    {
        m_receivedData = true;
    }
    else
    {
        m_receivedData = false;
    }
}

void ConsumerThread::writeToFIFOBuffer()
{
    return m_inputRingBuffer.writeToInternalBufferFrom(m_inputBuffer);
};
