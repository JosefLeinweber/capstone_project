#include "benchmark.h"


Benchmark::Benchmark()
{
}

Benchmark::~Benchmark()
{
}

//TODO: change form hardcoded to dynamic
bool Benchmark::finished()
{
    if (m_outgoingBenchmark.m_startTimestamp.size() > 100)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Benchmark::logBenchmark(const std::vector<std::int64_t> &startTimestamps,
                             const std::vector<std::int64_t> &endTimestamps,
                             const std::string &name,
                             std::shared_ptr<FileLogger> &fileLogger)
{
    fileLogger->logMessage("Benchmark | " + name);
    for (size_t i = 0; i < startTimestamps.size(); i++)
    {
        if (i >= endTimestamps.size())
        {
            break;
        }
        fileLogger->logMessage(
            "Benchmark | " + name + " | " + std::to_string(i) + " | " +
            std::to_string(endTimestamps[i] - startTimestamps[i]) + " ms");
    }
    // log size of vectors
    fileLogger->logMessage("Benchmark | " + name + " | startTimestamps size: " +
                           std::to_string(startTimestamps.size()));
    fileLogger->logMessage("Benchmark | " + name + " | endTimestamps size: " +
                           std::to_string(endTimestamps.size()));
}
