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
            auto event = robot.waitChanged({EventType::LINE_TRACKS_IS_DETECTED, EventType::SWITCHS_IS_DETECTED}, 0.5s);
            bool lineTrackValue = robot.getLineTracksIsDetected(0);
            if (!event.has_value())
                robot.setMotorsPower(0.5, 0.5);
            else if (event.value() == EventType::LINE_TRACKS_IS_DETECTED)
                if (lineTrackValue)
                    robot.setMotorsPower(-0.2, 0.2);
                else
                    robot.setMotorsPower(-0.1, 0.1);
            else if (event.value() == EventType::SWITCHS_IS_DETECTED)
                robot.setMotorsPower(-0.5, -0.3);
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
