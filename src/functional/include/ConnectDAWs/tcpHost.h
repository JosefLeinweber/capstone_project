#pragma once
#include "datagram.pb.h"
#include <boost/asio.hpp>
#include <iostream>

class TcpHost
{
public:
    TcpHost(boost::asio::io_context &ioContext, unsigned short port);

    ~TcpHost();

    void setupSocket();

    void asyncWaitForConnection(
        std::function<void(const boost::system::error_code &error)> callback,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    void startAccept();

    void initializeConnection(
        std::string ip,
        unsigned short port,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    void send(std::string &message);

    std::string receiveConfiguration();

    void asyncWaitForExitCall();

    void stopAsyncOperations();

    std::string serializeConfigurationData(ConfigurationData dataCollection);

    ConfigurationData deserializeConfigurationData(std::string &serializedData);

    bool isConnected();

private:
    void handleConnect(const boost::system::error_code &error);
    void handleConnectTimeout(const boost::system::error_code &error);
    std::shared_ptr<boost::asio::steady_timer> m_timer;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::system::error_code m_error;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
    bool m_isConnected = false;
};
