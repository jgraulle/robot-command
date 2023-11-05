#ifndef ROBOT_HPP
#define ROBOT_HPP

#include "jsonrpctcpclient.hpp"

#include <string>


class Robot
{
public:
    Robot(const std::string & hostIpAddress, uint16_t tcpPort);
    ~Robot();

    bool isLineTrackDetected(std::size_t index);

    enum class MotorIndex {RIGHT = 0, LEFT = 1};
    static std::string motorIndexToString(MotorIndex motorIndex);

    void setMotorSpeed(MotorIndex motorIndex, float value);
    void setMotorsSpeed(float rightValue, float leftValue);

private:
    Robot(const Robot &) = delete;
    Robot & operator=(const Robot &) = delete;

    JsonRpcTcpClient _jsonRpcTcpClient;
};

#endif
