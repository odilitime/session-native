cmake_minimum_required (VERSION 2.8.7)
project(opengem_renderers)

#-----------------------------------------------------------------------
# CPM configuration
#-----------------------------------------------------------------------
cmake_policy(SET CMP0011 NEW)
set(CPM_MODULE_NAME "opengem_renderers")
set(CPM_LIB_TARGET_NAME ${CPM_MODULE_NAME})

if ((DEFINED CPM_DIR) AND (DEFINED CPM_UNIQUE_ID) AND (DEFINED CPM_TARGET_NAME))
  set(CPM_LIB_TARGET_NAME ${CPM_TARGET_NAME})
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CPM_DIR})
  include(CPM)
else()
  set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
  find_package(Git)
  if(NOT GIT_FOUND)
    message(FATAL_ERROR "CPM requires Git.")
  endif()
  if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
    message(STATUS "Cloning repo (https://github.com/sapphio/cpm)")
    execute_process(
    COMMAND "${GIT_EXECUTABLE}" clone https://github.com/sapphio/cpm ${CPM_DIR}
    RESULT_VARIABLE error_code
    OUTPUT_QUIET ERROR_QUIET)
    if(error_code)
      message(FATAL_ERROR "CPM failed to get the hash for HEAD")
    endif()
  endif()
  include(${CPM_DIR}/CPM.cmake)
endif()

# Include CPM modules or externals here (with CPM_AddModule).

# CMAKE_SOURCE_DIR didn't adapt the source_dir to the target_dir
set(CPM_MODULE_CACHING_DIR "${CMAKE_SOURCE_DIR}")
message(STATUS "opengem_renderers caching to ${CPM_MODULE_CACHING_DIR}")

# grab a copy if we don't have one
CPM_EnsureRepoIsCurrent(
  TARGET_DIR ${CPM_MODULE_CACHING_DIR}/opengem/datastructures
  GIT_REPOSITORY https://gitgud.io/Sapphire/opengem/opengem_datastructures
  USE_CACHING 1
)

# SDL needs this
CPM_AddModule("opengem_datastructures"
  SOURCE_DIR ${CPM_MODULE_CACHING_DIR}/opengem/datastructures
  SOURCE_GHOST_GIT_REPO "https://gitgud.io/Sapphire/opengem/opengem_datastructures"
  USE_EXISTING_VER true
# I figure we probably need to export this...
#  EXPORT_MODULE true
)

CPM_InitModule(${CPM_MODULE_NAME})

macro(use_c99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      #message(STATUS "Setting gnu99")
      set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
    endif ()
  else ()
    #message(STATUS "New cmake, setting C99")
    set (CMAKE_C_STANDARD 99)
    # need this for gcc 4.2.1
    set(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
  endif ()
endmacro(use_c99)
macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)

# Fix behavior of CMAKE_C_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

# global include src/
include_directories(src)

# add source
add_subdirectory(src/)

#
# testing
#
#enable_testing()
#add_subdirectory(tests)
