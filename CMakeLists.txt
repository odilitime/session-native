cmake_minimum_required (VERSION 2.8.7)
project(session-native LANGUAGES C)

# make VERBOSE=1 for debugging

#find_package(opengem_datastructures)
#------------------------------------------------------------------------------
# Required CPM Setup - no need to modify - See: https://github.com/iauns/cpm
#------------------------------------------------------------------------------
cmake_policy(SET CMP0011 NEW)

set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm_packages" CACHE TYPE STRING)
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

#------------------------------------------------------------------------------
# CPM Modules
#------------------------------------------------------------------------------

set(CPM_MODULE_CACHING_DIR "${CMAKE_SOURCE_DIR}")
message(STATUS "session-native caching to ${CPM_MODULE_CACHING_DIR}")

# ++ MODULE: opengem_session
CPM_EnsureRepoIsCurrent(
  TARGET_DIR ${CPM_MODULE_CACHING_DIR}/opengem/session
  GIT_REPOSITORY https://gitgud.io/Sapphire/opengem/opengem_session.git
  USE_CACHING 1
)
CPM_AddModule("opengem_session"
  SOURCE_DIR ${CPM_MODULE_CACHING_DIR}/opengem/session
  SOURCE_GHOST_GIT_REPO "https://gitgud.io/Sapphire/opengem/opengem_session.git"
  USE_EXISTING_VER true
)

# ++ MODULE: opengem_ui
CPM_EnsureRepoIsCurrent(
  TARGET_DIR ${CPM_MODULE_CACHING_DIR}/opengem/ui
  GIT_REPOSITORY https://gitgud.io/Sapphire/opengem/opengem_ui.git
  USE_CACHING 1
)
CPM_AddModule("opengem_ui"
  SOURCE_DIR ${CPM_MODULE_CACHING_DIR}/opengem/ui
  USE_EXISTING_VER true
  SOURCE_GHOST_GIT_REPO "https://gitgud.io/Sapphire/opengem/opengem_ui.git"
)

CPM_Finish()

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
