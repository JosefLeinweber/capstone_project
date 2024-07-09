# ConnectDAWs - VST3 Audio Plugin

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

* has to be reloaded on audio device changes

#### Tools and Libraries

* [JUCE Framework](https://juce.com/)
* [Boost Asio](https://www.boost.org/doc/libs/1_85_0/doc/html/boost_asio.html)
* [Protobuf](https://developers.google.com/protocol-buffers)
* [CMake](https://cmake.org/)
* [Ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
* [Catch2](https://github.com/catchorg/Catch2)
* [pre-commit](https://pre-commit.com/)
* [Clang-tidy](https://clang.llvm.org/extra/clang-tidy/)
* [Clang-format](https://clang.llvm.org/docs/ClangFormat.html)

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

```bash
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
$ cmake --build build --config Debug --target ALL_BUILD
```

or to build tests only change the target to `ConnectDAWsUnitTests` or `ConnectDAWsIntegrationTests`

#### 2. Running

`$ ctest -T test --test-dir build/ -C Debug --output-on-failure`

## Project Documentation

The VST3 plugin is based on the JUCE framework. The plugin is structured in three main parts: the core, the functional part and the user interface. The core part holds the juce plugin structure and intializes the functional part and the user interface. The functional part is responsible for the logic of the plugin, such as establishing a connection and streaming audio. The user interface is responsible for the visual representation of the plugin and the user interaction.

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

### Introduction to JUCE AudioProcessor

The JUCE AudioProcessor is responsible for the audio processing and the communication between the plugin and the DAW. The AudioProcessor has several methods that are called by the DAW, such as `prepareToPlay`, `processBlock` and `releaseResources`. The `processBlock` method is called in real-time and is responsible for processing the audio data. Every time the DAW processes a block of audio data, the `processBlock` method is called.
In the ConnectDAWs plugin the ConnectDAWsAudioProcessor derives from the JUCE AudioProcessor and implements the audio processing and the communication with the functional part of the plugin.

### Relationships Between Main Objects

![alt text](https://github.com/JosefLeinweber/capstone_project/blob/trunk/docs/diagram_of_main_objects-1.png)

### Sequence Diagram

The following sequence diagram shows the an abstraction of the core functionality of the plugin.

![alt text](https://github.com/JosefLeinweber/capstone_project/blob/trunk/docs/sequence_diagram.jpg)

### User Flow Diagram

![alt text](https://github.com/JosefLeinweber/capstone_project/blob/trunk/docs/user_flow_diagram.jpg)

## Next Steps

### Plugin Settings

* Get rid of hardcoded values, make them configurable in the plugin settings.

### Testing

* Restructure testing. Most of current unit tests are integration tests, testing not one but at least two different units. Refactor the code to make it more testable.
* Create automated end-to-end tests to test the plugin as a whole.

### "Refactoring"

* Instead of declaring classes in the header files, create interfaces in the header files and implement them in the cpp files. This will allow mocking the classes and their behavior in the tests.
* Use dependency injection to make the code more testable. This will make the constructor of the classes longer, but will allow to inject mock objects in the tests.
* Rework the message system between gui and cmt.

### Security

* Make sure the configuration data is encripted before sending them over the network.
* Analyze the security of the plugin, what do I need to secure? What are the risks?
  
### Banchmarking

* Develop a benchmarking tool to measure the latency of the plugin. Separate between the latency of the connection and the latency of the audio processing.

### Features

* Implement a method to connect two plugins on different networks.
* Try to decrease the latency of the audio streaming.

## Aknowledgements

1. CppProjectTemplate
   * Author: franneck94
   * Link: https://github.com/franneck94/CppProjectTemplate
2. Wolfsounds Audio Plugin Template
   * Author: JanWilczek
   * Link: https://github.com/JanWilczek/audio-plugin-template
