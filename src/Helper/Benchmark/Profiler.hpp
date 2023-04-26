
#pragma once

#include <string>
#include <chrono>
#include <iostream>
#include <mutex>
#include <map>

class Chronometer {

    public:

        void start();

        double getElapsedTime();


    private:
    	std::chrono::high_resolution_clock::time_point startTime_;

};

class Profiler {

    public:
        Profiler(std::string id);

        ~Profiler();

        static void printProfiled(std::ostream& stream);

    private:
        Chronometer chrono;
        std::string id_;

        static std::mutex staticMutex_;
        static std::map<std::string, std::pair<int, double>> savedTimes_;

};
