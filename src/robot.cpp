#include "robot.hpp"

#include <json/value.h>


Robot::Robot(const std::string & hostIpAddress, uint16_t tcpPort)
    : _jsonRpcTcpClient(hostIpAddress, tcpPort)
{
}

Robot::~Robot()
{}

bool Robot::isLineTrackDetected(std::size_t index)
{
    Json::Value params;
    params["index"] = index;
    return _jsonRpcTcpClient.callMethod("isLineTrackDetected", params).asBool();
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
