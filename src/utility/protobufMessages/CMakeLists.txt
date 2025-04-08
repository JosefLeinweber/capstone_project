message(STATUS "Found Protobuf: ${protobuf_VERSION}")
set(PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/datagram.proto")
# set(GENERATED_PROTO_DIR "${CMAKE_BINARY_DIR}/generated")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${PROTO_FILES}"
                        PROTOC_OUT_DIR "${Protobuf_INCLUDE_DIRS}")
protobuf_generate_python(PY_SOURCES "${PROTO_FILES}"
                        PROTOC_OUT_DIR "${CMAKE_SOURCE_DIR}/tests/end2endTests")
add_library(protobufGenerated STATIC "${PROTO_SRCS}" "${PROTO_HDRS}")
target_link_libraries(protobufGenerated PUBLIC protobuf::libprotobuf)
add_custom_target(protobufGeneratedPython ALL DEPENDS "${PY_SOURCES}")

