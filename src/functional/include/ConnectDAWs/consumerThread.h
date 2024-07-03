#pragma once
#include "audioBuffer.h"
#include "udpHost.h"
#include <boost/asio.hpp>
#include <juce_core/juce_core.h>


class ConsumerThread : public juce::Thread
{
public:
    ConsumerThread(
        ConfigurationData remoteConfigurationData,
        ConfigurationData localConfigurationData,
        AudioBufferFIFO &inputRingBuffer,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
        const std::string threadName = "ConsumerThread");

    ~ConsumerThread() override;

    void run() override;

    void setupHost();

    bool receiveAudioFromRemoteProvider(std::chrono::milliseconds timeout);

    void writeToFIFOBuffer();

    void startIOContextInDifferentThread();
    bool timeOut(
        std::chrono::milliseconds timeout,
        std::chrono::time_point<std::chrono::high_resolution_clock> start);

    juce::AudioBuffer<float> m_inputBuffer;
    AudioBufferFIFO &m_inputRingBuffer;

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
};
