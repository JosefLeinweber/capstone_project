#pragma once
#include "audioBuffer.h"
#include "consumerThread.h"
#include "datagram.pb.h"
#include "logger.h"
#include "messenger.h"
#include "providerThread.h"
#include "tcpHost.h"
#include <functional>
#include <juce_core/juce_core.h>

class ConnectionManagerThread : public juce::Thread
{
public:
    ConnectionManagerThread(
        std::shared_ptr<Messenger> &guiMessenger,
        std::shared_ptr<Messenger> &cmtMessenger,
        ConfigurationData localConfigurationData,
        AudioBufferFIFO &inputRingBuffer,
        AudioBufferFIFO &outputRingBuffer,
        std::atomic<bool> &startConnection,
        std::atomic<bool> &stopConnection,
        const std::string threadName = "ConnectionManagerThread");

    ~ConnectionManagerThread() override;

    void run() override;

    void setupHost();

    void asyncWaitForConnection(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(500));

    void callbackFunction(const boost::system::error_code &error);

    void startIOContextInDifferentThread();

    bool initializeConnection(ConfigurationData remoteConfigurationData);

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

    void stopProviderAndConsumerThreads(std::chrono::seconds timeout);

    bool isConnected();

    void sendMessageToGUI(std::string type, std::string message);

    void handleMessage(const juce::Message &message);

    void initCMTMessenger();

    void setup();

    void stopAsyncWaitForConnection();

    boost::asio::io_context m_ioContext;
    std::jthread m_ioContextThread;
    std::atomic<bool> &m_startConnection;
    std::atomic<bool> &m_stopConnection;
    ConfigurationData m_remoteConfigurationData;

private:
    bool validateIpAddress(std::string ip);
    std::atomic<bool> m_incomingConnection = false;

    std::unique_ptr<FileLogger> m_fileLogger;
    std::unique_ptr<ProviderThread> m_providerThread;
    std::unique_ptr<ConsumerThread> m_consumerThread;
    std::unique_ptr<TcpHost> m_host;

    std::shared_ptr<Messenger> &m_guiMessenger;
    std::shared_ptr<Messenger> &m_cmtMessenger;

    AudioBufferFIFO &m_inputRingBuffer;
    AudioBufferFIFO &m_outputRingBuffer;
    ConfigurationData m_localConfigurationData;
};
