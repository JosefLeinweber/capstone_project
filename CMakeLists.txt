cmake_minimum_required(VERSION 3.22)
message(STATUS "Starting CMake configuration")

project(CapstoneProject VERSION 0.1.0 LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 23)
if (APPLE)
    # On Mac, we need to wait for a new JUCE version that fixes the compilation issue
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_TESTING "Enable building tests" OFF)

option(ENABLE_WARNINGS "Enable compiler warnings" ON)
option(ENABLE_WARNINGS_AS_ERRORS "Enable compiler warnings as errors" OFF)

option(ENABLE_SANITIZERS "Enable sanitizers" ON)

option(ENABLE_LTO "Enable LTO/IPO" OFF)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

message(STATUS "Installing with VCPKG")
include(    ${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake)
find_package(JUCE CONFIG REQUIRED)
find_package(Boost 1.83.0 REQUIRED COMPONENTS system coroutine context)
find_package(Protobuf REQUIRED)
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")


if(protobuf_FOUND)
    message(STATUS "Found Protobuf: ${protobuf_VERSION}")
    set(PROTO_FILES "${CMAKE_SOURCE_DIR}/src/proto/datagram.proto")
    # set(GENERATED_PROTO_DIR "${CMAKE_BINARY_DIR}/generated")
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${PROTO_FILES}"
                            PROTOC_OUT_DIR "${Protobuf_INCLUDE_DIRS}")
    message(STATUS "Generated Protobuf sources: ${PROTO_SRCS}")
    message(STATUS "Generated Protobuf headers: ${PROTO_HDRS}")
    add_library(protobufGenerated "${PROTO_SRCS}" "${PROTO_HDRS}")
    target_link_libraries(protobufGenerated PUBLIC protobuf::libprotobuf)
endif()


add_subdirectory(plugin)
add_subdirectory(src)


if (ENABLE_TESTING)
    find_package(catch2 3.5.4 REQUIRED)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)

    enable_testing()
    add_subdirectory(test)
endif()

if (ENABLE_SANITIZERS)
    include(Sanitizer)
    add_sanitizer_flags()
endif()

if (ENABLE_LTO)
    include(LTO)
endif()

message(STATUS "Finished CMake configuration")
