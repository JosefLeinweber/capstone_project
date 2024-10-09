find_package(Doxygen)
message(STATUS "Doxygen found: ${DOXYGEN_FOUND}")

if(DOXYGEN_FOUND)
    message(STATUS "Doxygen found, building documentation")
    add_custom_target(
        docs
        ${DOXYGEN_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)

else()
    message(STATUS "Doxygen not found, documentation will not be built")

endif()
