# ConnectDAWs VST3 Audio Plugin
[![pre-commit](https://github.com/JosefLeinweber/capstone_project/actions/workflows/pre-commit.yml/badge.svg)](https://github.com/JosefLeinweber/capstone_project/actions/workflows/pre-commit.yml)

Source code for the ConnectDAWs VST3 audio plugin. The plugin allows you to connect two DAWs (digital audio workstations) via internet and stream audio between them. The goal is to stream audio with low latency to allow for real-time collaboration between musicians.
The plugin is still in development. The current version is a prototype with basic functionality and a lot of limitations.

**The basic functionality includes:**

* connecting two running ConnectDAWs plugins on a local network

* streaming audio between two connected plugins (with currently long latency)

* basic GUI

* error handling for connection and plugin configuration issues

**Current biggest limitations:**

* only compatible with machines on the same local network

* only compatible with ipV6 addresses

* high latency

* only compatible with Windows

## Getting started with development

*Disclaimer: the project is currently only develped on Windows with the MSVC compiler. Developing on other platforms or other compilers might need additional configuration!*

### Prerequisites

* CMake 3.27 or higher

* C++ 23 compiler
  
* Clang-tidy & Clang-format (optional)

### Building the project

1. Clone the repository --recursively to get the submodules:

```bash
$ git clone --recursive https://github.com/JosefLeinweber/capstone_project.git
$ cd capstone_project
```

2. Create a build directory and run CMake:

```bash
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
$ cmake --build build --config Debug --target ALL_BUILD
```

### Building the VST3 in Release mode

When building in Release mode, tests have to be disabled. This can be done by setting the option `ENABLE_TESTING` to `OFF`. 

```bash
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=OFF
$ cmake --build build --config Release --target ConnectDAWs_VST3
```

### Building and Running Tests

#### 1. Building

Either build everything

`$ cmake --build build --config Debug --target ALL_BUILD`

or build the tests separately:

Unit Tests: `$ cmake --build build --config Debug --target ConnectDAWsUnitTests`

Integreation Tests: `$ cmake --build build --config Debug --target ConnectDAWsIntegrationTests`

#### 2. Running

`& ctest -T test --test-dir build/ -C Debug --output-on-failure`

## Project Documentation

The VST3 plugin is based on the JUCE framework. The plugin is structured in three main parts: the core, the functional part and the user interface. The core part olds the juce plugin structure and intializes the functional part and the user interface. The functional part is responsible for the logic of the plugin, such as establishing a connection and streaming audio. The user interface is responsible for the visual representation of the plugin and the user interaction.

### Project Structure

```bash
├── github/                 # Github configuration and workflows
├── cmake/                  # CMake functions and macros
├── external/               # External dependencies
├── src/                    # Source files
│   ├── core/               # Juce Framework Plugin code
│   ├── functional/         # Logic of the plugin
│   ├── ui/                 # User interface of the plugin
│   ├── utils/              # Utility 
├── tests/                  # Test files
│   ├── integration/        # Integration tests
│   ├── unit/               # Unit tests
├── tools/                  # Tools for development
```
