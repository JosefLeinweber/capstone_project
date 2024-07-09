#include "ConnectDAWs/tcpHost.h"


TcpHost::TcpHost(boost::asio::io_context &ioContext, int32_t port)
    : m_acceptor(
          ioContext,
          boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(),
                                         static_cast<unsigned short>(port)))
{
    std::cout << "TcpHost created" << std::endl;
    m_timer =
        std::make_shared<boost::asio::steady_timer>(m_acceptor.get_executor());
}

TcpHost::~TcpHost()
{
    if (m_socket)
    {
        if (m_socket->is_open())
        {
            m_socket->close();
        }
    }
    std::cout << "TcpHost destroyed" << std::endl;
}

void TcpHost::setupSocket()
{
    m_socket = std::make_unique<boost::asio::ip::tcp::socket>(
        m_acceptor.get_executor());
}

void TcpHost::asyncWaitForConnection(
    std::function<void(const boost::system::error_code &error)> callback,
    std::chrono::milliseconds timeout)
{
    if (timeout.count() > 0)
    {
        m_timer->expires_after(timeout);
        m_timer->async_wait([this](const boost::system::error_code &error) {
            if (!error)
            {
                std::cout << "TcpHost | asyncWaitForConnection timeout, "
                             "timer expiered"
                          << std::endl;
                m_acceptor.cancel();
            }
        });
        m_acceptor.async_accept(
            *m_socket,
            [this, callback](const boost::system::error_code &error) {
                callback(error);

                m_timer->cancel();
                if (!error)
                {
                    m_isConnected = true;
                }
                else
                {
                    m_isConnected = false;
                }
            });
    }
    else
    {
        m_acceptor.async_accept(
            *m_socket,
            [this, callback](const boost::system::error_code &error) {
                callback(error);
                m_isConnected = true;
            });
    }
}

void TcpHost::startAccept()
{
    m_acceptor.accept(*m_socket, m_error);
    if (m_error)
    {
        std::cout << "Failed to accept connection" << std::endl;
        m_socket->close();
        throw std::runtime_error("Failed to accept connection");
    }
    else
    {
        std::cout << "New connection established!" << std::endl;
    }
}
void TcpHost::handleConnect(const boost::system::error_code &error)
{
    if (!error)
    {
        std::cout << "TcpHost | Connection established" << std::endl;
        m_isConnected = true;
    }
    else
    {
        std::cout << "Failed to connect to remote host" << std::endl;
        std::cout << error.message() << std::endl;
        m_socket->close();
        m_isConnected = false;
    }
}

void TcpHost::handleConnectTimeout(const boost::system::error_code &error)
{
    if (!error)
    {
        std::cout << "TcpHost | asyncWaitForConnection timeout, timer expiered"
                  << std::endl;
        m_socket->cancel();
    }
    else
    {
        m_socket->cancel();
        //TODO: handle error
    }
}

void TcpHost::stopAsyncOperations()
{
    try
    {
        m_timer->cancel();
    }
    catch (const std::exception &e)
    {
        std::cout << "TcpHost | stopAsyncOperations | " << e.what()
                  << std::endl;
    }
}

void TcpHost::initializeConnection(std::string ip,
                                   int32_t port,
                                   std::chrono::milliseconds timeout)
{
    std::cout << "tcpHost initConnection  | Connecting to remote host"
              << std::endl;

    if (m_socket)
    {
        m_timer->expires_after(timeout);
        std::cout << "Connecting to remote host" << std::endl;

        m_socket->async_connect(boost::asio::ip::tcp::endpoint(
                                    boost::asio::ip::address::from_string(ip),
                                    static_cast<unsigned short>(port)),
                                [this](const boost::system::error_code &error) {
                                    handleConnect(error);
                                    m_timer->cancel();
                                });
        m_timer->async_wait([this](const boost::system::error_code &error) {
            handleConnectTimeout(error);
        });
    }
    else
    {
        std::cout << "No socket available" << std::endl;
        m_isConnected = false;
    }
}

bool TcpHost::validateIpAddress(std::string ip)
{
    try
    {
        auto ipAddress = boost::asio::ip::address::from_string(ip);
        return true;
    }
    catch (...)
    {
        std::cout << "TcpHost::validateIpAddress | Invalid IP address"
                  << std::endl;
        return false;
    }
}

void TcpHost::send(std::string &message)
{
    if (m_socket && m_socket->is_open())
    {
        std::size_t len = m_socket->write_some(
            boost::asio::buffer(message.data(), message.size()),
            m_error);

        if (len <= 0 || m_error)
        {
            std::cout << "Failed to send configuration" << std::endl;
            throw std::runtime_error("Failed to send configuration");
        }
        std::cout << len << " bytes sent" << std::endl;
    }
    else
    {
        std::cout << "PROBLEM:" << std::endl;
        if (!m_socket)
        {
            std::cout << "TcpHost::sendConfiguration | No socket available"
                      << std::endl;
            throw std::runtime_error("No socket available");
        }
        else
        {
            std::cout << "TcpHost::sendConfiguration | Socket is closed"
                      << std::endl;
            throw std::runtime_error("Socket is closed");
        }
    }
}

std::string TcpHost::receiveConfiguration()
{
    std::string message(100, ' ');
    if (m_socket && m_socket->is_open())
    {
        std::size_t len = m_socket->read_some(
            boost::asio::buffer(message.data(), message.size()),
            m_error);


        if (len <= 0)
        {
            std::cout << "Nothing got received" << std::endl;
            throw std::runtime_error("Nothing got received");
        }
        if (m_error)
        {
            std::cout << "Error during receive :" << m_error.message()
                      << std::endl;
            throw std::runtime_error(m_error.message());
        }

        std::cout << len << " bytes received" << std::endl;
        message.resize(len);
        return message;
    }
    else
    {
        if (!m_socket)
        {
            std::cout << "TcpHost::receiveConfiguration | No socket available"
                      << std::endl;
            throw std::runtime_error("No socket available");
        }
        else
        {
            std::cout << "TcpHost::receiveConfiguration| Socket is closed"
                      << std::endl;
            throw std::runtime_error("Socket is closed");
        }
    }
}


std::string TcpHost::serializeConfigurationData(
    ConfigurationData configurationData)
{
    std::string serializedData;
    if (!configurationData.SerializeToString(&serializedData))
    {
        std::cout << "failed to serialize data " << std::endl;
        throw std::runtime_error("Failed to serialize configuration data");
    }
    return serializedData;
}

ConfigurationData TcpHost::deserializeConfigurationData(
    std::string &serializedData)
{
    ConfigurationData configurationData;
    if (!configurationData.ParseFromString(serializedData))
    {
        std::cout << "faild to deserialize data " << std::endl;
        throw std::runtime_error("Failed to deserialize configuration data");
    }
    return configurationData;
}

bool TcpHost::isConnected()
{
    return m_isConnected;
}
