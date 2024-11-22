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
    return m_networkLatencyBenchmarkData.startTimestamps.size() == 100;
}

void Benchmark::logBenchmark(BenchmarkData benchmarkData,
                             std::shared_ptr<FileLogger> &fileLogger)
{
    fileLogger->logMessage("Benchmark | " + benchmarkData.name);
    for (int i = 0; i < benchmarkData.startTimestamps.size(); i++)
    {
        fileLogger->logMessage(
            "Benchmark | " + benchmarkData.name + " | " + std::to_string(i) +
            " | " +
            std::to_string(benchmarkData.endTimestamps[i] -
                           benchmarkData.startTimestamps[i]) +
            " ms");
    }
}
