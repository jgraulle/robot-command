#ifndef VALUES_HPP
#define VALUES_HPP

#include <optional>


template<typename EventType>
class IRobot
{
public:
    virtual ~IRobot() {}
    virtual void notify(EventType eventType, int changedCount) = 0;
};


template<typename T, typename EventType, EventType EVENT_TYPE_VALUE>
class Values
{
public:
    Values(IRobot<EventType> * robot) : _robot(robot) {}

    inline T get(std::size_t index) const {const std::lock_guard<std::mutex> lock(_mutex); if (index>=_values.size()) return {}; return _values.at(index)._value.value();}
    inline int getChangedCount(std::size_t index) const {const std::lock_guard<std::mutex> lock(_mutex); if (index>=_values.size()) return 0; return _values.at(index)._changedCount.value();}
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
        _robot->notify(EVENT_TYPE_VALUE, changedCount);
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
    IRobot<EventType> * _robot;
};

#endif 
