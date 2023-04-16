
#pragma once

#include <string>
#include <chrono>
#include <iostream>
#include <mutex>
#include <map>

class Chronometer {

    public:

        Chronometer() {}

        void start() {
            startTime_ = std::chrono::high_resolution_clock::now();
        }

        double getElapsedTime() {
            std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> fsec = endTime - startTime_;
            return fsec.count();
        }


    private:
    	std::chrono::high_resolution_clock::time_point startTime_;

};

class Profiler {

    public:
        Profiler(std::string id) : id_(id) {
            chrono.start();
        }

        ~Profiler() {

            double elapsedTime = chrono.getElapsedTime();

            std::lock_guard<std::mutex> lock(Profiler::staticMutex_);

            std::pair<int, double>& savedTime = Profiler::savedTimes_.emplace(id_, std::make_pair<int, double>(0, 0.0f)).first->second;
            ++savedTime.first;
            savedTime.second += elapsedTime;

        }

        static void printProfiled(std::ostream& stream) {
            
            stream << std::fixed << "\nProfiled functions :" << std::endl;

            std::lock_guard<std::mutex> lock(Profiler::staticMutex_);
            for (auto const& savedTime : savedTimes_) {
                stream << "\t- " << savedTime.first << " : " << (savedTime.second.second/(double)savedTime.second.first) << std::endl;;
            }

        }

    private:
        Chronometer chrono;
        std::string id_;

        static std::mutex staticMutex_;
        static std::map<std::string, std::pair<int, double>> savedTimes_;

};
