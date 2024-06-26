
set(LIBRARY_NAME "SharedValuesForTests")

set(LIBRARY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sharedValues.cpp")
set(LIBRARY_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/sharedValues.h")
set(LIBRARY_INCLUDES "./" "${CMAKE_BINARY_DIR}/configured_files/include")



add_library("${LIBRARY_NAME}" "${LIBRARY_SOURCES}" "${LIBRARY_HEADERS}")
target_include_directories("${LIBRARY_NAME}" PUBLIC "${LIBRARY_INCLUDES}" "${GENERATED_PROTO_DIR}")
target_link_libraries("${LIBRARY_NAME}"
    PUBLIC
        "protobufGenerated"
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        PRIVATE
        juce_audio_basics

)
