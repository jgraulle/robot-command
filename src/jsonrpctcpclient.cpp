#include "jsonrpctcpclient.hpp"

#include <utility>
#include <memory>
#include <json/json.h>
#include <iostream>
#include <asio/streambuf.hpp>
#include <asio/write.hpp>
#include <asio/read_until.hpp>
#include <asio/read.hpp>
#include <asio/buffer.hpp>
#include <thread>


JsonRpcTcpClient::JsonRpcTcpClient(const std::string & hostIpAddress, unsigned short tcpPort)
    : _ioc()
    , _socket(_ioc)
    , _jsonRpcId(1)
    , _tcpStreambuf()
    , _tcpOutStream(&_tcpStreambuf)
    , _jsonStreamWriter(nullptr)
    , _receiveMethodResponseSem(0)
    , _isStartReceive(false)
{
    Json::StreamWriterBuilder jsonStreamWriterBuilder;
    jsonStreamWriterBuilder["indentation"] = "";
    _jsonStreamWriter.reset(jsonStreamWriterBuilder.newStreamWriter());

    _socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(hostIpAddress), tcpPort));
}

JsonRpcTcpClient::~JsonRpcTcpClient()
{
    _socket.close();
}

void JsonRpcTcpClient::bindNotification(const std::string & methodName, const std::function<void(Json::Value)> & notificationHandle)
{
    assert(!_isStartReceive);
    _notificationHandles.insert(std::make_pair(methodName, notificationHandle));
}

void JsonRpcTcpClient::startReceive()
{
    _isStartReceive = true;
    std::thread([](JsonRpcTcpClient * thus){thus->receive();}, this).detach();
}

void JsonRpcTcpClient::callNotification(const char * methodName, const Json::Value & params)
{
    // Prepare message
    Json::Value message;
    message["jsonrpc"] = "2.0";
    message["method"] = methodName;
    message["params"] = params;

    // Send message
    _jsonStreamWriter->write(message, &_tcpOutStream);
    _tcpOutStream << static_cast<char>(0x0A);
    asio::write(_socket, _tcpStreambuf);

#ifdef JSONRPC_DEBUG
    // Print message
    std::cout << "send message ";
    _jsonStreamWriter->write(message, &std::cout);
    std::cout << std::endl;
#endif
}

Json::Value JsonRpcTcpClient::callMethod(const char * methodName, const Json::Value & param)
{
    // Prepare message
    Json::Value message;
    message["jsonrpc"] = "2.0";
    message["method"] = methodName;
    message["params"] = param;
    message["id"] = _jsonRpcId;

    // Send message
    _jsonStreamWriter->write(message, &_tcpOutStream);
    _tcpOutStream << static_cast<char>(0x0A);
    asio::write(_socket, _tcpStreambuf);

#ifdef JSONRPC_DEBUG
    // Print message
    std::cout << "send message ";
    _jsonStreamWriter->write(message, &std::cout);
    std::cout << std::endl;
#endif

    // Wait response
#ifdef JSONRPC_DEBUG
    std::cout << "wait response..." << std::endl;
#endif
    _receiveMethodResponseSem.acquire();
    Json::Value responseJson = _receiveMethodResponseJsonValue;
    assert(_jsonRpcId == responseJson["id"].asInt());
    _receiveMethodResponseJsonValue = Json::Value();

#ifdef JSONRPC_DEBUG
    // Print response
    std::cout << "Receive response ";
    _jsonStreamWriter->write(responseJson, &std::cout);
    std::cout << std::endl;
#endif

    _jsonRpcId++;
    return responseJson["result"];
}

void JsonRpcTcpClient::receive()
{
    asio::streambuf tcpStreambuf;
    std::istream tcpInStream(&tcpStreambuf);

    while (true)
    {
        // Wait message
        asio::read_until(_socket, tcpStreambuf, 0x0A);
#ifdef JSONRPC_DEBUG
        std::cout << "wait message..." << std::endl;
#endif

        // Extract one json message
        std::string messageJsonStr;
        std::getline(tcpInStream, messageJsonStr, static_cast<char>(0x0A));

        // Parse this json message
        Json::CharReaderBuilder builder;
        JSONCPP_STRING errs;
        Json::Value messageJson;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(messageJsonStr.c_str(), messageJsonStr.c_str()+messageJsonStr.size(),
                &messageJson, &errs))
            throw std::runtime_error(errs);

        // If method response
        if (messageJson.isMember("id"))
        {
            assert(_receiveMethodResponseJsonValue.isNull());
            _receiveMethodResponseJsonValue = messageJson;
            _receiveMethodResponseSem.release();
        }
        // If notification
        else
        {
#ifdef JSONRPC_DEBUG
            // Print notification
            std::cout << "Receive notification ";
            _jsonStreamWriter->write(messageJson, &std::cout);
            std::cout << std::endl;
#endif
            // Find the notification and execute it
            auto it = _notificationHandles.find(messageJson["method"].asString());
            if (it != _notificationHandles.end())
            {
                it->second(messageJson["params"]);
            }
        }
    }
}
