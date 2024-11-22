// a class to keep track of the different buffers needed for the timestamp benchmarking
// and the benchmarking itself
#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "logger.h"

struct BenchmarkData
{
    std::string name;
    std::vector<std::int64_t> startTimestamps;
    std::vector<std::int64_t> endTimestamps;
};

class Benchmark
{
public:
    Benchmark();
    ~Benchmark();

    void startBenchmark(std::string name);
    void endBenchmark(std::string name);
    void logBenchmark(BenchmarkData benchmarkData,
                      std::shared_ptr<FileLogger> &fileLogger);

    bool finished();

    BenchmarkData m_networkLatencyBenchmarkData;
    BenchmarkData m_outgoingAudioBenchmarkData;
    BenchmarkData m_incomingAudioBenchmarkData;
};
