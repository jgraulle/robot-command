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


JsonRpcTcpClient::JsonRpcTcpClient(const std::string & hostIpAddress, unsigned short tcpPort)
    : _ioc()
    , _jsonRpcId(1)
    , _endpoint(asio::ip::address::from_string(hostIpAddress), tcpPort)
    , _tcpStreambuf()
    , _tcpOutStream(&_tcpStreambuf)
    , _tcpInStream(&_tcpStreambuf)
    , _jsonStreamWriter(nullptr)
{
    Json::StreamWriterBuilder jsonStreamWriterBuilder;
    jsonStreamWriterBuilder["indentation"] = "";
    _jsonStreamWriter.reset(jsonStreamWriterBuilder.newStreamWriter());
}

JsonRpcTcpClient::~JsonRpcTcpClient()
{
}

void JsonRpcTcpClient::callNotification(const char * methodName, const Json::Value & params)
{
    asio::ip::tcp::socket socket(_ioc);
    socket.connect(_endpoint);

    // Prepare message
    Json::Value message;
    message["jsonrpc"] = "2.0";
    message["method"] = methodName;
    message["params"] = params;

    // Send message
    _jsonStreamWriter->write(message, &_tcpOutStream);
    _tcpOutStream << static_cast<char>(0x0A);
    asio::write(socket, _tcpStreambuf);

#ifdef JSONRPC_DEBUG
    // Print message
    std::cout << "send message ";
    _jsonStreamWriter->write(message, &std::cout);
    std::cout << std::endl;
#endif

    socket.close();
}

Json::Value JsonRpcTcpClient::callMethod(const char * methodName, const Json::Value & param)
{
    asio::ip::tcp::socket socket(_ioc);
    socket.connect(_endpoint);

    // Prepare message
    Json::Value message;
    message["jsonrpc"] = "2.0";
    message["method"] = methodName;
    message["params"] = param;
    message["id"] = _jsonRpcId;

    // Send message
    _jsonStreamWriter->write(message, &_tcpOutStream);
    _tcpOutStream << static_cast<char>(0x0A);
    asio::write(socket, _tcpStreambuf);

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
    asio::read_until(socket, _tcpStreambuf, 0x0A);

    // Extract one json message
    std::string messageJsonStr;
    std::getline(_tcpInStream, messageJsonStr, static_cast<char>(0x0A));

    // Parse this json message
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    Json::Value responseJson;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(messageJsonStr.c_str(), messageJsonStr.c_str()+messageJsonStr.size(),
            &responseJson, &errs))
        throw std::runtime_error(errs);

#ifdef JSONRPC_DEBUG
    // Print response
    std::cout << "Receive response ";
    _jsonStreamWriter->write(responseJson, &std::cout);
    std::cout << std::endl;
#endif

    socket.close();
    _jsonRpcId++;
    return responseJson["result"];
}
