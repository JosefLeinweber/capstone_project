# Find Python interpreter
find_package(Python REQUIRED COMPONENTS Interpreter)

# Check if Python was found
if (NOT Python_FOUND)
    message(FATAL_ERROR "Python interpreter not found. Please install Python and try again.")
endif()

# Create a Python virtual environment
set(VENV_DIR "${CMAKE_BINARY_DIR}/python_venv")
execute_process(
    COMMAND ${Python_EXECUTABLE} -m venv ${VENV_DIR}
    RESULT_VARIABLE VENV_CREATION_RESULT
)

if (NOT VENV_CREATION_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to create Python virtual environment.")
else()
    message(STATUS "Successfully created Python virtual environment at ${VENV_DIR}.")
endif()

# Define the Python executable from the virtual environment
if (WIN32)
    set(VENV_PYTHON "${VENV_DIR}/Scripts/python.exe")
else()
    set(VENV_PYTHON "${VENV_DIR}/bin/python")
endif()

# Upgrade pip and install required pip packages using the virtual environment's Python
execute_process(
    COMMAND ${VENV_PYTHON} -m pip install --upgrade pip
    RESULT_VARIABLE PIP_UPGRADE_RESULT
)

if (NOT PIP_UPGRADE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to upgrade pip in the virtual environment.")
else()
    message(STATUS "Successfully upgraded pip in the virtual environment.")
endif()

# Install required pip packages
execute_process(
    COMMAND ${VENV_PYTHON} -m pip install protobuf google3 numpy
    RESULT_VARIABLE PIP_INSTALL_RESULT
)

if (NOT PIP_INSTALL_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to install required Python packages in the virtual environment.")
else()
    message(STATUS "Successfully installed required Python packages in the virtual environment.")
endif()
