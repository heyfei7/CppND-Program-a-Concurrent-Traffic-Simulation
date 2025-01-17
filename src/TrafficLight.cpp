#include <iostream>
#include <random>
#include <functional>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> _lock(_mutex);
    _condition.wait(_lock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> _lock(_mutex);
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight() {}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    if (getCurrentPhase() == TrafficLightPhase::green)
    {
        return;
    }
    while (true)
    {
        TrafficLightPhase phase = _queue.receive();
        if (phase == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(4000,6000);
auto dice = std::bind ( distribution, generator );

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::chrono::milliseconds cycleDuration(dice());
    std::chrono::milliseconds delta;

    std::chrono::time_point<std::chrono::steady_clock> cycleStart = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> currentTime;

    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    while(true)
    {
        currentTime = std::chrono::steady_clock::now();
        delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - cycleStart);
        if (delta >= cycleDuration)
        {
            if (getCurrentPhase() == TrafficLightPhase::green)
            {
                _currentPhase = TrafficLightPhase::red;
            }
            else
            {
                _currentPhase = TrafficLightPhase::green;
            }
            _queue.send(std::move(_currentPhase));
            cycleDuration = std::chrono::milliseconds(dice());
            cycleStart = std::chrono::steady_clock::now();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}