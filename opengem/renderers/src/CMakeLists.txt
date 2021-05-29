option(PREFER_SDL2 "use SDL2 instead of SDL1 code if detected" )
option(IGNORE_GLFW "use SDLx instead of GLFW code if detected" )

# find 3rd party libraries
find_package(OpenGL)

find_package(Freetype)
if(Freetype_FOUND)
  message(STATUS "FreeType Version: ${FREETYPE_VERSION_STRING}")
endif()

# do GLFW detection
if (NOT IGNORE_GLFW)
  find_package(glfw3 3.3 CONFIG PATH_SUFFIXES "/cmake/glfw3")
  if(glfw3_FOUND)
    message(STATUS "glfw3.3 detected, vulkan enabled")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAS_VULKAN -DGLFW33")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_VULKAN -DGLFW33")
  else()
    # fallback to 3.2
    find_package(glfw3 3.2 CONFIG PATH_SUFFIXES "/cmake/glfw3")
    if(glfw3_FOUND)
      message(STATUS "glfw3.2 detected, vulkan enabled")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAS_VULKAN -DGLFW32")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_VULKAN -DGLFW32")
      #if python exists enable plugins that depend on python, this allows the user
      #to turn off the python plugins even if python is found
      #option(ENABLE_glfw_plugins "Turn on Plugins that depend on glfw3 existing" ${glfw3_FOUND})
    else()
      # fallback to 3.1 for 10.6
      find_package(glfw3 3.1 CONFIG PATH_SUFFIXES "/cmake/glfw3")
      if(glfw3_FOUND)
        message(STATUS "glfw3.1 detected, vulkan disabled")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGLFW31")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLFW31")
      else()
        find_package(glfw3 3.0 CONFIG PATH_SUFFIXES "/cmake/glfw3")
        if(glfw3_FOUND)
          message(STATUS "glfw3.0 detected, vulkan disabled, cursors disabled")
        endif()
      endif()
    endif()
  endif()
  if(glfw3_FOUND)
    # glfw 3.2 on mac 10.14 adjustments
    # HINTS "/usr/local/include/"
    find_path(OG_GLFW_INCLUDE NAMES "GLFW/glfw3.h")
    message(STATUS "GLFW3_LIBRARY: ${GLFW3_LIBRARY}")
    if (NOT GLFW3_LIBRARY)
      #set(GLFW3_LIBRARY 'glfw3')
    endif()
    message(STATUS "GLFW3_LIBRARY: ${GLFW3_LIBRARY}")
    message(STATUS "glfw.h: ${OG_GLFW_INCLUDE}")
    message(STATUS "GLFW libraries: ${GLFW_LIBRARIES} ${GLFW_LIBRARY} in ${GLFW_LIBRARY_DIR}")
    message(STATUS "glfw libraries: ${glfw_LIBRARIES} ${glfw_LIBRARY} in ${glfw_LIBRARY_DIR}")
    # GLFW3_LIBRARY on 10.6 (has full path), ubuntu does not use GLFW3_x
    message(STATUS "GLFW3 libraries: ${GLFW3_LIBRARIES} ${GLFW3_LIBRARY} in ${GLFW3_LIBRARY_DIR}")
    message(STATUS "glfw3 libraries: ${glfw3_LIBRARIES} ${glfw3_LIBRARY} in ${glfw3_LIBRARY_DIR}")
    # message(STATUS "glfw static libraries: ${GLFW_STATIC_LIBRARIES}")
    # GLFW3_INCLUDE_DIR on 10.6, ubuntu doesn't have either of these...
    # 10.14 doesn't have any either
    message(STATUS "GLFW includes: ${GLFW_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} ${GLFW3_INCLUDE_DIRS} ${GLFW_INCLUDES}  ${GLFW3_INCLUDES} ${GLFW_INCLUDEDIR} ${GLFW3_INCLUDEDIR} ${GLFW_INCLUDE_DIRECTORIES} ${GLFW3_INCLUDE_DIRECTORIES}")
    message(STATUS "glfw includes: ${glfw_INCLUDE_DIR} ${glfw3_INCLUDE_DIR} ${glfw_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS} ${glfw_INCLUDES} ${glfw3_INCLUDES} ${glfw_INCLUDEDIR} ${glfw3_INCLUDEDIR} ${glfw_INCLUDE_DIRECTORIES} ${glfw3_INCLUDE_DIRECTORIES}")
  endif()
endif()

# do SDL detection
# so SDL_FOUND can be set on a 2.0 framework... ugh...
if (NOT PREFER_SDL2)
  find_package(SDL 1 EXACT)
  if(SDL_FOUND)
  message(STATUS "SDL Version: ${SDL_VERSION_STRING}")
  message(STATUS "SDL Include: ${SDL_INCLUDE_DIR}")
  #message(STATUS "SDL Root: ${SDL_ROOT}")
  message(STATUS "SDL Lib: ${SDL_LIBRARY}")
  #message(STATUS "SDL Libs: ${SDL_LIBRARIES}")
else()
  find_package(SDL2)
  if(SDL2_FOUND)
    #message(STATUS "SDL2 Version: ${SDL2_VERSION_STRING}")
    #message(STATUS "SDL2 Include: ${SDL2_INCLUDE_DIR}")
    message(STATUS "SDL2 Includes: ${SDL2_INCLUDE_DIRS}")
    #message(STATUS "SDL2 cflags: ${SDL2_CFLAGS}")
    #message(STATUS "SDL2 cflags: ${SDL2_CFLAGS_OTHER}")
    #message(STATUS "SDL2 Root: ${SDL2_ROOT}")
    #message(STATUS "SDL2 Lib: ${SDL2_LIBRARY}")
    message(STATUS "SDL2 Libs: ${SDL2_LIBRARIES}")
  endif()
endif()
else()
  find_package(SDL2)
  if(SDL2_FOUND)
    #message(STATUS "SDL2 Version: ${SDL2_VERSION_STRING}")
    #message(STATUS "SDL2 Include: ${SDL2_INCLUDE_DIR}")
    message(STATUS "SDL2 Includes: ${SDL2_INCLUDE_DIRS}")
    #message(STATUS "SDL2 cflags: ${SDL2_CFLAGS}")
    #message(STATUS "SDL2 cflags: ${SDL2_CFLAGS_OTHER}")
    #message(STATUS "SDL2 Root: ${SDL2_ROOT}")
    #message(STATUS "SDL2 Lib: ${SDL2_LIBRARY}")
    message(STATUS "SDL2 Libs: ${SDL2_LIBRARIES}")
  else()
    find_package(SDL 1 EXACT)
    if(SDL_FOUND)
      message(STATUS "SDL Version: ${SDL_VERSION_STRING}")
      message(STATUS "SDL Include: ${SDL_INCLUDE_DIR}")
      #message(STATUS "SDL Root: ${SDL_ROOT}")
      message(STATUS "SDL Lib: ${SDL_LIBRARY}")
      #message(STATUS "SDL Libs: ${SDL_LIBRARIES}")
    endif()
  endif()
endif()

# source

set(OG_RENDERER_SRC
  renderer.c
  ../include/opengem/renderer/renderer.h
)
source_group("renderers" FILES
  renderer.c
  renderer.h
)
add_library(opengem_renderer ${OG_RENDERER_SRC})
add_library(opengem::renderer ALIAS opengem_renderer)

target_include_directories(opengem_renderer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include/opengem/renderer/)

use_c99()

if(glfw3_FOUND)
  include_directories("glfw/include" ${OG_GLFW_INCLUDE})
  # 10.6 doesn't see glfw only glfw3?
  # 10.14 needs this (glfw 3.2)
  if (NOT GLFW3_LIBRARY)
    target_link_libraries(opengem_renderer glfw)
  endif()
  # link_directories(${GLFW_LIBRARY_DIR})
  # glfw 3.1 on 10.6 needs CoreFoundation brought in
  if(APPLE)
    # may select iOS and other appleOSes
    # 3.1 needs functions in 10.8+ CoreGraphics ( -framework CoreGraphics isn't separated until later...)
    # FIND_LIBRARY(APP_SERVICES_LIBRARY ApplicationServices )
    target_link_libraries(opengem_renderer ${GLFW_LIBRARIES} ${GLFW3_LIBRARY} "-framework CoreFoundation -framework ApplicationServices -framework Appkit -framework IOKit -framework CoreVideo")
  else()
    target_link_libraries(opengem_renderer ${GLFW_LIBRARIES} ${GLFW3_LIBRARY})
  endif()
  target_sources(opengem_renderer PRIVATE
    glfw/glfw.c
    glfw/glfw.h
  )
  source_group("renderer_plugins" FILES
    glfw/glfw.c
    glfw/glfw.h
  )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGLFW3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLFW3")
endif()
if(SDL_FOUND)
  include_directories(${SDL_INCLUDE_DIR})
  target_link_libraries(opengem_renderer ${SDL_LIBRARY})
  target_sources(opengem_renderer PRIVATE
    sdl/sdl.c
    sdl/sdl.h
  )
  source_group("renderer_plugins" FILES
    sdl/sdl.c
    sdl/sdl.h
  )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDL")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL")
endif()
if(SDL2_FOUND)
  include_directories(${SDL2_INCLUDE_DIRS})
  target_link_libraries(opengem_renderer ${SDL2_LIBRARIES})
  target_sources(opengem_renderer PRIVATE
    sdl2/sdl2.c
    sdl2/sdl2.h
  )
  source_group("renderer_plugins" FILES
    sdl2/sdl2.c
    sdl2/sdl2.h
  )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDL2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL2")
endif()

if(Freetype_FOUND)
  target_include_directories(opengem_renderer PRIVATE ${FREETYPE_INCLUDE_DIRS})
  target_link_libraries(opengem_renderer ${FREETYPE_LIBRARIES})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAS_FT2")
endif()

if (OPENGL_INCLUDE_DIR)
  message(STATUS "OpenGL includes ${OPENGL_INCLUDE_DIR}")
  target_include_directories(opengem_renderer PRIVATE ${OPENGL_INCLUDE_DIR})
endif()
if (OPENGL_gl_LIBRARY_FOUND)
  target_link_libraries(opengem_renderer ${OPENGL_gl_LIBRARY})
endif()
if (OPENGL_INCLUDE_DIR_FOUND AND OPENGL_gl_LIBRARY_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAS_OGL")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_OGL")
endif()
if(OPENGL_FOUND)
  #message(STATUS "Should add ogl to mdrenderer linking stage")
  target_link_libraries(opengem_renderer ${OPENGL_gl_LIBRARY})
endif()

target_sources(opengem_renderer PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/truetype/truetype.c
  ${CMAKE_CURRENT_SOURCE_DIR}/truetype/truetype.h
)
source_group("fonts" FILES
  truetype/truetype.c
  truetype/truetype.h
)

