#pragma once
#include "AudioBuffer.h"
#include "ConsumerThread.h"
#include "Host.h"
#include "ProviderThread.h"
#include "TcpHost.h"
#include "datagram.pb.h"
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>


class ConnectionManagerThread : public juce::Thread
{
public:
    ConnectionManagerThread(ConfigurationData localConfigurationData,
                            AudioBufferFIFO &inputRingBuffer,
                            AudioBufferFIFO &outputRingBuffer,
                            std::atomic<bool> &startConnection,
                            std::atomic<bool> &stopConnection);

    ~ConnectionManagerThread() override;

    void run() override;

    void setupHost();

    void asyncWaitForConnection(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(500));

    void callbackFunction(const boost::system::error_code &error);

    void startIOContextInDifferentThread();

    void initializeConnection(ConfigurationData remoteConfigurationData);

    bool sendConfigurationData(ConfigurationData localConfigurationData);

    bool receiveConfigurationData();

    ConfigurationData remoteConfigurationDataFromGUI();

    bool exchangeConfigurationDataWithRemote(
        ConfigurationData configurationData);

    ConfigurationData generateConfigurationData();

    bool startUpProviderAndConsumerThreads(
        ConfigurationData localConfigurationData,
        ConfigurationData remoteConfigurationData,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(3000));

    void resetToStartState();

    void asyncWaitForClosingRequest();

    void stopProviderAndConsumerThreads(std::chrono::seconds timeout);

    ConfigurationData getRemoteConfigurationData() const;

    bool incomingConnection() const;

    boost::asio::io_context m_ioContext;
    std::jthread m_ioContextThread;
    std::atomic<bool> &m_startConnection;
    std::atomic<bool> &m_stopConnection;

private:
    std::atomic<bool> m_incomingConnection = false;

    std::unique_ptr<ProviderThread> m_providerThread;
    std::unique_ptr<ConsumerThread> m_consumerThread;

    std::unique_ptr<TcpHost> m_host;


    AudioBufferFIFO &m_inputRingBuffer;
    AudioBufferFIFO &m_outputRingBuffer;
    ConfigurationData m_localConfigurationData;
    ConfigurationData m_remoteConfigurationData;
};
