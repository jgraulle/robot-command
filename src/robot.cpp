#include "robot.hpp"

#include <json/value.h>
#include <iostream>


Robot::Robot(const std::string & hostIpAddress, uint16_t tcpPort)
    : _jsonRpcTcpClient(hostIpAddress, tcpPort)
    , _irProximitysDistanceDetected(this)
    , _lineTracksIsDetected(this)
    , _lineTracksValue(this)
    , _encoderWheelsValue(this)
    , _switchsIsDetected(this)
    , _ultrasoundsDistanceDetected(this)
    , _isReadySemaphore(0)
    , _eventCvMutex()
    , _eventCv()
{
    _jsonRpcTcpClient.bindNotification("irProximityDistanceDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _irProximitysDistanceDetected.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("lineTrackIsDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _lineTracksIsDetected.set(index, params["value"].asBool(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("lineTrackValue", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _lineTracksValue.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("encoderWheelValue", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _encoderWheelsValue.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("switchIsDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _switchsIsDetected.set(index, params["value"].asBool(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("ultrasoundDistanceDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _ultrasoundsDistanceDetected.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("setIsReady", [this](const Json::Value & params){
        assert(params.isNull());
        _isReadySemaphore.release();
    });
    _jsonRpcTcpClient.startReceive();
}

Robot::~Robot()
{}

void Robot::setMotorPower(MotorIndex motorIndex, float value)
{
    Json::Value params;
    params["motorIndex"] = motorIndexToStringHelper(motorIndex);
    params["value"] = value;
    _jsonRpcTcpClient.callNotification("setMotorPower", params);

}
void Robot::setMotorsPower(float rightValue, float leftValue)
{
    Json::Value params;
    params["rightValue"] = rightValue;
    params["leftValue"] = leftValue;
    _jsonRpcTcpClient.callNotification("setMotorsPower", params);
}

void Robot::waitChanged(EventType eventType) {
    auto eventTypes = waitParamHelper({eventType});
    waitChangedHelper(eventTypes);
}

EventType Robot::waitChanged(const std::set<EventType> & eventTypes)
{
    auto eventTypesWithChangedCount = waitParamHelper(eventTypes);
    return waitChangedHelper(eventTypesWithChangedCount);
}

void Robot::notify(EventType eventType, int changedCount)
{
    {
        std::lock_guard<std::mutex> lk(_eventCvMutex);
        _lastNotifiedEventType.insert_or_assign(eventType, changedCount);
    }
    _eventCv.notify_all();
}

std::string Robot::motorIndexToStringHelper(MotorIndex motorIndex)
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

EventType Robot::waitChangedHelper(std::map<EventType, int> & eventTypes) {
    std::unique_lock<std::mutex> lk(_eventCvMutex);
    EventType notifiedEventType;
    _eventCv.wait(lk, [eventTypes, &notifiedEventType, lastNotifiedEventType = std::ref(_lastNotifiedEventType)]{
        for (auto eventType : eventTypes) {
            auto itFind = lastNotifiedEventType.get().find(eventType.first);
            if (   itFind != lastNotifiedEventType.get().end()
                && itFind->second > eventType.second)
            {
                itFind->second = eventType.second;
                notifiedEventType = eventType.first;
                return true;
            }
        }
        return false;
    });
    return notifiedEventType;
}

std::map<EventType, int> Robot::waitParamHelper(std::set<EventType> eventTypes)
{
    std::map<EventType, int> toReturn;
    std::unique_lock<std::mutex> lk(_eventCvMutex);
    for (auto eventType : eventTypes)
    {
        int changedCount = -1;
        auto it = _lastNotifiedEventType.find(eventType);
        if (it != _lastNotifiedEventType.end())
            changedCount = it->second;
        toReturn.insert(std::make_pair(eventType, changedCount));
    }
    return toReturn;
}
