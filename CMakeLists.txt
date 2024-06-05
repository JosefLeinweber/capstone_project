
# Mandatory line, sets the minimum version of CMake that should be used with this repository.
# I specified 3.22 because I trust it. However, currently I have 3.26 installed on my machine.
# To verify your version run
# $ cmake --version
cmake_minimum_required(VERSION 3.22)
message(STATUS "Starting CMake configuration")
# Sets a few variables, like PROJECT_NAME
project(WolfSoundAudioPluginTemplate)

# Always use the newest C++ standard on green-field projects if possible.
set(CMAKE_CXX_STANDARD 23)
if (APPLE)
    # On Mac, we need to wait for a new JUCE version that fixes the compilation issue
    set(CMAKE_CXX_STANDARD 20)
endif()

# I like to download the dependencies to the same folder as the project.
# If you want to install them system wide, set CPM_SOURCE_CACHE with the path to the dependencies
# either as an environment variable or pass it to the cmake script with -DCPM_SOURCE_CACHE=<path>.
# set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)


message(STATUS "Installing with VCPKG")
include(    ${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake)
find_package(juce 7.0.12 REQUIRED)
find_package(Boost 1.83.0 REQUIRED COMPONENTS system coroutine context)
find_package(catch2 3.5.4 REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include_directories(${Boost_INCLUDE_DIRS})



include(CTest)
enable_testing()
add_subdirectory(src)
add_subdirectory(plugin)
add_subdirectory(test)


message(STATUS "Finished CMake configuration")