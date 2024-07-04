#include "ConnectDAWs/connectDAWs.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>


TEST_CASE("ConnectDAWs | getIp")
{
    ConnectDAWs connectDAWs;
    std::string ip = connectDAWs.getIp();
    REQUIRE(ip != std::string());
    std::cout << ip << std::endl;
    REQUIRE(ip.length() == 39);
}
