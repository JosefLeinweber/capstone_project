// a class to keep track of the different buffers needed for the timestamp benchmarking
// and the benchmarking itself
#pragma once

#include <chrono>
#include <string>
#include <vector>

struct BenchmarkData
{
    std::string name;
    std::vector<std::chrono::high_resolution_clock::time_point> startTimestamps;
    std::vector<std::chrono::high_resolution_clock::time_point> endTimestamps;
};

class Benchmark
{
public:
    Benchmark();
    ~Benchmark();

    void startBenchmark(std::string name);
    void endBenchmark(std::string name);
    void printBenchmark(std::string name);

    BenchmarkData m_networkLatencyBenchmarkData;
    BenchmarkData m_outgoingAudioBenchmarkData;
    BenchmarkData m_incomingAudioBenchmarkData;
};
