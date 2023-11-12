#include "robot.hpp"

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <thread>

using namespace std::chrono_literals;


int main(int argc, char ** argv)
{
    std::string hostIpAddress("127.0.0.1");
    uint16_t tcpPort = 6543;

    if (argc == 3)
    {
        hostIpAddress = std::string(argv[1]);
        std::istringstream iss(argv[2]);
        iss.exceptions(std::istringstream::failbit);
        iss >> tcpPort;
    }

    std::cout << "Try to connect to " << hostIpAddress << ":" << tcpPort << " ..." << std::endl;
    Robot robot(hostIpAddress, tcpPort);
    robot.waitReady();

    try
    {
        while (true)
        {
            auto event = robot.waitChanged({Robot::EventType::LINE_TRACKS_IS_DETECTED, Robot::EventType::SWITCHS_IS_DETECTED}, 1s);
            if (!event.has_value())
                robot.setMotorsSpeed(1.0, 1.0);
            else if (event.value() == Robot::EventType::LINE_TRACKS_IS_DETECTED)
                robot.setMotorsSpeed(-0.2, 0.2);
            else if (event.value() == Robot::EventType::SWITCHS_IS_DETECTED)
                robot.setMotorsSpeed(-1.0, -0.8);
            else
                std::cout << "error" << std::endl;
        }
    }
    catch (std::exception & e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
