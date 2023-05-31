#pragma once
#include <chrono>

class Timer {
private:
    using Clock = std::chrono::high_resolution_clock;
    std::chrono::time_point<Clock> start_time;

public:
    void start() {
        start_time = Clock::now();
    }

    double elapsedMilliseconds() {
        auto end_time = Clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
        return elapsed.count();
    }
};

