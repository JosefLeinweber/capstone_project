add_subdirectory(utility)

message(STATUS "Configuring Plugin CMakeLists.txt ...")

project(ConnectDAWs VERSION 0.1.0 LANGUAGES C CXX)

juce_add_plugin(${PROJECT_NAME}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_AUTO_MANIFEST FALSE

    PLUGIN_MANUFACTURER_CODE JSLL
    PLUGIN_CODE Cdaw

    FORMATS Standalone VST3
    PRODUCT_NAME ConnectDAWsExtraName
)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/functional/include/${PROJECT_NAME}")

target_sources(${PROJECT_NAME}
    PRIVATE
        core/PluginProcessor.cpp
        core/PluginEditor.cpp

        core/PluginEditor.h
        core/PluginProcessor.h

        functional/audioBuffer.cpp
        functional/connectDAWs.cpp
        functional/connectionManagerThread.cpp
        functional/consumerThread.cpp
        functional/logger.cpp
        functional/messenger.cpp
        functional/providerThread.cpp
        functional/tcpHost.cpp
        functional/udpHost.cpp

        ${INCLUDE_DIR}/audioBuffer.h
        ${INCLUDE_DIR}/connectDAWs.h
        ${INCLUDE_DIR}/connectionManagerThread.h
        ${INCLUDE_DIR}/consumerThread.h
        ${INCLUDE_DIR}/logger.h
        ${INCLUDE_DIR}/messenger.h
        ${INCLUDE_DIR}/providerThread.h
        ${INCLUDE_DIR}/tcpHost.h
        ${INCLUDE_DIR}/udpHost.h
)

# Sets the include directories of the plugin project.
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/functional/include
)


# Links to all necessary dependencies. The present ones are recommended by JUCE.
# If you use one of the additional modules, like the DSP module, you need to specify it here.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        Boost::boost
        Boost::system
        "protobufGenerated"
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# These definitions are recommended by JUCE.
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)


if (MSVC)
    # Silences some deprecation warnings that (hopefully) will be fixed in a future release of JUCE.
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING)
endif()

if (WIN32)
    # Define _WIN32_WINNT to specify the minimum Windows version
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            _WIN32_WINNT=0x0601
    )
endif()

message(STATUS "Configuring Plugin CMakeLists.txt done")

