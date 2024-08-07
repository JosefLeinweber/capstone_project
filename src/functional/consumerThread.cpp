#include "ConnectDAWs/consumerThread.h"


ConsumerThread::ConsumerThread(ConfigurationData remoteConfigurationData,
                               ConfigurationData localConfigurationData,
                               RingBuffer &inputRingBuffer,
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
    if (m_workGuard)
    {
        m_workGuard->reset();
    }

    if (!m_ioContext.stopped())
    {
        m_ioContext.stop();
    }
    if (m_ioContextThread.joinable())
    {
        m_ioContextThread.join();
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

            writeToRingBuffer();
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
        m_workGuard = std::make_shared<boost::asio::executor_work_guard<
            boost::asio::io_context::executor_type>>(
            boost::asio::make_work_guard(m_ioContext));
        startIOContextInDifferentThread();
    }
    catch (...)
    {

        signalThreadShouldExit();
    }
};

void ConsumerThread::startIOContextInDifferentThread()
{

    m_ioContextThread = std::jthread([&]() { m_ioContext.run(); });
};

bool ConsumerThread::receiveAudioFromRemoteProvider(
    std::chrono::milliseconds timeout)
{
    m_udpHost->receiveAudioBuffer(m_inputBuffer,
                                  std::bind(&ConsumerThread::receiveHandler,
                                            this,
                                            std::placeholders::_1,
                                            std::placeholders::_2));

    auto startTime = std::chrono::high_resolution_clock::now();
    while (!m_receivedData)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (threadShouldExit())
        {
            std::cout << "ConsumerThread | receiveAudioFromRemoteProvider | "
                         "Thread should exit"
                      << std::endl;
            m_udpHost->cancelReceive();
            return false;
        }
        if (timeOut(timeout, startTime))
        {
            std::cout << "ConsumerThread | receiveAudioFromRemoteProvider | "
                         "Timeout"
                      << std::endl;
            signalThreadShouldExit();
            m_udpHost->cancelReceive();
            return false;
        }
    }

    m_receivedData = false;
    return true;
};

bool ConsumerThread::timeOut(
    std::chrono::milliseconds timeout,
    std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now() - start) > timeout;
};

void ConsumerThread::receiveHandler(const boost::system::error_code &error,
                                    std::size_t bytes_transferred)
{
    // to avoid unsuded warning
    (void)bytes_transferred;

    if (!error)
    {
        m_receivedData = true;
    }
    else
    {
        m_receivedData = false;
    }
}

void ConsumerThread::writeToRingBuffer()
{
    return m_inputRingBuffer.copyFrom(m_inputBuffer);
};
