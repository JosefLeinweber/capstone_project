#pragma once
#include "AudioBuffer.h"
#include "ConsumerThread.h"
#include "Host.h"
#include "ProviderThread.h"
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>


class ConnectionManagerThread : public juce::Thread
{
public:
    ConnectionManagerThread(AudioBufferFIFO &inputRingBuffer,
                            AudioBufferFIFO &outputRingBuffer);

    void run() override;

    void setupHost();

    void asyncWaitForConnection();

    void callbackFunction(const boost::system::error_code &error,
                          size_t bytes_transferred);

    bool validateConnection();

    void stopThreadSafely();

    bool startUpProviderAndConsumerThreads();

    void waitForClosingRequest();

    void closeProviderAndConsumerThreads();


private:
    std::atomic<bool> m_isProviderConnected;
    std::atomic<bool> m_isConsumerConnected;
    std::unique_ptr<ProviderThread> m_providerThread;
    std::unique_ptr<ConsumerThread> m_consumerThread;
    AudioBufferFIFO &m_inputRingBuffer;
    AudioBufferFIFO &m_outputRingBuffer;
    bool m_incommingConnection = false;
    std::unique_ptr<Host> m_host;
};
