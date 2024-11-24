#include "ConnectDAWS/connectDAWs.h"
#include "benchmark.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <thread>


TEST_CASE("Benchmark | in ConnectDAWs | copyFrom")
{
    std::shared_ptr<Benchmark> benchmarkPtr = std::make_shared<Benchmark>();
    std::jthread thread([&benchmarkPtr]() {
        std::vector<std::int64_t> source;
        source.push_back(0);
        benchmarkPtr->copyFrom(source);
    });

    thread.join();

    REQUIRE(benchmarkPtr->m_fifo.getNumReady() == 1);
    REQUIRE(benchmarkPtr->m_buffer[0] == 0);
}
