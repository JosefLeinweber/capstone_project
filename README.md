# ConnectDAWs VST3 Audio Plugin

Source code for the ConnectDAWs VST3 audio plugin. The plugin allows you to connect two DAWs (digital audio workstations) via internet and stream audio between them. The goal is to stream audio with low latency to allow for real-time collaboration between musicians.
The plugin is still in development, and only the basic functionality is implemented.

**The basic functionality includes:**

* connecting two running ConnectDAWs plugins on a local network

* streaming audio between two connected plugins (with currently long latency)

* basic GUI

* error handling for connection and plugin configuration issues

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
$ cmake -S . -B build
$ cmake --build build --config Debug --target ALL_BUILD
```

### Running tests

To run the tests, execute the following command in the main directory:

```bash
"C:\Program Files\CMake\bin\cmake.EXE" --build c:/Users/herr_/CODE/bachelor/v0.1.0/capstone_project/build --config Debug --target ConnectDAWsUnitTests -j 14 --

ctest -j14 -C Debug -T test --output-on-failure 
```
