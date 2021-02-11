add_executable(session-native
  session-native
  snode.h
  snode.c
)

target_link_libraries(session-native
  opengem::datastructures
  opengem::ui
  opengem::parsers
  opengem::timer
  opengem::net
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
