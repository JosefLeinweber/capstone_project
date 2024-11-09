#include "ConnectDAWs/consumerThread.h"


ConsumerThread::ConsumerThread(
    ConfigurationData remoteConfigurationData,
    ConfigurationData localConfigurationData,
    RingBuffer &inputRingBuffer,
    std::shared_ptr<std::vector<std::uint64_t>> &differenceBuffer,
    std::chrono::milliseconds timeout,
    const std::string threadName)

    : juce::Thread(threadName), m_timeout(timeout),
      m_remoteConfigurationData(remoteConfigurationData),
      m_localConfigurationData(localConfigurationData),
      m_inputRingBuffer(inputRingBuffer), m_differenceBuffer(differenceBuffer)
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
    //TODO: Implement correct version
    //! TEMPORARY FIX
    std::vector<uint8_t> buffer(
        (m_inputBuffer.getNumChannels() * m_inputBuffer.getNumSamples() *
         sizeof(float)) +
        sizeof(uint64_t));
    m_udpHost->asyncReceiveAudioBuffer(
        buffer,
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

    //TODO: Implement correct version, move this code somewhere else

    std::uint64_t timestamp;
    std::memcpy(&timestamp, buffer.data(), sizeof(timestamp));

    if (m_differenceBuffer != nullptr)
    {


        std::uint64_t currentTimestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();

        std::uint64_t difference = currentTimestamp - timestamp;


        // 1. add difference to m_differenceBuffer

        m_differenceBuffer->push_back(difference);

        // 2. check if m_differenceBuffer is full, which means the benchmark is finished
        if (m_differenceBuffer->size() == 1000)
        {
            signalThreadShouldExit();
        }
    }

    std::memcpy(m_inputBuffer.getWritePointer(0),
                buffer.data() + sizeof(timestamp),
                m_inputBuffer.getNumChannels() * m_inputBuffer.getNumSamples() *
                    sizeof(float));


    m_receivedData = false; //reset m_receivedData flag
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
