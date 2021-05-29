#include <stdio.h>
#include "include/opengem/renderer/renderer.h"
#include <stdlib.h>

// Hook in graphical renderes

// has to be loaded before GLFW3
#if (GLFW3 || SDL || SDL2)
//#include <GL/glew.h>
#endif

#include "renderer.h"
#ifdef GLFW3
  #include "glfw/glfw.h"
#endif
#ifdef SDL
  #include "sdl/sdl.h"
#endif
#ifdef SDL2
  #include "sdl2/sdl2.h"
#endif

BaseRenderer *og_get_renderer() {
#ifdef GLFW3
  printf("compiled with GLFW\n");
#endif
#ifdef SDL
  printf("compiled with SDL1\n");
#endif
#ifdef SDL2
  printf("compiled with SDL2\n");
#endif
  
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef GLFW3
  printf("Using GLFW3\n");
  // is there a better way to do this?
  extern BaseRenderer renderer_glfw;
  renderer_glfw.init();
  return &renderer_glfw;
#elif (SDL)
  printf("Using SDL1\n");
  extern BaseRenderer renderer_sdl;
  renderer_sdl.init();
  return &renderer_sdl;
#elif (SDL2)
  printf("Using SDL2\n");
  extern BaseRenderer renderer_sdl2;
  renderer_sdl2.init();
  return &renderer_sdl2;
#else
  return 0;
#endif
}

void window_init(struct window *const this) {
  this->delayResize = 0;
  this->textScaleX = 1.0;
  this->textScaleY = 1.0;
  // configure event tree
  event_tree_init(&this->event_handlers);
}

void event_tree_init(struct event_tree *this) {
  this->onResize = 0;
  this->onMouseDown = 0;
  this->onMouseDownUser = 0;
  this->onMouseUp = 0;
  this->onMouseUpUser = 0;
  this->onMouseMove = 0;
  this->onMouseMoveUser = 0;
  this->onMouseOver = 0;
  this->onMouseOut = 0;
  this->onWheel = 0;
  this->onWheelUser = 0;
  this->onKeyUp = 0;
  this->onKeyUpUser = 0;
  this->onKeyRepeat = 0;
  this->onKeyDown = 0;
  this->onKeyPress = 0;
  this->onTextPaste = 0;
  this->onTextPasteUser = 0;
  this->onFocus = 0;
  this->onFocusUser = 0;
  this->onBlur = 0;
  this->onFocusUser = 0;
}
