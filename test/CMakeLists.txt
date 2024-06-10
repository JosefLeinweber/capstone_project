
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
    
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

# set(TEST_NETWORK "NetworkThreadTests")
# set(TESTS_SOURCES test_NetworkThread.cpp)
# set(TEST_INCLUDES "./")

# add_executable(${TEST_NETWORK} ${TESTS_SOURCES})
# target_include_directories(${TEST_NETWORK} PUBLIC ${TEST_INCLUDES})
# target_link_libraries(${TEST_NETWORK} PUBLIC 
#     ${LIBRARY_NAME} 
#     Catch2::Catch2WithMain
#     "AudioBuffer"
#     "Host"
#     "NetworkThread"
    

#     juce::juce_recommended_config_flags
#     juce::juce_recommended_lto_flags
#     juce::juce_recommended_warning_flags
#     PRIVATE
#     juce::juce_audio_utils
# )

set(TEST_HOST "HostTests")
set(TESTS_SOURCES test_Host.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_HOST} ${TESTS_SOURCES})
target_include_directories(${TEST_HOST} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_HOST} PUBLIC 
    ${LIBRARY_NAME} 
    Catch2::Catch2WithMain
    "AudioBuffer"
    "Host"
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
    "Host"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

set(TEST_PROVIDERTHREAD "ProviderThreadTests")
set(TESTS_SOURCES test_ProviderThread.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_PROVIDERTHREAD} ${TESTS_SOURCES})
target_include_directories(${TEST_PROVIDERTHREAD} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_PROVIDERTHREAD} PUBLIC 
    ${LIBRARY_NAME} 
    Catch2::Catch2WithMain
    "AudioBuffer"
    "Host"
    "ProviderThread"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

catch_discover_tests(${TEST_PROVIDERTHREAD} OUTPUT_DIR "ProviderThreadTests" OUTPUT_PREFIX "ProviderThread_")  

# catch_discover_tests(${TEST_NETWORK} OUTPUT_DIR "NetworkThreadTests" OUTPUT_PREFIX "NetworkThread_")

catch_discover_tests(${TEST_INTEGRATIONS} OUTPUT_DIR "IntegrationTests" OUTPUT_PREFIX "Integration_")

catch_discover_tests(${TEST_HOST} OUTPUT_DIR "HostTests" OUTPUT_PREFIX "Host_")

catch_discover_tests(${TEST_AUDIOBUFFER})
