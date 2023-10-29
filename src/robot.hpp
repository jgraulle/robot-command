#ifndef ROBOT_HPP
#define ROBOT_HPP

#include <string>

namespace jsonrpc
{
    class Client;
}


class Robot
{
public:
    Robot(jsonrpc::Client & jsonRcpClient);
    ~Robot();

    bool isLineTrackDetected(std::size_t index);

    enum class MotorIndex {RIGHT = 0, LEFT = 1};
    static std::string motorIndexToString(MotorIndex motorIndex);

    void setMotorSpeed(MotorIndex motorIndex, float value);
    void setMotorsSpeed(float rightValue, float leftValue);

private:
    jsonrpc::Client & _jsonRcpClient;
};

#endif
