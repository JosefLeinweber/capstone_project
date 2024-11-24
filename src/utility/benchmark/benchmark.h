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
    std::string name;
    std::vector<std::int64_t> startTimestamps;
    std::vector<std::int64_t> endTimestamps;
};

class Benchmark
{
public:
    Benchmark();

    ~Benchmark();

    bool finished();

    void logBenchmark(const std::vector<std::int64_t> &startTimestamps,
                      const std::vector<std::int64_t> &endTimestamps,
                      const std::string &name,
                      std::shared_ptr<FileLogger> &fileLogger);

    // Function to copy data from source buffer to the ring buffer
    void copyFrom(const std::vector<std::int64_t> &source);

    // Function to copy data from the ring buffer to the destination buffer
    void copyTo(std::vector<std::int64_t> &destination);

    int getNumReadyToRead() const
    {
        return m_fifo.getNumReady();
    }

    int getTotalSize() const
    {
        return m_fifo.getTotalSize();
    }

    std::vector<std::int64_t> m_buffer;

    juce::AbstractFifo m_fifo;

    void debugFunction(std::vector<int64_t> &source);


    std::vector<std::int64_t> m_startTimestamps;
    std::vector<std::int64_t> m_endTimestamps;

private:
};
