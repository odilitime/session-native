find_package(Protobuf REQUIRED)

find_path(PROTOBUF-C_INCLUDE_DIR protobuf-c/protobuf-c.h HINTS ${PC_PROTOBUF-C_INCLUDEDIR} ${PC_PROTOBUF-C_INCLUDE_DIRS} PATH_SUFFIXES protobuf-c)

find_library(PROTOBUF-C_LIBRARY NAMES protobuf-c HINTS ${PC_PROTOBUF-C_LIBDIR} ${PC_PROTOBUF-C_LIBRARY_DIRS})

add_executable(session-native
  session-native
  ${PROTO_SRCS} ${PROTO_HDRS}
)

target_link_libraries(session-native
  opengem::datastructures
  opengem::ui
  opengem::parsers
  opengem::timer
  opengem::net
  opengem::session
  ${PROTOBUF-C_LIBRARY}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_link_libraries(session-native m)
endif()

# for xcode
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../rsrc/07558_CenturyGothic.ttf
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/Resources)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../rsrc/ca-bundle.crt
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/Resources)

# for cli
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../rsrc/07558_CenturyGothic.ttf
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../rsrc/ca-bundle.crt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources)
