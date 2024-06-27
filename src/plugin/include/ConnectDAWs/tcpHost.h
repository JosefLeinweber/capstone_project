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

    void initializeConnection(std::string ip, unsigned short port);

    void send(std::string &message);

    std::string receiveConfiguration();

    void asyncWaitForExitCall();

    std::string serializeConfigurationData(ConfigurationData dataCollection);

    ConfigurationData deserializeConfigurationData(std::string &serializedData);

    bool incomingConnection()
    {
        return m_incomingConnection;
    };

private:
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::system::error_code m_error;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
    bool m_incomingConnection = false;
};
