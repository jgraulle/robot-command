#ifndef ROBOT_HPP
#define ROBOT_HPP

#include "jsonrpctcpclient.hpp"

#include <string>
#include <optional>
#include <semaphore>
#include <condition_variable>
#include <set>
#include <chrono>


class Robot
{
public:
    Robot(const std::string & hostIpAddress, uint16_t tcpPort);
    ~Robot();

    inline void waitReady() {using namespace std::chrono_literals; _isReadySemaphore.acquire();
            std::this_thread::sleep_for(100ms);}

    enum class EventType
    {
        IR_PROXIMITYS_DISTANCE_DETECTED,
        LINE_TRACKS_IS_DETECTED,
        LINE_TRACKS_VALUE,
        ENCODER_WHEELS_VALUE,
        SWITCHS_IS_DETECTED,
        ULTRASOUNDS_DISTANCE_DETECTED
    };

    template<typename T>
    class Values
    {
    public:
        Values(Robot * robot, EventType eventType) : _robot(robot), _eventType(eventType) {}

        inline T get(std::size_t index) const {const std::lock_guard<std::mutex> lock(_mutex); return _values.at(index)._value.value();}
        inline int getChangedCount(std::size_t index) const {const std::lock_guard<std::mutex> lock(_mutex); return _values.at(index)._changedCount.value();}
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
            _robot->notify(_eventType, changedCount);
        }

    private:
        struct Value
        {
            Value() : _value(), _changedCount() {}
            std::optional<T> _value;
            std::optional<int> _changedCount;
        };
        std::vector<Value> _values;
        mutable std::mutex _mutex;
        Robot * _robot;
        EventType _eventType;
    };

    const Values<std::size_t> & getIrProximitysDistanceDetected() const {return _irProximitysDistanceDetected;}
    const Values<bool> & getLineTracksIsDetected() const {return _lineTracksIsDetected;}
    const Values<std::uint8_t> & getLineTracksValue() const {return _lineTracksValue;}
    const Values<std::size_t> & getEncoderWheelValue() const {return _encoderWheelsValue;}
    const Values<bool> & getSwitchsIsDetected() const {return _switchsIsDetected;}
    const Values<std::size_t> & getUltrasoundsDistanceDetected() const {return _ultrasoundsDistanceDetected;}

    enum class MotorIndex {RIGHT = 0, LEFT = 1};
    static std::string motorIndexToString(MotorIndex motorIndex);

    void setMotorSpeed(MotorIndex motorIndex, float value);
    void setMotorsSpeed(float rightValue, float leftValue);

    void notify(EventType eventType, int changedCount);

    void waitChanged(EventType eventType);
    EventType waitChanged(const std::set<EventType> & eventType);

    template<typename _Rep, typename _Period>
    bool waitChanged(EventType eventType, const std::chrono::duration<_Rep, _Period> & duration);
    template<typename _Rep, typename _Period>
    std::optional<EventType> waitChanged(const std::set<EventType> & eventTypes, const std::chrono::duration<_Rep, _Period> & duration);

private:
    Robot(const Robot &) = delete;
    Robot & operator=(const Robot &) = delete;

    void waitChangedHelper(EventType eventType, int & changedCount);
    EventType waitChangedHelper(std::map<EventType, int> & eventTypes);
    template<typename _Rep, typename _Period>
    bool waitChangedHelper(EventType eventType, int & changedCount, const std::chrono::duration<_Rep, _Period> & duration);
    template<typename _Rep, typename _Period>
    std::optional<EventType> waitChangedHelper(std::map<EventType, int> & eventTypes, const std::chrono::duration<_Rep, _Period> & duration);

    std::map<EventType, int> waitParamHelper(std::set<EventType> eventTypes);

    JsonRpcTcpClient _jsonRpcTcpClient;
    Values<std::size_t> _irProximitysDistanceDetected;
    Values<bool> _lineTracksIsDetected;
    Values<std::uint8_t> _lineTracksValue;
    Values<std::size_t> _encoderWheelsValue;
    Values<bool> _switchsIsDetected;
    Values<std::size_t> _ultrasoundsDistanceDetected;
    std::binary_semaphore _isReadySemaphore;
    std::mutex _eventCvMutex;
    std::condition_variable _eventCv;
    std::map<EventType, int> _lastNotifiedEventType;
};


template<typename _Rep, typename _Period>
bool Robot::waitChanged(EventType eventType, const std::chrono::duration<_Rep, _Period> & duration)
{
    auto eventTypes = waitParamHelper({eventType});
    return waitChangedHelper(eventTypes, duration).has_value();
}

template<typename _Rep, typename _Period>
bool Robot::waitChangedHelper(EventType eventType, int & changedCount, const std::chrono::duration<_Rep, _Period> & duration)
{
    std::map<EventType, int> eventTypes{{eventType, changedCount}};
    bool toReturn = waitChangedHelper(eventTypes, duration).has_value();
    changedCount = eventTypes.at(eventType);
    return toReturn;
}

template<typename _Rep, typename _Period>
std::optional<Robot::EventType> Robot::waitChanged(const std::set<EventType> & eventTypes, const std::chrono::duration<_Rep, _Period> & duration)
{
    auto eventTypesWithChangedCount = waitParamHelper(eventTypes);
    return waitChangedHelper(eventTypesWithChangedCount, duration);
}

template<typename _Rep, typename _Period>
std::optional<Robot::EventType> Robot::waitChangedHelper(std::map<EventType, int> & eventTypes, const std::chrono::duration<_Rep, _Period> & duration)
{
    std::unique_lock<std::mutex> lk(_eventCvMutex);
    EventType notifiedEventType;
    if (!_eventCv.wait_for(lk, duration, [eventTypes, &notifiedEventType, lastNotifiedEventType = std::ref(_lastNotifiedEventType)]{
        for (auto eventType : eventTypes) {
            if (lastNotifiedEventType.get().at(eventType.first)>eventType.second)
            {
                lastNotifiedEventType.get().at(eventType.first) = eventType.second;
                notifiedEventType = eventType.first;
                return true;
            }
        }
        return false;
    }))
        return {};
    return notifiedEventType;
}

#endif
