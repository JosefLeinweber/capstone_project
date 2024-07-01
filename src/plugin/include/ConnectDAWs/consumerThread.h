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

    void setupHost(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(500));

    bool receiveAudioFromRemoteProvider();

    void writeToFIFOBuffer();

    juce::AudioBuffer<float> m_inputBuffer;
    AudioBufferFIFO &m_inputRingBuffer;

private:
    boost::asio::io_context m_ioContext;
    std::unique_ptr<UdpHost> m_udpHost;
    std::chrono::milliseconds m_timeout;
    ConfigurationData m_remoteConfigurationData;
    ConfigurationData m_localConfigurationData;
};
