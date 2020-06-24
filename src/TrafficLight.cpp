#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> recieveLock(_mutx);
    _condVar.wait(recieveLock, [this]{ return !_messages.empty();} );
    T rxMsg = std::move(_messages.back());
    return rxMsg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> sendLock(_mutx);
    _messages.push_back(std::move(msg));
    _condVar.notify_one();
    }


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(1)
    {
        auto phase = _trafficLightPhase_MQ.receive();
        if(phase == TrafficLightPhase::green) break;
    }
    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a 
    //thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
    std::cout << "simulate has started \n";
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

    auto prevTStart = std::chrono::high_resolution_clock::now();
    srand((std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    while(1)
    {
        auto tStart = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(tStart - prevTStart).count();

        if(((4000 + (std::rand()%2000)) <= duration))
        {
            prevTStart = tStart;
            if (_currentPhase != red)   {_currentPhase = red;}
            else                        {_currentPhase = green;}
            _trafficLightPhase_MQ.send(std::move(_currentPhase));
        }    

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }
}