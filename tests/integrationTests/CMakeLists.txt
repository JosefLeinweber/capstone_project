message("Building Integraion tests")

set(TEST_NAME "ConnectDAWsIntegrationTests")

set(TESTS_SOURCES
integrationTest_ConnectionManagerThread.cpp
#integrationTest_Host_AudioBuffer.cpp
integrationTest_Provider_Consumer.cpp
integrationTest_Benchmark.cpp
)

add_executable(${TEST_NAME} ${TESTS_SOURCES})

target_link_libraries(${TEST_NAME}
PRIVATE
    "SharedValuesForTests"
    "ConnectDAWs"
PUBLIC
    Catch2::Catch2WithMain

)

catch_discover_tests(${TEST_NAME} OUTPUT_DIR "IntegraionTest" TEST_PREFIX "integraion: ")
