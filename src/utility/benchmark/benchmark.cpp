#include "benchmark.h"


BenchmarkData::BenchmarkData()
{
    m_measurmentRunning = false;
    m_startTimestamps.reserve(2300);
    m_endTimestamps.reserve(2300);
}
BenchmarkData::~BenchmarkData()
{
}

void BenchmarkData::recordStartTimestamp()
{
    // if (m_measurmentRunning)
    // {
    //     return;
    // }
    // else
    // {
    //     m_startTimestamps.push_back(
    //         std::chrono::duration_cast<std::chrono::milliseconds>(
    //             std::chrono::system_clock::now().time_since_epoch())
    //             .count());
    //     m_measurmentRunning = true;
    // }

    m_startTimestamps.push_back(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count());
}

void BenchmarkData::recordEndTimestamp()
{
    // if (m_measurmentRunning)
    // {
    //     m_endTimestamps.push_back(
    //         std::chrono::duration_cast<std::chrono::milliseconds>(
    //             std::chrono::system_clock::now().time_since_epoch())
    //             .count());
    //     m_measurmentRunning = false;
    // }
    // else
    // {
    //     return;
    // }

    m_endTimestamps.push_back(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count());
}


Benchmark::Benchmark()
{
}

Benchmark::~Benchmark()
{
}

//TODO: change form hardcoded to dynamic
bool Benchmark::finished()
{
    return (m_pluginIncomingBenchmark.m_endTimestamps.size() > 1000);
}

std::string Benchmark::calculateLatency(
    const std::vector<std::int64_t> &startTimestamps,
    const std::vector<std::int64_t> &endTimestamps)
{
    std::string latency;

    for (int i = 0; i < startTimestamps.size(); ++i)
    {
        if (i > endTimestamps.size() - 1)
        {
            break;
        }
        latency += std::to_string(endTimestamps[i] - startTimestamps[i]) + ", ";
    }
    return latency;
}

int Benchmark::addDelay(int n)
{

    // if (n <= 1)
    //     return n;
    // return addDelay(n - 1) + addDelay(n - 2);
    return 0;
}

void Benchmark::logBenchmark(const std::vector<std::int64_t> &startTimestamps,
                             const std::vector<std::int64_t> &endTimestamps,
                             const std::string &name,
                             std::shared_ptr<FileLogger> &fileLogger)
{
    fileLogger->logMessage(name + " | start");
    // size of start and end timestamps
    fileLogger->logMessage("size of start timestamps: " +
                           std::to_string(startTimestamps.size()));
    fileLogger->logMessage("size of end timestamps: " +
                           std::to_string(endTimestamps.size()));
    std::string latency = calculateLatency(startTimestamps, endTimestamps);
    fileLogger->logMessage("latency: " + latency);

    std::string startTimestampsString;
    for (auto &timestamp : startTimestamps)
    {
        startTimestampsString += std::to_string(timestamp) + ", ";
    }
    fileLogger->logMessage("start timestamps: " + startTimestampsString);
    std::string endTimestampsString;
    for (auto &timestamp : endTimestamps)
    {
        endTimestampsString += std::to_string(timestamp) + ", ";
    }

    fileLogger->logMessage("end timestamps: " + endTimestampsString);
}
