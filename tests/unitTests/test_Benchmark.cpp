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


TEST_CASE("Benchmark | copyFrom ")
{
    Benchmark benchmark;
    std::vector<std::int64_t> source(100, 0);
    benchmark.copyFrom(source);
    REQUIRE(benchmark.m_fifo.getNumReady() == 99);
    REQUIRE(benchmark.m_buffer[0] == 0);
}

TEST_CASE("Benchmark | copyTo ")
{
    // GIVEN: Benchmark object with 100 elements in the buffer
    Benchmark benchmark;
    std::vector<std::int64_t> source(100, 1);
    benchmark.copyFrom(source);
    // WHEN: copyTo is called
    std::vector<std::int64_t> destination(100, 0);
    benchmark.copyTo(destination);
    // THEN: destination should be equal to source
    REQUIRE(benchmark.m_fifo.getNumReady() == 0);
    REQUIRE(destination[0] == 1);
}

TEST_CASE("Benchmark | finished ")
{
    // GIVEN: Benchmark object with 100 elements in the buffer
    Benchmark benchmark;
    std::vector<std::int64_t> source(100, 1);
    benchmark.copyFrom(source);
    // WHEN: finished is called
    bool result = benchmark.finished();
    // THEN: result should be true
    REQUIRE(result == true);
}
