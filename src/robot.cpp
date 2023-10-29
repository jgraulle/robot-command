#include "robot.hpp"

#include <jsonrpccpp/client.h>


Robot::Robot(jsonrpc::Client & jsonRcpClient)
    : _jsonRcpClient(jsonRcpClient)
{}

Robot::~Robot()
{}

bool Robot::isLineTrackDetected(std::size_t index)
{
    Json::Value params;
    params["index"] = index;
    return _jsonRcpClient.CallMethod("isLineTrackDetected", params).asBool();
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
    _jsonRcpClient.CallNotification("setMotorSpeed", params);
}
void Robot::setMotorsSpeed(float rightValue, float leftValue)
{
    Json::Value params;
    params["rightValue"] = rightValue;
    params["leftValue"] = leftValue;
    _jsonRcpClient.CallNotification("setMotorsSpeed", params);
}
