#include "robot.hpp"

#include <json/value.h>
#include <iostream>


Robot::Robot(const std::string & hostIpAddress, uint16_t tcpPort)
    : _jsonRpcTcpClient(hostIpAddress, tcpPort)
    , _irProximitysDistanceDetected()
    , _lineTracksIsDetected()
    , _lineTracksValue()
    , _speedsValue()
    , _switchsIsDetected()
    , _ultrasonicsDistanceDetected()
    , _isReadySemaphore(0)
{
    _jsonRpcTcpClient.bindNotification("irProximityDistanceDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _irProximitysDistanceDetected.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("lineTrackIsDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _lineTracksIsDetected.set(index, params["value"].asBool(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("lineTrackValue", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _lineTracksValue.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("speedValue", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _speedsValue.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("switchIsDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _switchsIsDetected.set(index, params["value"].asBool(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("ultrasonicDistanceDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _ultrasonicsDistanceDetected.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("setIsReady", [this](const Json::Value & params){
        assert(params.asBool());
        _isReadySemaphore.release();
    });
    _jsonRpcTcpClient.startReceive();
}

Robot::~Robot()
{}

std::string Robot::motorIndexToString(MotorIndex motorIndex)
{
    switch (motorIndex)
    {
        case MotorIndex::RIGHT:
            return "RIGHT";
        case MotorIndex::LEFT:
            return "LEFT";
    }
    throw std::invalid_argument(std::string("Cannot convert ")
            + std::to_string(static_cast<int>(motorIndex)) + " into Robot::MotorIndex");
}

void Robot::setMotorSpeed(MotorIndex motorIndex, float value)
{
    Json::Value params;
    params["motorIndex"] = motorIndexToString(motorIndex);
    params["value"] = value;
    _jsonRpcTcpClient.callNotification("setMotorSpeed", params);

}
void Robot::setMotorsSpeed(float rightValue, float leftValue)
{
    Json::Value params;
    params["rightValue"] = rightValue;
    params["leftValue"] = leftValue;
    _jsonRpcTcpClient.callNotification("setMotorsSpeed", params);
}
