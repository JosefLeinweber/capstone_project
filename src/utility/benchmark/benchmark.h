// a class to keep track of the different buffers needed for the timestamp benchmarking
// and the benchmarking itself
#pragma once

#include <chrono>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <string>
#include <vector>

#include "logger.h"

struct BenchmarkData
{
    BenchmarkData();
    ~BenchmarkData();
    std::string name;
    std::vector<std::int64_t> m_startTimestamps;
    std::vector<std::int64_t> m_endTimestamps;
    std::atomic<bool> m_measurmentRunning;

    void recordStartTimestamp();
    void recordEndTimestamp();
};

class Benchmark
{
public:
    Benchmark();

    ~Benchmark();

    bool finished();

    int addDelay(int delay_operations);


    void logBenchmark(const std::vector<std::int64_t> &startTimestamps,
                      const std::vector<std::int64_t> &endTimestamps,
                      const std::string &name,
                      std::shared_ptr<FileLogger> &fileLogger);
    std::string calculateLatency(
        const std::vector<std::int64_t> &startTimestamps,
        const std::vector<std::int64_t> &endTimestamps);

    BenchmarkData m_networkBenchmark;
    BenchmarkData m_pluginOutgoingBenchmark;
    BenchmarkData m_pluginIncomingBenchmark;
};
