#include "robot.hpp"

#include <json/value.h>
#include <iostream>


Robot::Robot(const std::string & hostIpAddress, uint16_t tcpPort)
    : _jsonRpcTcpClient(hostIpAddress, tcpPort)
{
    _jsonRpcTcpClient.bindNotification("isLineTrackDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        const std::lock_guard<std::mutex> lock(_mutex);
        if (index>=_isLineTrackDetected.size())
            _isLineTrackDetected.resize(index+1);
        _isLineTrackDetected.at(index)._value = params["value"].asBool();
        if (_isLineTrackDetected.at(index)._changedCount.has_value())
        {
            _isLineTrackDetected.at(index)._changedCount.value()++;
            assert(_isLineTrackDetected.at(index)._changedCount == params["changedCount"].asInt());
        }
        else
            _isLineTrackDetected.at(index)._changedCount = params["changedCount"].asInt();
        _isLineTrackDetected.at(index).notify();
    });
    _jsonRpcTcpClient.startReceive();
}

Robot::~Robot()
{}

bool Robot::isLineTrackDetected(std::size_t index)
{
    const std::lock_guard<std::mutex> lock(_mutex);
    if (index>=_isLineTrackDetected.size())
        _isLineTrackDetected.resize(index+1);
    if (!_isLineTrackDetected.at(index)._value.has_value())
    {
        Json::Value params;
        params["index"] = index;
        _isLineTrackDetected.at(index)._value = _jsonRpcTcpClient.callMethod("isLineTrackDetected", params).asBool();
    }
    // Else
    return _isLineTrackDetected.at(index)._value.value();
}

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
