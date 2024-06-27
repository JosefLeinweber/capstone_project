#pragma once
#include "audioBuffer.h"
#include "consumerThread.h"
#include "datagram.pb.h"
#include "providerThread.h"
#include "tcpHost.h"
#include "udpHost.h"
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <juce_core/juce_core.h>

class ConnectDAWs;

class MessageToGUI : public juce::Message
{
public:
    MessageToGUI(const std::string &ipAddress, int port);

    std::string ip;
    int port;
};

class MessageToCMT : public juce::Message
{
public:
    MessageToCMT(const std::string &ipAddress, int port);

    std::string ip;
    int port;
};


class ConnectionManagerThread : public juce::Thread,
                                public juce::MessageListener
{
public:
    ConnectionManagerThread(ConnectDAWs &audioProcessor,
                            ConfigurationData localConfigurationData,
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

    void sendMessageToGUI(const std::string &ip, int port);

    void handleMessage(const juce::Message &message) override;

    boost::asio::io_context m_ioContext;
    std::jthread m_ioContextThread;
    std::atomic<bool> &m_startConnection;
    std::atomic<bool> &m_stopConnection;

private:
    std::atomic<bool> m_incomingConnection = false;

    std::unique_ptr<ProviderThread> m_providerThread;
    std::unique_ptr<ConsumerThread> m_consumerThread;

    std::unique_ptr<TcpHost> m_host;

    ConnectDAWs &m_audioProcessor;

    AudioBufferFIFO &m_inputRingBuffer;
    AudioBufferFIFO &m_outputRingBuffer;
    ConfigurationData m_localConfigurationData;
    ConfigurationData m_remoteConfigurationData;
};
