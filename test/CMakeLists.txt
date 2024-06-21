add_subdirectory(integrationTests)
add_subdirectory(helpers)
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
    "SharedValuesForTests"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

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
    "SharedValuesForTests"
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
    "SharedValuesForTests"
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
    "SharedValuesForTests"
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
    "SharedValuesForTests"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

set(TEST_TCPHOST "TcpHostTests")
set(TESTS_SOURCES test_TcpHost.cpp)
set(TEST_INCLUDES "./")

add_executable(${TEST_TCPHOST} ${TESTS_SOURCES})
target_include_directories(${TEST_TCPHOST} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_TCPHOST} PUBLIC
    ${LIBRARY_NAME}
    Catch2::Catch2WithMain
    "TcpHost"
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    PRIVATE
    juce::juce_audio_utils
)

#catch_discover_tests(${TEST_TCPHOST} OUTPUT_DIR "TcpHostTests" OUTPUT_PREFIX "TcpHost_")

catch_discover_tests(${TEST_CONNCETIONMANAGERTHREAD} OUTPUT_DIR "ConnectionManagerThreadTests" OUTPUT_PREFIX "ConnectionManagerThread_")

catch_discover_tests(${TEST_PROVIDERTHREAD} )

catch_discover_tests(${TEST_CONSUMERTHREAD})

# catch_discover_tests(${TEST_HOST})

# catch_discover_tests(${TEST_AUDIOBUFFER})

