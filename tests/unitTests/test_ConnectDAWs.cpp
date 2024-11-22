#include "ConnectDAWs/connectDAWs.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>

// test initalization of ConnectDAWs

TEST_CASE("ConnectDAWs | Constructor", "[ConnectDAWs]")
{
    // Validate that the ConnectDAWs object is initialized correctly
    try
    {
        ConnectDAWs connectDAWs;
    }
    catch (const std::exception &e)
    {
        FAIL("ConnectDAWs object could not be initialized");
    }
}
