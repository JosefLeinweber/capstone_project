#include "benchmark.h"


Benchmark::Benchmark() : m_fifo(100)
{
    m_buffer.resize(100);
}

Benchmark::~Benchmark()
{
}

//TODO: change form hardcoded to dynamic
bool Benchmark::finished()
{
    //! Dumpy implementation for debugging
    return false;
}

void Benchmark::logBenchmark(const std::vector<std::int64_t> &startTimestamps,
                             const std::vector<std::int64_t> &endTimestamps,
                             const std::string &name,
                             std::shared_ptr<FileLogger> &fileLogger)
{
    fileLogger->logMessage("Benchmark | " + name);
    for (size_t i = 0; i < startTimestamps.size(); i++)
    {
        fileLogger->logMessage(
            "Benchmark | " + name + " | " + std::to_string(i) + " | " +
            std::to_string(endTimestamps[i] - startTimestamps[i]) + " ms");
    }
}
void Benchmark::copyFrom(const std::vector<std::int64_t> &source)
{
    auto writeHandle = m_fifo.write(source.size());

    if (writeHandle.blockSize1 > 0)
    {
        std::copy(source.begin(),
                  source.begin() + writeHandle.blockSize1,
                  m_buffer.begin() + writeHandle.startIndex1);
    }
    if (writeHandle.blockSize2 > 0)
    {
        std::copy(source.begin() + writeHandle.blockSize1,
                  source.end(),
                  m_buffer.begin() + writeHandle.startIndex2);
    }
}

void Benchmark::copyTo(std::vector<std::int64_t> &destination)
{
    auto readHandle = m_fifo.read(destination.size());

    if (readHandle.blockSize1 > 0)
    {
        std::copy(m_buffer.begin() + readHandle.startIndex1,
                  m_buffer.begin() + readHandle.startIndex1 +
                      readHandle.blockSize1,
                  destination.begin());
    }
    if (readHandle.blockSize2 > 0)
    {
        std::copy(m_buffer.begin() + readHandle.startIndex2,
                  m_buffer.begin() + readHandle.startIndex2 +
                      readHandle.blockSize2,
                  destination.begin() + readHandle.blockSize1);
    }
}

void Benchmark::debugFunction(std::vector<int64_t> &source)
{
    std::cout << "Benchmark | debugFunction | " << std::endl;
}
