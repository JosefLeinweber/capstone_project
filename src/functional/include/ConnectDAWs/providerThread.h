#pragma once
#include "audioBuffer.h"
#include "datagram.pb.h"
#include "udpHost.h"
#include <boost/asio.hpp>
#include <juce_core/juce_core.h>

class ProviderThread : public juce::Thread
{
public:
    ProviderThread(
        ConfigurationData remoteConfigurationData,
        ConfigurationData localConfigurationData,
        AudioBufferFIFO &outputRingBuffer,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(2000),
        const std::string threadName = "ProviderThread");


    ~ProviderThread() override;

    void run() override;

    void setupHost();

    bool readFromFIFOBuffer(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    bool sendAudioToRemoteConsumer();

    AudioBufferFIFO &m_outputRingBuffer;
    juce::AudioBuffer<float> m_outputBuffer;

private:
    boost::asio::io_context m_ioContext;
    std::unique_ptr<UdpHost> m_udpHost;
    std::chrono::milliseconds m_timeout;
    ConfigurationData m_remoteConfigurationData;
    ConfigurationData m_localConfigurationData;
};