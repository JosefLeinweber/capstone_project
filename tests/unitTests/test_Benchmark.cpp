#include "benchmark.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>


TEST_CASE("Benchmark | Constructor", "[Benchmark]")
{
    try
    {
        Benchmark benchmark;
    }
    catch (std::exception &e)
    {
        FAIL("No error should be thrown");
    }
}

TEST_CASE("Benchmark | add value to benchmark")
{
    Benchmark benchmark;
    benchmark.m_networkLatencyBenchmarkData.startTimestamps.push_back(1);
    benchmark.m_networkLatencyBenchmarkData.endTimestamps.push_back(2);
    REQUIRE(benchmark.m_networkLatencyBenchmarkData.startTimestamps.size() ==
            1);
    REQUIRE(benchmark.m_networkLatencyBenchmarkData.endTimestamps.size() == 1);
}

TEST_CASE("Benchmark | finished")
{
    Benchmark benchmark;
    for (int i = 0; i < 100; i++)
    {
        benchmark.m_networkLatencyBenchmarkData.startTimestamps.push_back(1);
        benchmark.m_networkLatencyBenchmarkData.endTimestamps.push_back(2);
    }
    REQUIRE(benchmark.finished());
}
