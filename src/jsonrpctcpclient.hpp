#ifndef JSONRPCTCPCLIENT_HPP
#define JSONRPCTCPCLIENT_HPP

#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <asio/io_context.hpp>

namespace Json
{
    class Value;
    class StreamWriter;
}


class JsonRpcTcpClient
{
public:
    JsonRpcTcpClient(const std::string & hostIpAddress, unsigned short tcpPort);
    ~JsonRpcTcpClient();

    void callNotification(const char * methodName, const Json::Value & param);
    Json::Value callMethod(const char * methodName, const Json::Value & param);

private:
    JsonRpcTcpClient(const JsonRpcTcpClient &) = delete;
    JsonRpcTcpClient & operator=(const JsonRpcTcpClient &) = delete;

    asio::io_context _ioc;
    asio::ip::tcp::socket _socket;
    int _jsonRpcId;
    asio::streambuf _tcpStreambuf;
    std::ostream _tcpOutStream;
    std::istream _tcpInStream;
    std::unique_ptr<Json::StreamWriter> _jsonStreamWriter;
};

#endif