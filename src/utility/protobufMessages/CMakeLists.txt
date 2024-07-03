message(STATUS "Found Protobuf: ${protobuf_VERSION}")
set(PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/datagram.proto")
# set(GENERATED_PROTO_DIR "${CMAKE_BINARY_DIR}/generated")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${PROTO_FILES}"
                        PROTOC_OUT_DIR "${Protobuf_INCLUDE_DIRS}")
add_library(protobufGenerated STATIC "${PROTO_SRCS}" "${PROTO_HDRS}")
target_link_libraries(protobufGenerated PUBLIC protobuf::libprotobuf)

