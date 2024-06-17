#pragma once
#include "Host.h"
#include "datagram.pb.h"
#include <boost/asio.hpp>
#include <iostream>

struct ConfigurationDataStruct
{
    addressData providerAddress;
    addressData consumerAddress;

    pbConfigurationData toPb()
    {
        pbConfigurationData configurationData;
        configurationData.set_provider_ip(providerAddress.ip);
        configurationData.set_provider_port(providerAddress.port);
        configurationData.set_consumer_ip(consumerAddress.ip);
        configurationData.set_consumer_port(consumerAddress.port);
        return configurationData;
    }

    void fromPb(pbConfigurationData pbConfigurationData)
    {
        providerAddress.ip = pbConfigurationData.provider_ip();
        providerAddress.port = pbConfigurationData.provider_port();
        consumerAddress.ip = pbConfigurationData.consumer_ip();
        consumerAddress.port = pbConfigurationData.consumer_port();
    }
};


class TcpHost
{
public:
    TcpHost(boost::asio::io_context &ioContext, unsigned short port);

    ~TcpHost();

    void setupSocket();

    void startAsyncAccept();

    void startAccept();

    void initializeConnection(addressData remoteAddress);

    void send(std::string &message);

    std::string receiveConfiguration();

    void asyncWaitForExitCall();

    std::string serializeConfigurationData(pbConfigurationData dataCollection);

    ConfigurationDataStruct deserializeConfigurationData(
        std::string &serializedData);

private:
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::system::error_code m_error;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
    boost::asio::io_context m_ioContext;
};
