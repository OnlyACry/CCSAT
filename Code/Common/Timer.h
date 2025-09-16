#pragma once
#include<iostream>
#include<string>
#include <chrono>
#include"Config/TypeDef.h"
#ifndef TIMER_DEF
#define TIMER_DEF

namespace WorkSpace{
class Timer{
    private:

    std::chrono::steady_clock::time_point startTime;       // Start time of the timer
    double timeLimit;                                         // Time limit in seconds set from configuration
    double runTime;

    public:

    Timer(const Config& config) {
        timeLimit = std::stoi(config.getOption(Config::TimeLimit))*1000;
        startTime = std::chrono::steady_clock::now();      // Capture the start time
    }

    /// @brief Function to check if the time limit has been exceeded
    /// @return 
    bool isTimeOut()
    {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        // printf("timeout check: startTime:%lld, currentTime:%lld, elapsedTime:%lld\n", startTime, currentTime, elapsedTime);
        return elapsedTime >= timeLimit;
    }
    // other func

    /// @brief Get the time the program has been running
    /// @return Elapsed time in seconds
    void getElapsedTime() {
        auto currentTime = std::chrono::steady_clock::now();
        runTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count()/1000.0;
        // std::cout << "currentTime: " << std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count()
        // << ", runTime: " << runTime
        // << ", start time: " << std::chrono::duration_cast<std::chrono::milliseconds>(startTime.time_since_epoch()).count()
        // << std::endl;
    }

    double getRunTime() const {
        return runTime;
    }

};
} // namespace WorkSpace
#endif
