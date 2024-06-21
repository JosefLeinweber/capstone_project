include(Catch)

set(TEST_NAME "IntegrationTests")
set(TESTS_SOURCES integrationTest_ConnectionManagerThread.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_NAME} ${TESTS_SOURCES})
target_include_directories(${TEST_NAME} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_NAME} PUBLIC
    ${LIBRARY_NAME}
    Catch2::Catch2WithMain
    "AudioBuffer"
    "Host"
    "ProviderThread"
    "ConsumerThread"
    "ConnectionManagerThread"
    "protobufGenerated"
    "SharedValuesForTests"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

catch_discover_tests(${TEST_NAME} TEST_PREFIX "integration.")


