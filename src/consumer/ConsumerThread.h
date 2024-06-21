#pragma once
#include "AudioBuffer.h"
#include "Host.h"
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>


class ConsumerThread : public juce::Thread
{
public:
    ConsumerThread(ConfigurationData remoteConfigurationData,
                   ConfigurationData localConfigurationData,
                   AudioBufferFIFO &inputRingBuffer,
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
    std::unique_ptr<Host> m_host;

    ConfigurationData m_remoteConfigurationData;
    ConfigurationData m_localConfigurationData;
};
