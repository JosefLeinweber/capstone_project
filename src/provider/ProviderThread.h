#pragma once
#include "AudioBuffer.h"
#include "Host.h"
#include "datagram.pb.h"
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>

class ProviderThread : public juce::Thread
{
public:
    ProviderThread(ConfigurationData remoteConfigurationData,
                   ConfigurationData localConfigurationData,
                   AudioBufferFIFO &outputRingBuffer,

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
    std::unique_ptr<Host> m_host;

    ConfigurationData m_remoteConfigurationData;
    ConfigurationData m_localConfigurationData;
};
