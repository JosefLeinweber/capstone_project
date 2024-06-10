set(LIBRARY_NAME "NetworkThread")

set(LIBRARY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}.cpp")
set(LIBRARY_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}.h")
set(LIBRARY_INCLUDES "./" "${CMAKE_BINARY_DIR}/configured_files/include")



add_library("${LIBRARY_NAME}" "${LIBRARY_SOURCES}" "${LIBRARY_HEADERS}")
target_include_directories("${LIBRARY_NAME}" PUBLIC "${LIBRARY_INCLUDES}")
target_link_libraries("${LIBRARY_NAME}"
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        "AudioBuffer"
        "Host"
)