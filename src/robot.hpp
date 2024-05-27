#ifndef ROBOT_HPP
#define ROBOT_HPP

#include "jsonrpctcpclient.hpp"
#include "values.hpp"

#include <string>
#include <optional>
#include <semaphore>
#include <condition_variable>
#include <set>
#include <chrono>

enum class EventType
{
    IR_PROXIMITYS_DISTANCE_DETECTED, //!< A new value of IR distance have been received
    LINE_TRACKS_IS_DETECTED, //!< A new value of the boolean is line detected have been received
    LINE_TRACKS_VALUE, //!< A new value of the raw line color have been received
    ENCODER_WHEELS_VALUE, //!< A new value of the encoder wheels have been received
    SWITCHS_IS_DETECTED, //!< A new value of the switch have been received
    ULTRASOUNDS_DISTANCE_DETECTED //!< A new value of ultrasound distance have been received
};

class Robot : public IRobot<EventType>
{
public:
    //! @brief Create a new robot connexion with a robot server (simu or reel).
    Robot(const std::string & hostIpAddress, uint16_t tcpPort);

    //! @brief Close the robot connexion.
    virtual ~Robot();

    //! @brief Wait for the robot server to be ready to send or receive messages.
    inline void waitReady() {using namespace std::chrono_literals; _isReadySemaphore.acquire();
            std::this_thread::sleep_for(100ms);}

    //! @return The last IR distance receive from robot server (in pixel on simu).
    //! @param index The index of this sensor ont he robot starting from 0.
    std::size_t getIrProximitysDistanceDetected(std::size_t index) const {return _irProximitysDistanceDetected.get(index);}

    //! @return True if the line has been detected in the last receive message from robot server.
    //! @param index The index of this sensor ont he robot starting from 0.
    bool getLineTracksIsDetected(std::size_t index) const {return _lineTracksIsDetected.get(index);}

    //! @return The last raw line color receive from robot server (0 to 255).
    //! @param index The index of this sensor ont he robot starting from 0.
    std::uint8_t getLineTracksValue(std::size_t index) const {return _lineTracksValue.get(index);}

    //! @return The last encoder wheel latice count receive from robot server.
    //! @param index The index of this sensor ont he robot starting from 0.
    std::size_t getEncoderWheelValue(std::size_t index) const {return _encoderWheelsValue.get(index);}

    //! @return True if last switch state receive from robot server is pressed.
    //! @param index The index of this sensor ont he robot starting from 0.
    bool getSwitchsIsDetected(std::size_t index) const {return _switchsIsDetected.get(index);}

    //! @return The last ultrasound distance receive from robot server (in pixel on simu).
    //! @param index The index of this sensor ont he robot starting from 0.
    std::size_t getUltrasoundsDistanceDetected(std::size_t index) const {return _ultrasoundsDistanceDetected.get(index);}

    //! @brief Set current motor(s) power(s).
    //! @param value PWM between -1.0 and 1.0.
    //! \{
    enum class MotorIndex {RIGHT = 0, LEFT = 1};
    void setMotorPower(MotorIndex motorIndex, float value);
    void setMotorsPower(float rightValue, float leftValue);
    //! \}

    //! @brief Wait until this specific event has been received
    void waitChanged(EventType eventType);

    //! @brief Wait until one of the listed event has been received
    //! @return The received event
    EventType waitChanged(const std::set<EventType> & eventType);

    //! @brief Wait until this specific event has been received or timeout
    //! @return True if this event has been receive or false if timeout
    template<typename _Rep, typename _Period>
    bool waitChanged(EventType eventType, const std::chrono::duration<_Rep, _Period> & duration);

    //! @brief Wait until one of the listed event has been received
    //! @return The received event if a event of the list has been received or an empty value if timeout
    template<typename _Rep, typename _Period>
    std::optional<EventType> waitChanged(const std::set<EventType> & eventTypes, const std::chrono::duration<_Rep, _Period> & duration);

private:
    Robot(const Robot &) = delete;
    Robot & operator=(const Robot &) = delete;

    void notify(EventType eventType, int changedCount) override;

    static std::string motorIndexToStringHelper(MotorIndex motorIndex);
    EventType waitChangedHelper(std::map<EventType, int> & eventTypes);
    template<typename _Rep, typename _Period>
    std::optional<EventType> waitChangedHelper(std::map<EventType, int> & eventTypes, const std::chrono::duration<_Rep, _Period> & duration);
    std::map<EventType, int> waitParamHelper(std::set<EventType> eventTypes);

    JsonRpcTcpClient _jsonRpcTcpClient;
    Values<std::size_t, EventType, EventType::IR_PROXIMITYS_DISTANCE_DETECTED> _irProximitysDistanceDetected;
    Values<bool, EventType, EventType::LINE_TRACKS_IS_DETECTED> _lineTracksIsDetected;
    Values<std::uint8_t, EventType, EventType::LINE_TRACKS_VALUE> _lineTracksValue;
    Values<std::size_t, EventType, EventType::ENCODER_WHEELS_VALUE> _encoderWheelsValue;
    Values<bool, EventType, EventType::SWITCHS_IS_DETECTED> _switchsIsDetected;
    Values<std::size_t, EventType, EventType::ULTRASOUNDS_DISTANCE_DETECTED> _ultrasoundsDistanceDetected;
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
std::optional<EventType> Robot::waitChanged(const std::set<EventType> & eventTypes, const std::chrono::duration<_Rep, _Period> & duration)
{
    auto eventTypesWithChangedCount = waitParamHelper(eventTypes);
    return waitChangedHelper(eventTypesWithChangedCount, duration);
}

template<typename _Rep, typename _Period>
std::optional<EventType> Robot::waitChangedHelper(std::map<EventType, int> & eventTypes, const std::chrono::duration<_Rep, _Period> & duration)
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
