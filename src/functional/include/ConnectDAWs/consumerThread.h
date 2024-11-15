#pragma once
#include "ringBuffer.h"
#include "udpHost.h"
#include <boost/asio.hpp>
#include <juce_core/juce_core.h>


class ConsumerThread : public juce::Thread
{
public:
    ConsumerThread(
        ConfigurationData remoteConfigurationData,
        ConfigurationData localConfigurationData,
        RingBuffer &inputRingBuffer,
        std::shared_ptr<std::vector<std::uint64_t>> &differenceBuffer,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
        const std::string threadName = "ConsumerThread");

    ~ConsumerThread() override;

    void run() override;

    void setupHost();

    bool receiveAudioFromRemoteProvider(std::chrono::milliseconds timeout);

    void writeToRingBuffer();

    void startIOContextInDifferentThread();
    bool timeOut(
        std::chrono::milliseconds timeout,
        std::chrono::time_point<std::chrono::high_resolution_clock> start);

    std::uint64_t calculateLatency(std::uint64_t timestamp);
    void saveLatencyToBuffer(std::uint64_t timestamp);
    bool benchmarkFinishedCollectingData(int numValues);

    juce::AudioBuffer<float> m_inputBuffer;
    RingBuffer &m_inputRingBuffer;

private:
    void receiveHandler(const boost::system::error_code &error,
                        std::size_t bytes_transferred);
    std::atomic<bool> m_receivedData = false;
    std::jthread m_ioContextThread;
    boost::asio::io_context m_ioContext;
    std::shared_ptr<boost::asio::executor_work_guard<
        boost::asio::io_context::executor_type>>
        m_workGuard;
    std::unique_ptr<UdpHost> m_udpHost;
    std::chrono::milliseconds m_timeout;
    ConfigurationData m_remoteConfigurationData;
    ConfigurationData m_localConfigurationData;
    std::shared_ptr<std::vector<std::uint64_t>> &m_differenceBuffer;
};
