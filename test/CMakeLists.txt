
include(Catch)


set(TEST_AUDIOBUFFER "AudioBufferTests")
set(TESTS_SOURCES test_AudioBuffer.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_AUDIOBUFFER} ${TESTS_SOURCES})
target_include_directories(${TEST_AUDIOBUFFER} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_AUDIOBUFFER} PUBLIC 
    ${LIBRARY_NAME} 
    Catch2::Catch2WithMain
    "AudioBuffer"
    "NetworkingThread"
    
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

set(TEST_NETWORKINGTHREAD "NetworkingThreadTests")
set(TESTS_SOURCES test_NetworkingThread.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_NETWORKINGTHREAD} ${TESTS_SOURCES})
target_include_directories(${TEST_NETWORKINGTHREAD} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_NETWORKINGTHREAD} PUBLIC 
    ${LIBRARY_NAME} 
    Catch2::Catch2WithMain
    "AudioBuffer"
    "NetworkingThread"
    
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

set(TEST_SERVER "ServerTests")
set(TESTS_SOURCES test_Server.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_SERVER} ${TESTS_SOURCES})
target_include_directories(${TEST_SERVER} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_SERVER} PUBLIC 
    ${LIBRARY_NAME} 
    Catch2::Catch2WithMain
    "AudioBuffer"
    "Server"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

set(TEST_INTEGRATIONS "IntegrationTests")
set(TESTS_SOURCES test_Integration.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_INTEGRATIONS} ${TESTS_SOURCES})
target_include_directories(${TEST_INTEGRATIONS} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_INTEGRATIONS} PUBLIC 
    ${LIBRARY_NAME} 
    Catch2::Catch2WithMain
    "AudioBuffer"
    "Server"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

catch_discover_tests(${TEST_NETWORKINGTHREAD} OUTPUT_DIR "NetworkingThreadTests" OUTPUT_PREFIX "NetworkingThread_")

catch_discover_tests(${TEST_INTEGRATIONS} OUTPUT_DIR "IntegrationTests" OUTPUT_PREFIX "Integration_")

catch_discover_tests(${TEST_SERVER} OUTPUT_DIR "ServerTests" OUTPUT_PREFIX "Server_")

catch_discover_tests(${TEST_AUDIOBUFFER})
