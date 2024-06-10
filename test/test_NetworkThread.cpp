#include <catch2/catch_test_macros.hpp>
#include "AudioBuffer.h"
#include "NetworkThread.h"

TEST_CASE("NetworkThread | Constructor") {
    bool sucess = false;
    try {
        NetworkThread networkThread;
        sucess = true;
    } catch (std::exception& e) {
        FAIL(e.what());
    }

    REQUIRE(sucess);
}




