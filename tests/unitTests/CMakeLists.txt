message("Building Unit tests")

set(TEST_NAME "ConnectDAWsUnitTests")

set(TESTS_SOURCES
    test_RingBuffer.cpp
    test_UdpHost.cpp
    test_ProviderThread.cpp
    test_ConsumerThread.cpp
    test_ConnectionManagerThread.cpp
    test_TcpHost.cpp
    test_ConnectDAWs.cpp
)

add_executable(${TEST_NAME} ${TESTS_SOURCES})

target_link_libraries(${TEST_NAME}
PRIVATE
    "SharedValuesForTests"
    "ConnectDAWs"
PUBLIC
    Catch2::Catch2WithMain

)

catch_discover_tests(${TEST_NAME})
