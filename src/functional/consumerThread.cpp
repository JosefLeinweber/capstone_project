#include "ConnectDAWs/consumerThread.h"


ConsumerThread::ConsumerThread(ConfigurationData remoteConfigurationData,
                               ConfigurationData localConfigurationData,
                               RingBuffer &inputRingBuffer,
                               std::shared_ptr<Benchmark> &benchmark,
                               std::shared_ptr<FileLogger> &fileLogger,
                               std::chrono::milliseconds timeout,
                               const std::string threadName)

    : juce::Thread(threadName), m_timeout(timeout),
      m_remoteConfigurationData(remoteConfigurationData),
      m_localConfigurationData(localConfigurationData),
      m_inputRingBuffer(inputRingBuffer), m_benchmark(benchmark),
      m_fileLogger(fileLogger)
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
    m_fileLogger->logMessage("ConsumerThread | run | start");
    setupHost();
    m_fileLogger->logMessage("ConsumerThread | run | entering while loop");
    while (!threadShouldExit())
    {
        //!For some reason loggin is this while loop makes the test run for ever, but only when executing all tests
        // m_fileLogger->logMessage(
        //     "ConsumerThread | run | check if audio received");
        if (receiveAudioFromRemoteProvider(m_timeout))
        {

            writeToRingBuffer();
        }
    }
    m_fileLogger->logMessage("ConsumerThread | run | exit");
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
        sizeof(int64_t));
    m_udpHost->asyncReceiveAudioBuffer(
        buffer,
        std::bind(&ConsumerThread::receiveHandler,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2));

    auto startTime = std::chrono::high_resolution_clock::now();

    m_fileLogger->logMessage("ConsumerThread | ready to receive data ");

    while (!m_receivedData)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (threadShouldExit())
        {
            std::cout << "ConsumerThread | receiveAudioFromRemoteProvider | "
                         "Thread should exit"
                      << std::endl;
            m_fileLogger->logMessage(
                "ConsumerThread | receiveAudioFromRemoteProvider "
                "| Thread should exit");
            m_udpHost->cancelReceive();
            return false;
        }
        if (timeOut(timeout, startTime))
        {
            std::cout << "ConsumerThread | receiveAudioFromRemoteProvider | "
                         "Timeout"
                      << std::endl;
            m_fileLogger->logMessage(
                "ConsumerThread | receiveAudioFromRemoteProvider | "
                "Timeout");
            signalThreadShouldExit();
            m_udpHost->cancelReceive();
            return false;
        }
    }

    m_fileLogger->logMessage(
        "ConsumerThread | receiveAudioFromRemoteProvider | received audio!");
    //TODO: Implement correct version, move this code somewhere else
    std::int64_t timestamp;
    std::memcpy(&timestamp, buffer.data(), sizeof(timestamp));

    //TODO: change to dynamic
    if (true)
    {
        saveTimestamps(timestamp);

        m_fileLogger->logMessage(
            "ConsumerTHread | receiveAudioFromRemoteProvider | before "
            "m_benchmark->finished()");
        if (m_benchmark->finished())
        {
            m_fileLogger->logMessage(
                "ConsumerTHread | receiveAudioFromRemoteProvider | benchmark "
                "finished");

            signalThreadShouldExit();
        }
    }

    m_fileLogger->logMessage("ConsumerTHread | receiveAFRP | before copying");

    std::memcpy(m_inputBuffer.getWritePointer(0),
                buffer.data() + sizeof(timestamp),
                m_inputBuffer.getNumChannels() * m_inputBuffer.getNumSamples() *
                    sizeof(float));

    m_fileLogger->logMessage("ConsumerTHread | receiveAFRP | after copying");

    m_receivedData = false; //reset m_receivedData flag
    return true;
};

void ConsumerThread::saveTimestamps(int64_t timestamp)
{
    m_fileLogger->logMessage("ConsumerTHread | saveTimestamps | started");
    // std::vector<int64_t> timestamps;
    // timestamps.push_back(timestamp);
    // m_benchmark->debugFunction(timestamps);
    // m_fileLogger->logMessage("ConsumerTHread | saveTimestamps | finished");
    try
    {
        std::vector<int64_t> timestamps;
        timestamps.push_back(timestamp);
        m_benchmark->copyFrom(timestamps);
        m_fileLogger->logMessage(
            "ConsumerTHread | saveTimestamps | start timestamp "
            "saved");
    }
    catch (const std::exception &e)
    {
        m_fileLogger->logMessage(
            "ConsumerTHread | saveTimestamps | failed to save start "
            "timestamp");
        m_fileLogger->logMessage(e.what());
    }
    std::vector<int64_t> currentTimestamps;
    currentTimestamps.push_back(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    m_benchmark->copyFrom(currentTimestamps);
    m_fileLogger->logMessage("ConsumerTHread | saveTimestamps | finished");
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
