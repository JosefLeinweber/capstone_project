add_subdirectory(integrationTests)
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

set(TEST_CONSUMERTHREAD "ConsumerThreadTests")
set(TESTS_SOURCES test_ConsumerThread.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_CONSUMERTHREAD} ${TESTS_SOURCES})
target_include_directories(${TEST_CONSUMERTHREAD} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_CONSUMERTHREAD} PUBLIC
    ${LIBRARY_NAME}
    Catch2::Catch2WithMain
    "AudioBuffer"
    "Host"
    "ConsumerThread"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

set(TEST_CONNCETIONMANAGERTHREAD "ConnectionManagerThreadTests")
set(TESTS_SOURCES test_ConnectionManagerThread.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_CONNCETIONMANAGERTHREAD} ${TESTS_SOURCES})
target_include_directories(${TEST_CONNCETIONMANAGERTHREAD} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_CONNCETIONMANAGERTHREAD} PUBLIC
    ${LIBRARY_NAME}
    Catch2::Catch2WithMain
    "AudioBuffer"
    "Host"
    "ConnectionManagerThread"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)
catch_discover_tests(${TEST_CONNCETIONMANAGERTHREAD} OUTPUT_DIR "ConnectionManagerThreadTests" OUTPUT_PREFIX "ConnectionManagerThread_")

catch_discover_tests(${TEST_PROVIDERTHREAD} )

catch_discover_tests(${TEST_CONSUMERTHREAD} OUTPUT_DIR "ConsumerThreadTests" OUTPUT_PREFIX "ConsumerThread_")

catch_discover_tests(${TEST_HOST} OUTPUT_DIR "HostTests" OUTPUT_PREFIX "Host_")

catch_discover_tests(${TEST_AUDIOBUFFER})

