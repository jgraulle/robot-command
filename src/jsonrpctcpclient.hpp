#ifndef JSONRPCTCPCLIENT_HPP
#define JSONRPCTCPCLIENT_HPP

#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <asio/io_context.hpp>
#include <semaphore>
#include <json/value.h>

namespace Json
{
    class StreamWriter;
}


class JsonRpcTcpClient
{
public:
    JsonRpcTcpClient(const std::string & hostIpAddress, unsigned short tcpPort);
    ~JsonRpcTcpClient();

    using NotificationHandle = std::function<void(Json::Value)>;
    void bindNotification(const std::string & methodName, const NotificationHandle & notificationHandle);

    //! @warning start receive only after bind all notification
    void startReceive();

    void callNotification(const char * methodName, const Json::Value & param);
    Json::Value callMethod(const char * methodName, const Json::Value & param);

private:
    JsonRpcTcpClient(const JsonRpcTcpClient &) = delete;
    JsonRpcTcpClient & operator=(const JsonRpcTcpClient &) = delete;

    void receive();

    asio::io_context _ioc;
    asio::ip::tcp::socket _socket;
    int _jsonRpcId;
    asio::streambuf _tcpStreambuf;
    std::ostream _tcpOutStream;
    std::unique_ptr<Json::StreamWriter> _jsonStreamWriter;
    std::binary_semaphore _receiveMethodResponseSem;
    Json::Value _receiveMethodResponseJsonValue;
    std::map<std::string, NotificationHandle> _notificationHandles;
    bool _isStartReceive;
};

#endif