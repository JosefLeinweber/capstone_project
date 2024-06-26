cmake_minimum_required(VERSION 3.22)
message(STATUS "Starting CMake configuration")

project(PlaceHolderNAME VERSION 0.1.0 LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 23)

if (APPLE)
    # On Mac, we need to wait for a new JUCE version that fixes the compilation issue
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


option(ENABLE_TESTING "Enable building tests" ON)

option(ENABLE_WARNINGS "Enable compiler warnings" ON)

option(ENABLE_WARNINGS_AS_ERRORS "Enable compiler warnings as errors" OFF)

option(ENABLE_SANITIZERS "Enable sanitizers" ON)

option(ENABLE_LTO "Enable LTO/IPO" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

message(STATUS "Installing with VCPKG")
# Set vcpkg target triplet for static libraries
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet")

include(${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake)

# Set vcpkg target triplet for static libraries

find_package(juce 7.0.12 REQUIRED)

find_package(Boost 1.83.0 REQUIRED COMPONENTS system coroutine context)

find_package(Protobuf REQUIRED)


add_subdirectory(src)


if (ENABLE_TESTING)
    message(STATUS "Testing enabled")
    find_package(catch2 3.5.4 REQUIRED)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)

    enable_testing()
    add_subdirectory(test)
endif()

if (ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled")
    include(Sanitizer)
    add_sanitizer_flags()
endif()

if (ENABLE_LTO)
    message(STATUS "LTO enabled")
    include(LTO)
endif()

message(STATUS "Finished CMake configuration")
