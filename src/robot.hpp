#ifndef ROBOT_HPP
#define ROBOT_HPP

#include "jsonrpctcpclient.hpp"

#include <string>
#include <semaphore>
#include <optional>


class Robot
{
public:
    Robot(const std::string & hostIpAddress, uint16_t tcpPort);
    ~Robot();

    bool isLineTrackDetected(std::size_t index);
    inline void isLineTrackDetectedWait(std::size_t index) {_isLineTrackDetected.at(index).wait();}
    template<typename _Rep, typename _Period>
    inline bool isLineTrackDetectedWait(std::size_t index, const std::chrono::duration<_Rep, _Period> & duration)
            {return _isLineTrackDetected.at(index).wait(duration);}

    enum class MotorIndex {RIGHT = 0, LEFT = 1};
    static std::string motorIndexToString(MotorIndex motorIndex);

    void setMotorSpeed(MotorIndex motorIndex, float value);
    void setMotorsSpeed(float rightValue, float leftValue);

private:
    Robot(const Robot &) = delete;
    Robot & operator=(const Robot &) = delete;

    JsonRpcTcpClient _jsonRpcTcpClient;
    template<typename T>
    struct Value
    {
        Value() : _value(), _changedCount(), _semaphore(std::make_unique<std::binary_semaphore>(0)) {}
        void notify() {_semaphore->release();}
        void wait() {_semaphore->acquire();}
        template<typename _Rep, typename _Period>
        bool wait(const std::chrono::duration<_Rep, _Period> & duration) {return _semaphore->try_acquire_for(duration);}
        std::optional<T> _value;
        std::optional<int> _changedCount;
        std::unique_ptr<std::binary_semaphore> _semaphore;
    };
    std::vector<Value<bool>> _isLineTrackDetected; // Updated by receive and main thread => need mutex
    std::mutex _mutex;
};

#endif
