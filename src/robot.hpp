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

    inline void waitReady() {_isReadySemaphore.acquire();}

    template<typename T>
    class Values
    {
    public:
        inline T get(std::size_t index) const {const std::lock_guard<std::mutex> lock(_mutex); return _values.at(index)._value.value();}
        void waitChanged(std::size_t index) const {_values.at(index)._semaphore->acquire();}
        template<typename _Rep, typename _Period>
        bool waitChanged(std::size_t index, const std::chrono::duration<_Rep, _Period> & duration) const {return _values.at(index)._semaphore->try_acquire_for(duration);}
        void set(std::size_t index, T v, int changedCount)
        {
            const std::lock_guard<std::mutex> lock(_mutex);
            if (index>=_values.size())
                _values.resize(index+1);
            Value & value = _values.at(index);
            value._value = v;
            if (value._changedCount.has_value())
            {
                value._changedCount.value()++;
                assert(value._changedCount == changedCount);
            }
            else
                value._changedCount = changedCount;
            value._semaphore->release();
        }

    private:
        struct Value
        {
            Value() : _value(), _changedCount(), _semaphore(std::make_unique<std::binary_semaphore>(0)) {}
            std::optional<T> _value;
            std::optional<int> _changedCount;
            mutable std::unique_ptr<std::binary_semaphore> _semaphore;
        };
        std::vector<Value> _values;
        mutable std::mutex _mutex;
    };

    const Values<std::size_t> & getIrProximitysDistanceDetected() const {return _irProximitysDistanceDetected;}
    const Values<bool> & getLineTracksIsDetected() const {return _lineTracksIsDetected;}
    const Values<std::uint8_t> & getLineTracksValue() const {return _lineTracksValue;}
    const Values<std::size_t> & getSpeedValue() const {return _speedsValue;}
    const Values<bool> & getSwitchsIsDetected() const {return _switchsIsDetected;}
    const Values<std::size_t> & getUltrasonicsDistanceDetected() const {return _ultrasonicsDistanceDetected;}

    enum class MotorIndex {RIGHT = 0, LEFT = 1};
    static std::string motorIndexToString(MotorIndex motorIndex);

    void setMotorSpeed(MotorIndex motorIndex, float value);
    void setMotorsSpeed(float rightValue, float leftValue);

private:
    Robot(const Robot &) = delete;
    Robot & operator=(const Robot &) = delete;

    JsonRpcTcpClient _jsonRpcTcpClient;
    Values<std::size_t> _irProximitysDistanceDetected;
    Values<bool> _lineTracksIsDetected;
    Values<std::uint8_t> _lineTracksValue;
    Values<std::size_t> _speedsValue;
    Values<bool> _switchsIsDetected;
    Values<std::size_t> _ultrasonicsDistanceDetected;
    std::binary_semaphore _isReadySemaphore;
};

#endif
