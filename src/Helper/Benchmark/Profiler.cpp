
#include <Helper/Benchmark/Profiler.hpp>

std::mutex Profiler::staticMutex_;
std::map<std::string, std::pair<int, double>> Profiler::savedTimes_;
