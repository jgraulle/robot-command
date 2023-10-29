#include "robot.hpp"

#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/tcpsocketclient.h>
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
    jsonrpc::TcpSocketClient tcpClient(hostIpAddress, tcpPort);
    jsonrpc::Client jsonRcpClient(tcpClient);
    Robot robot(jsonRcpClient);

    try
    {
        while (true)
        {
            robot.setMotorsSpeed(1.0, 1.0);
            std::this_thread::sleep_for(1s);
            robot.setMotorsSpeed(-0.2, 0.2);
            std::this_thread::sleep_for(1s);
        }
    }
    catch (jsonrpc::JsonRpcException & e)
    {
        std::cerr << e.what() << std::endl;
    }
}
