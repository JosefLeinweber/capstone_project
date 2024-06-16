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

    ~ConnectionManagerThread() override;

    void run() override;

    void setupHost(addressData hostAddress);

    void asyncWaitForConnection(std::chrono::milliseconds timeout);

    void callbackFunction(const boost::system::error_code &error,
                          size_t bytes_transferred);

    bool validateConnection();

    void stopThreadSafely();

    bool startUpProviderAndConsumerThreads(addressData providerAddress,
                                           addressData consumerAddress,
                                           addressData remoteConsumerAddress);

    void asyncWaitForClosingRequest();

    void closeProviderAndConsumerThreads();
    void stopProviderAndConsumerThreads(std::chrono::seconds timeout);

    std::unique_ptr<Host> &getHost()
    {
        return m_host;
    };

    bool incommingConnection()
    {
        return m_incomingConnection;
    };

    //! DELETE
    void onlyStartConsumerThread(addressData consumerAddress);

private:
    std::atomic<bool> m_isProviderConnected = false;
    std::atomic<bool> m_isConsumerConnected = false;
    std::atomic<bool> m_incomingConnection = false;
    std::atomic<bool> m_closingRequest = false;

    std::unique_ptr<ProviderThread> m_providerThread;
    std::unique_ptr<ConsumerThread> m_consumerThread;
    std::unique_ptr<Host> m_host;

    AudioBufferFIFO &m_inputRingBuffer;
    AudioBufferFIFO &m_outputRingBuffer;
};
