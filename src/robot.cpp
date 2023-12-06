#include "robot.hpp"

#include <json/value.h>
#include <iostream>


Robot::Robot(const std::string & hostIpAddress, uint16_t tcpPort)
    : _jsonRpcTcpClient(hostIpAddress, tcpPort)
    , _irProximitysDistanceDetected(this, EventType::IR_PROXIMITYS_DISTANCE_DETECTED)
    , _lineTracksIsDetected(this, EventType::LINE_TRACKS_IS_DETECTED)
    , _lineTracksValue(this, EventType::LINE_TRACKS_VALUE)
    , _speedsValue(this, EventType::SPEEDS_VALUE)
    , _switchsIsDetected(this, EventType::SWITCHS_IS_DETECTED)
    , _ultrasonicsDistanceDetected(this, EventType::ULTRASONICS_DISTANCE_DETECTED)
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
    _jsonRpcTcpClient.bindNotification("speedValue", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _speedsValue.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("switchIsDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _switchsIsDetected.set(index, params["value"].asBool(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("ultrasonicDistanceDetected", [this](const Json::Value & params){
        std::size_t index = params["index"].asUInt();
        _ultrasonicsDistanceDetected.set(index, params["value"].asUInt(), params["changedCount"].asInt());
    });
    _jsonRpcTcpClient.bindNotification("setIsReady", [this](const Json::Value & params){
        assert(params.isNull());
        _isReadySemaphore.release();
    });
    _jsonRpcTcpClient.startReceive();
}

Robot::~Robot()
{}

std::string Robot::motorIndexToString(MotorIndex motorIndex)
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

void Robot::setMotorSpeed(MotorIndex motorIndex, float value)
{
    Json::Value params;
    params["motorIndex"] = motorIndexToString(motorIndex);
    params["value"] = value;
    _jsonRpcTcpClient.callNotification("setMotorSpeed", params);

}
void Robot::setMotorsSpeed(float rightValue, float leftValue)
{
    Json::Value params;
    params["rightValue"] = rightValue;
    params["leftValue"] = leftValue;
    _jsonRpcTcpClient.callNotification("setMotorsSpeed", params);
}

void Robot::notify(EventType eventType, int changedCount)
{
    {
        std::lock_guard<std::mutex> lk(_eventCvMutex);
        _lastNotifiedEventType.insert_or_assign(eventType, changedCount);
    }
    _eventCv.notify_all();
}

std::map<Robot::EventType, int> Robot::waitParam(std::set<EventType> eventTypes)
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

void Robot::waitChanged(EventType eventType) {
    auto eventTypes = waitParam({eventType});
    waitChanged(eventTypes);
}

Robot::EventType Robot::waitChanged(const std::set<EventType> & eventTypes)
{
    auto eventTypesWithChangedCount = waitParam(eventTypes);
    return waitChanged(eventTypesWithChangedCount);
}

void Robot::waitChanged(EventType eventType, int & changedCount) {
    std::map<EventType, int> eventTypes{{eventType, changedCount}};
    waitChanged(eventTypes);
    changedCount = eventTypes.at(eventType);
}

Robot::EventType Robot::waitChanged(std::map<EventType, int> & eventTypes) {
    std::unique_lock<std::mutex> lk(_eventCvMutex);
    EventType notifiedEventType;
    _eventCv.wait(lk, [eventTypes, &notifiedEventType, lastNotifiedEventType = std::ref(_lastNotifiedEventType)]{
        for (auto eventType : eventTypes) {
            if (lastNotifiedEventType.get().at(eventType.first)>eventType.second)
            {
                lastNotifiedEventType.get().at(eventType.first) = eventType.second;
                notifiedEventType = eventType.first;
                return true;
            }
        }
        return false;
    });
    return notifiedEventType;
}