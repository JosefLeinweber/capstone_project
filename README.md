# ConnectDAWs VST3 Audio Plugin

Source code for the ConnectDAWs VST3 audio plugin. The plugin allows you to connect two DAWs (digital audio workstations) via internet and stream audio between them. The goal is to stream audio with low latency to allow for real-time collaboration between musicians.
The plugin is still in development, and only the basic functionality is implemented. The basic functionality includes:

* connecting two running ConnectDAWs plugins on a local network

* streaming audio between two connected plugins (long latency)

* basic GUI 

* error handling for connection and plugin configuration issues

## Getting started with development

Disclaimer: the project is currently only develped on Windows with the MSVC compiler. Developing on other platforms might need additional configuration.

### Prerequisites

* CMake 3.27 or higher

* C++ 23 compiler
  
* Clang-tidy & Clang-format (optional)

