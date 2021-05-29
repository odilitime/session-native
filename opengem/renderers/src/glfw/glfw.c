#include <stdio.h>  // printf
#include <stdlib.h> // malloc
#include <math.h>   // for pow
#include <string.h> // for memcpy

// MacOS: deprecated but not removed
#define GL_SILENCE_DEPRECATION

#if (defined(GL_VERSION_1_4) || defined(GL_VERSION_1_5) || defined(GL_VERSION_2_0) || defined(GL_VERSION_2_1))
#define OLD_GL14_MIPMAP
#endif

// VULKAN is 3.2+
#ifdef HAS_VULKAN
  #ifdef WANT_VULKAN
    #define GLFW_INCLUDE_VULKAN
  #endif
#endif
#include "glfw.h"

// may need to be after opengl is included
#if __APPLE__ && __MACH__
#include <OpenGL/glext.h> // for mipmaps
//#include <OpenGL/glxext.h> // for mipmaps
#else
#define GL_GLEXT_PROTOTYPES // for ubuntu glGenerateMipmap warning
#include <GL/glext.h> // for mipmaps
//#include <glxext.h> // for mipmaps
#endif


#include "include/opengem/renderer/renderer.h"
//#include "../../../../parsers/truetype/truetype.h"

void checkGLState(const char *whereWhat) {
  GLenum glErr=glGetError();
  if(glErr != GL_NO_ERROR) {
    printf("[%s] - not ok: %d\n", whereWhat, glErr);
  }
}

void glfw_window_clear(const struct window *const pWin) {
  // FIXME: create set function
  float r = (pWin->clearColor >> 24) & 0xFF;
  float g = (pWin->clearColor >> 16) & 0xFF;
  float b = (pWin->clearColor >>  8) & 0xFF;
  float a = (pWin->clearColor >>  0) & 0xFF;
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT); checkGLState("glfw_window_clear");
}

void glfw_window_swap(const struct window *const pWin) {
  glfwSwapBuffers(pWin->rawWindow); checkGLState("glfw_window_swap");
}

struct sprite *glfw_window_createSprite(const unsigned char *texture, uint16_t p_w, uint16_t p_h) {
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  if (!spr) {
    printf("glfw_window_createSprite - Can't allocate sprite\n");
    return 0;
  }
  spr->color = 0;
  // set minimums
  sizes w = p_w;
  sizes h = p_h;
  if (!w) w = 1;
  if (!h) h = 1;

  // make sure texture size is a power of two
  uint32_t potw = pow(2, ceil(log(w) / log(2)));
  uint32_t poth = pow(2, ceil(log(h) / log(2)));
  printf("potting from [%d,%d] to [%d,%d]\n", w, h, potw, poth);
  
  //unsigned char *data = (unsigned char *)malloc(sizeof(unsigned char) * poth * potw * 4);
  unsigned char *data = (unsigned char *)calloc(1, sizeof(unsigned char) * poth * potw * 4);
  if (!data) {
    printf("glfw_window_createSprite - Can't allocate texture\n");
    free(spr);
    return 0;
  }
  
  // scale texture to pot
  if (p_w && p_h) {
    for (uint16_t cy = 0; cy < h; cy++) {
      //printf("copy [%d]bytes from [%d] to [%d]\n", w, texture + cy * w, data + cy * potw);
      memcpy(data + cy * potw, texture + cy * w, w);
    }
  } else {
    printf("glfw_window_createSprite - no source texture\n");
  }
  
  spr->s0 = 0.0f;
  spr->t0 = 0.0f;
  spr->s1 = 1.0f;
  spr->t1 = 1.0f;

  //spr->s0 = 0.0f;
  //spr->t0 = 0.0f;
  //spr->s1 = w / (double)potw;
  //spr->t1 = h / (double)poth;
  //spr->s1 = potw / (double)w;
  //spr->t1 = poth / (double)h;
  //printf("glfw_window_createSprite [%f, %f] - [%f, %f]\n", spr->s0, spr->t0, spr->s1, spr->t1);
  
  /*
  for (uint16_t py = 0; py < h; py++) {
    for (uint16_t px = 0; px < w; px++) {
      for (unsigned int i = 0; i < 4; i++) {
        size_t read = ((px * 4) + (py * 4 * w)) + i;
        unsigned char val = texture[read];
        size_t ypos = potw - 1 - py; // flip y
        size_t write = ((px * 4) + (ypos * 4 * potw)) + i;
        data[write] = val;
      }
    }
  }
  */
  
  // create
  glGenTextures(1, &spr->number); checkGLState("glfw_window_createSprite - genTexture");
  // select
  glBindTexture(GL_TEXTURE_2D, spr->number); checkGLState("glfw_window_createSprite - bindTexture");
#ifdef OLD_GL14_MIPMAP
  glTexParameteri(Target, GL_GENERATE_MIPMAP, GL_TRUE);
#endif
  // upload
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture); checkGLState("glfw_window_createSprite - texImage2D");
  // process
#ifndef OLD_GL14_MIPMAP
  glGenerateMipmap(GL_TEXTURE_2D); checkGLState("glfw_window_createSprite - generateMipmap");
#endif
  // unselect
  glBindTexture(GL_TEXTURE_2D, 0); checkGLState("glfw_window_createSprite - unbindTexture");
  free(data);
  return spr;
}

// passed width/height
struct sprite *glfw_window_createTextSprite(const struct window *pWin, const unsigned char *texture, const sizes p_w, const sizes p_h) {
  //printf("createTextSprite [%dx%d]\n", p_w, p_h);
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  if (!spr) {
    printf("glfw_window_createTextSprite - Can't allocate sprite\n");
    return 0;
  }
  spr->color = 0;
  // set minimums
  sizes w = p_w;
  sizes h = p_h;
  if (!w) w = 1;
  if (!h) h = 1;
  glGenTextures(1, &spr->number);
  //printf("Made texture[%d]\n", spr->number);

  // make sure texture size is a power of two
  uint32_t potw = pow(2, ceil(log(w) / log(2)));
  uint32_t poth = pow(2, ceil(log(h) / log(2)));
  //printf("text texture upload [%d,%d]~^2 [%d,%d]\n", w, h, potw, poth);
  
  unsigned char *data = calloc(1, poth * potw);
  if (!data) {
    printf("glfw_window_createTextSprite - Can't allocate texture\n");
    free(spr);
    return 0;
  }
  // if there's a source texture
  if (p_w && p_h) {
    // scale texture to pot
    for (uint16_t cy = 0; cy < h; cy++) {
      //printf("copy [%d]bytes from [%d] to [%d]\n", w, texture + cy * w, data + cy * potw);
      memcpy(data + cy * potw, texture + cy * w, w);
    }
  }
  // texture coords
  /*
  res->s1 = res->width / res->textureWidth;
  res->t1 = res->height / res->textureHeight;
  */
  spr->s0 = 0.0f;
  spr->t0 = 0.0f;
  spr->s1 = w / (double)potw;
  spr->t1 = h / (double)poth;
  //printf("[%f, %f] - [%f, %f]\n", spr->s0, spr->t0, spr->s1, spr->t1);
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  
  glBindTexture(GL_TEXTURE_2D, spr->number); checkGLState("glfw_window_createTextSprite - bindTexture");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef OLD_GL14_MIPMAP
  glTexParameteri(Target, GL_GENERATE_MIPMAP, GL_TRUE);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, potw, poth, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data); checkGLState("glfw_window_createTextSprite - glTexImage2D GL_LUMINANCE");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, potw, poth, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data); checkGLState("glfw_window_createTextSprite - glTexImage2D GL_ALPHA");
#ifndef OLD_GL14_MIPMAP
  glGenerateMipmap(GL_TEXTURE_2D); checkGLState("glfw_window_createTextSprite - glGenerateMipmap");
#endif
  glBindTexture(GL_TEXTURE_2D, 0); checkGLState("glfw_window_createTextSprite - unBindTex");

  // I flipped this maybe due to coords...
  /*
  handle->s0 = textureMap.map[0];
  handle->t0 = textureMap.map[1];
  handle->s1 = textureMap.map[2];
  handle->t1 = textureMap.map[3];
  */
  free(data);
  
  return spr;
}

struct sprite* glfw_window_createSpriteFromColor(const uint32_t color) {
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  if (!spr) {
    printf("glfw_window_createSpriteFromColor - Can't allocate sprite\n");
    return 0;
  }
  
  unsigned char texture[1][1][4];
  texture[0][0][0]=(color >> 24) & 0xFF;
  texture[0][0][1]=(color >> 16) & 0xFF;
  texture[0][0][2]=(color >>  8) & 0xFF;
  texture[0][0][3]=(color >>  0) & 0xFF;
  if (!texture[0][0][3]) {
    printf("alpha [%d]\n", texture[0][0][3]);
  }
  spr->color = color;

  spr->s0 = 0.0f;
  spr->t0 = 0.0f;
  spr->s1 = 1.0f; // 1px / 1px
  spr->t1 = 1.0f; // 1px / 1px

  glGenTextures(1, &spr->number);
  glBindTexture(GL_TEXTURE_2D, spr->number);
#ifdef OLD_GL14_MIPMAP
  glTexParameteri(Target, GL_GENERATE_MIPMAP, GL_TRUE);
#endif
  // I think texture could just be color
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
#ifndef OLD_GL14_MIPMAP
  glGenerateMipmap(GL_TEXTURE_2D);
#endif
  glBindTexture(GL_TEXTURE_2D, 0);
  checkGLState("glfw_window_createSpriteFromColor");
  return spr;
}

// would be nice if we could decouple the draws from vetrex changes
// vertex change forces a draw, so you can have a draw without change vertexes
// so we can check to see if cache is up todate, and if not reupload the next vertexes

// we can queue the writes and regroup and flush
// use timers if needed

// changing rect is costly...
// cache[textid][]=positions do we have this position? when do we expire?
// or can we shift/transform a VBO?

void glfw_window_drawSpriteBox(const struct window *pWin, const struct sprite *texture, const struct og_rect *position) {
  //printf("glfw_window_drawSpriteBox drawing box [%d,%d]-[%d,%d] using[%d]\n", position->x, position->y, position->w, position->h, texture->number);
  if (!texture) {
    printf("Called drawSpiteBox with no texture\n");
    return;
  }
  
  if (texture->color) {
    //printf("color optimized route? [%x]\n", texture->color);
    //glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    //glBindTexture(GL_TEXTURE_2D, texture->number);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const uint8_t *colors = (uint8_t *)&texture->color;
    glColor4ub(colors[3], colors[2], colors[1], colors[0]);
    glBegin(GL_QUADS);
    glVertex2i(position->x, position->y);
    glVertex2i(position->x, position->y+position->h);
    glVertex2i(position->x+position->w, position->y+position->h);
    glVertex2i(position->x+position->w, position->y);
    glEnd();
    // reset color
    glColor4ub(255, 255, 255, 255);
    //glBindTexture(GL_TEXTURE_2D, 0);
    //glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    return;
  }
  
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture->number);
  glBegin(GL_QUADS);
    glTexCoord2i(texture->s0, texture->t0); glVertex2i(position->x, position->y);
    glTexCoord2i(texture->s0, texture->t1); glVertex2i(position->x, position->y+position->h);
    glTexCoord2i(texture->s1, texture->t1); glVertex2i(position->x+position->w, position->y+position->h);
    glTexCoord2i(texture->s1, texture->t0); glVertex2i(position->x+position->w, position->y);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
  return;
  //GLuint vertexBufferObjectBox;
  //glGenBuffers(1, &vertexBufferObjectBox); checkGLState("glGenBuffers");
  /*
  float vertices[20] = {
    0.0f, 0.0f, 0.0f,    0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,    1.0f, 1.0f,
    0.0f, 0.0f, 0.0f,    1.0f, 0.0f,
    0.0f, 0.0f, 0.0f,    0.0f, 0.0f
  };
  */
  
  // select buffer
  //glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectBox);
  // write buffer
  //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  /*
  GLfloat vertices[20] = {
    0.0f, 0.0f, 0.0f,    0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,    1.0f, 1.0f,
    0.0f, 0.0f, 0.0f,    1.0f, 0.0f,
    0.0f, 0.0f, 0.0f,    0.0f, 0.0f
  };
  vertices[(0 * 5) + 0] = position->x;
  vertices[(0 * 5) + 1] = position->y + position->h;
  vertices[(1 * 5) + 0] = position->x + position->w;
  vertices[(1 * 5) + 1] = position->y + position->h;
  vertices[(2 * 5) + 0] = position->x + position->w;
  vertices[(2 * 5) + 1] = position->y;
  vertices[(3 * 5) + 0] = position->x;
  vertices[(3 * 5) + 1] = position->y;
  */
  
  /*
  GLfloat vertices[12] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
  };
  vertices[(0 * 3) + 0] = position->x;
  vertices[(0 * 3) + 1] = position->y + position->h;
  vertices[(1 * 3) + 0] = position->x + position->w;
  vertices[(1 * 3) + 1] = position->y + position->h;
  vertices[(2 * 3) + 0] = position->x + position->w;
  vertices[(2 * 3) + 1] = position->y;
  vertices[(3 * 3) + 0] = position->x;
  vertices[(3 * 3) + 1] = position->y;
  */
  
  GLfloat vertices[] = {0, 480, 0, // bottom left corner
    0,  0, 0, // top left corner
    640,  0, 0, // top right corner
    640, 480, 0}; // bottom right corner

  
  //glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectBox); checkGLState("glBindBuffer");
  //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); checkGLState("glBufferData");
  
  /*
  float textureTransformMatrix[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
  textureShader->bind();
  GLint transformLocation = textureShader->uniform("transform");
  glUniformMatrix4fv(transformLocation, 1, GL_FALSE, textureTransformMatrix); glUniformMatrix4fv");
  */
  
  // vao are 3.0+
  /*
  GLuint vao = 0;
  glGenVertexArrays(1, &vao); checkGLState("glGenVertexArrays");
  glBindVertexArray(vao); checkGLState("glBindVertexArray");
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectBox); checkGLState("glBindBuffer");
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); checkGLState("glVertexAttribPointer");
  */
  glColor3b(0, 0, 255);
  //glBindTexture(GL_TEXTURE_2D, texture->number); checkGLState("glBindTexture");
  GLubyte indices[] = {0,1,2, // first triangle (bottom left - top left - top right)
    0,2,3}; // second triangle (bottom left - top right - bottom right)
  glVertexPointer(3, GL_FLOAT, 0, vertices); checkGLState("glVertexPointer");
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices); checkGLState("glDrawElements");
  checkGLState("glfw_window_drawSpriteBox");
}

void glfw_window_drawSpriteText(const struct window *pWin, const struct sprite *texture, uint32_t color, const struct og_rect *position) {
  //printf("glfw_window_drawSpriteText drawing box [%d,%d]-[%d,%d] [%x] textNum[%d]\n", position->x, position->y, position->w, position->h, color, texture->number);
  const uint8_t *colors=(uint8_t *)&color;
  /*
  GLbyte channels[4];
  channels[0]=(color >> 24) & 0xFF;
  channels[1]=(color >> 16) & 0xFF;
  channels[2]=(color >>  8) & 0xFF;
  channels[3]=(color >>  0) & 0xFF;
  */
  //printf("color [%d,%d,%d]%d\n", colors[3], colors[2], colors[1], colors[0]);
  //printf("display texture [%f, %f] [%d]\n", texture->s1, texture->t1, texture->number);
  if (color == 0x000000ff) {
    //printf("GLFW Warning: drawing with black at [%d,%d]-[%d,%d]\n", position->x, position->y, position->w, position->h);
  }
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, texture->number);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4ub(colors[3], colors[2], colors[1], colors[0]);
  glBegin(GL_QUADS);
    glTexCoord2f(texture->s0, texture->t0); glVertex2i(position->x, position->y);
    glTexCoord2f(texture->s0, texture->t1); glVertex2i(position->x, position->y+position->h);
    glTexCoord2f(texture->s1, texture->t1); glVertex2i(position->x+position->w, position->y+position->h);
    glTexCoord2f(texture->s1, texture->t0); glVertex2i(position->x+position->w, position->y);
  glEnd();
  // reset color
  glColor4ub(255, 255, 255, 255);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  //glColor4ub(0, 0, 255, 0);
  checkGLState("glfw_window_drawSpriteText");
  return;
}

bool glfw_window_move(const struct window *pWin, int16_t x, int16_t y) {
  glfwSetWindowPos(pWin->rawWindow, x, y);
  return true;
}

bool glfw_window_getPos(const struct window *pWin, int *x, int *y) {
  // need glfw 2.0
  glfwGetWindowPos(pWin->rawWindow, x, y);
  return true;
}

bool glfw_window_setPos(const struct window *pWin, int x, int y) {
  return false;
}

bool glfw_window_minimize(const struct window *pWin) {
  glfwIconifyWindow(pWin->rawWindow);
  return true;
}

bool glfw_window_maximize(const struct window *pWin) {
  // needs 3.3
#ifdef GLFW33
  glfwSetWindowAttrib(pWin->rawWindow, GLFW_DECORATED, true);
#endif
#ifdef HAS_VULKAN
  // needs 3.2
  glfwMaximizeWindow(pWin->rawWindow);
#endif
#ifdef GLFW33
  // FIXME: reposition up higher and resize...
  glfwSetWindowAttrib(pWin->rawWindow, GLFW_DECORATED, false);
#endif
  return true;
}

bool glfw_window_restoreSize(const struct window *pWin) {
  glfwRestoreWindow(pWin->rawWindow);
  return true;
}

const char *glfw_window_getClipboard(const struct window *pWin) {
  return glfwGetClipboardString(pWin->rawWindow);
}

void glfw_window_setClipboard(const struct window *pWin, char *in) {
  glfwSetClipboardString(pWin->rawWindow, in);  
}

GLFWcursor* cursorHand;
GLFWcursor* cursorArrow;
GLFWcursor* cursorIbeam;

bool glfw_renderer_init() {
  if (!glfwInit())
  {
    printf("glfwInit failed\n");
    // Initialization failed
    return 0;
  }

#ifdef HAS_VULKAN
  if (glfwVulkanSupported())
  {
    printf("Vulkan compute support detected\n");
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }
#endif
  cursorHand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
  cursorArrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  cursorIbeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
  return true;
}

void glfw_changeCursor(const struct window *pWin, uint8_t type) {
  // 3.1 has glfwSetCursor
  // https://www.glfw.org/docs/3.1/input.html
  //printf("glfw_changeCursor[%d]\n", type);
  switch(type) {
    case 0:
      glfwSetCursor(pWin->rawWindow, cursorArrow);
      break;
    case 1:
      glfwSetCursor(pWin->rawWindow, cursorHand);
      break;
    case 2:
      glfwSetCursor(pWin->rawWindow, cursorIbeam);
      break;
  }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  struct window *pWindow = glfwGetWindowUserPointer(window);
  //printf("[%f x %f]\n", xoffset, yoffset);
  if (pWindow->event_handlers.onWheel) {
    pWindow->event_handlers.onWheel(pWindow, xoffset*10, yoffset*10, pWindow->event_handlers.onWheelUser);
  }
}

void resize_callback(GLFWwindow* window, int w, int h) {
  struct window *pWindow = glfwGetWindowUserPointer(window);
  int width, height;
  glfwGetFramebufferSize(pWindow->rawWindow, &width, &height);
  //printf("canvas[%d x %d] vs output[%d,%d]\n", w, h, width, height);
  glViewport(0, 0, width, height); checkGLState("glfw_resize_callback::glViewport");

  glMatrixMode(GL_PROJECTION); checkGLState("glfw_renderer_createWindow::glMatrixMode");
  glLoadIdentity(); checkGLState("glfw_renderer_createWindow::glLoadIdentity");
  glOrtho(0, w, h, 0, 0, 1); checkGLState("glfw_renderer_createWindow::glOrtho");
  glMatrixMode(GL_MODELVIEW); checkGLState("glfw_renderer_createWindow::glMatrixMode2");
  glLoadIdentity(); checkGLState("glfw_renderer_createWindow::glLoadIdentity2");

  // update internal reporting size
  pWindow->width  = w;
  pWindow->height = h;
  
  //pWindow->renderDirty = true;
  pWindow->delayResize = 1;
  /*
  if (pWindow->event_handlers.onResize) {
    pWindow->event_handlers.onResize(pWindow, w, h, 0);
  }
  */
}

// mousemove
void cursor_callback(GLFWwindow* window, double x, double y) {
  struct window *pWindow = glfwGetWindowUserPointer(window);
  // we need to store this for mouseDown
  pWindow->cursorX = x;
  pWindow->cursorY = y;
  if (pWindow->event_handlers.onMouseMove) {
    // check if we're over the window and discard off window events...
    //printf("cursor callback[%f,%f]\n", x, y);
    pWindow->event_handlers.onMouseMove(pWindow, (int16_t)x, (int16_t)y, pWindow->event_handlers.onMouseMoveUser);
  }
}

void mouseButton_callback(GLFWwindow* window, int button, int action, int mods) {
  struct window *pWindow = glfwGetWindowUserPointer(window);
  if (action) {
    if (pWindow->event_handlers.onMouseDown) {
      pWindow->event_handlers.onMouseDown(pWindow, button, mods, pWindow->event_handlers.onMouseDownUser);
    }
  } else {
    if (pWindow->event_handlers.onMouseUp) {
      pWindow->event_handlers.onMouseUp(pWindow, button, mods, pWindow->event_handlers.onMouseUpUser);
    }
  }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  struct window *pWindow = glfwGetWindowUserPointer(window);
  //printf("glfw key [%d,%d][%d]\n", key, scancode, action);
  // remap
  switch(key) {
    case GLFW_KEY_BACKSPACE: key = 8; break;
    case GLFW_KEY_ENTER:     key = 13; break;
  }
  if (mods & GLFW_MOD_SHIFT) {
    if (key >= '1' && key <= '5') {
      if (key == '2') key = 64;
      else
        key -= 16;
    } else
    if (key == '6') key = 94;
    else
    if (key == '7') key = 38;
    else
    if (key == '8') key = 42;
    else
    if (key == '9') key = 40;
    else
    if (key == '0') key = 41;
    else
    if (key == '/') key = 63; // map to ?
    else
    if (key == ';') key = 58; // map ; to :
  } else {
    // lower case it by adding 32
    if (key >= 'A' && key <= 'Z') {
      key += 'a' - 'A';
    }
  }
  switch(key) {
    // hacks for dos like arrow keys
    case GLFW_KEY_UP:
      key = 224 + 72;
      break;
    case GLFW_KEY_DOWN:
      key = 224 + 80;
      break;
    case GLFW_KEY_LEFT:
      key = 224 + 75;
      break;
    case GLFW_KEY_RIGHT:
      key = 224 + 77;
      break;
    case GLFW_KEY_TAB:
      key = 9;
      break;
  }
  if (action == GLFW_RELEASE) {
    if (pWindow->event_handlers.onKeyUp) {
      pWindow->event_handlers.onKeyUp(pWindow, key, scancode, mods, pWindow->event_handlers.onKeyUpUser);
    }
  } else if (action == GLFW_REPEAT) {
    if (pWindow->event_handlers.onKeyRepeat) {
      pWindow->event_handlers.onKeyRepeat(pWindow, key, scancode, mods, 0);
    } else {
      // release the press
      if (pWindow->event_handlers.onKeyUp) {
        pWindow->event_handlers.onKeyUp(pWindow, key, scancode, mods, pWindow->event_handlers.onKeyUpUser);
      }
      // press it again
      if (pWindow->event_handlers.onKeyDown) {
        pWindow->event_handlers.onKeyDown(pWindow, key, scancode, mods, 0);
      }
    }
  } else {
    if (pWindow->event_handlers.onKeyDown) {
      pWindow->event_handlers.onKeyDown(pWindow, key, scancode, mods, 0);
    }
  }
}

struct window* glfw_renderer_createWindow(const char *title, const struct og_rect *position, unsigned int flags) {
  if (flags & OG_RENDERER_WINDOW_NODECORATION) {
    glfwWindowHint(GLFW_DECORATED, false); // hide titlebar
  }
#ifdef GLFW_COCOA_GRAPHICS_SWITCHING
  printf("Supports GLFW_COCOA_GRAPHICS_SWITCHING\n");
  glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_TRUE);
#endif
#ifdef GLFW_COCOA_RETINA_FRAMEBUFFER
  printf("Supports GLFW_COCOA_RETINA_FRAMEBUFFER\n");
#endif
  GLFWwindow* tWindow = glfwCreateWindow(position->w, position->h, title, NULL, NULL);
  if (!tWindow)
  {
    // Window or OpenGL context creation failed
    printf("window or ogl failed\n");
    return 0;
  }
  
  int mj, mn, rv;
  glfwGetVersion(&mj, &mn, &rv);
  printf("GLFW version %d.%d.%d [%s]\n", mj, mn, rv, glfwGetVersionString());
  
  glfwMakeContextCurrent(tWindow);
  int width, height;
  glfwGetFramebufferSize(tWindow, &width, &height);
#ifdef FLOATCOORDS
  printf("Asked for [%fx%f] got [%dx%d]\n", position->w, position->h, width, height);
#else
  printf("Asked for [%dx%d] got [%dx%d]\n", position->w, position->h, width, height);
#endif
  checkGLState("glfw_renderer_createWindow");
  /*
  glViewport(0, 0, width, height); checkGLState("glfw_renderer_createWindow::glViewport");
  glMatrixMode(GL_PROJECTION); checkGLState("glfw_renderer_createWindow::glMatrixMode");
  glLoadIdentity(); checkGLState("glfw_renderer_createWindow::glLoadIdentity");
  glOrtho(0, position->w, position->h, 0, 0, 1); checkGLState("glfw_renderer_createWindow::glOrtho");
  glMatrixMode(GL_MODELVIEW); checkGLState("glfw_renderer_createWindow::glMatrixMode2");
  glLoadIdentity(); checkGLState("glfw_renderer_createWindow::glLoadIdentity2");
  */

  const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString(GL_VERSION); // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  glfwSwapInterval(1);     // Lock to vertical sync of monitor (normally 60Hz, so 60fps)
  
  struct window *pWindow = (struct window *)malloc(sizeof(struct window));
  if (!pWindow) {
    printf("glfw_renderer_createWindow - Can't allocate window\n");
    return 0;
  }
  window_init(pWindow);
  pWindow->rawWindow = tWindow;
  pWindow->width     = position->w;
  pWindow->height    = position->h;
  pWindow->clear     = glfw_window_clear;
  pWindow->swap      = glfw_window_swap;
  pWindow->createSprite          = glfw_window_createSprite;
  pWindow->createTextSprite      = glfw_window_createTextSprite;
  pWindow->createSpriteFromColor = glfw_window_createSpriteFromColor;
  pWindow->drawSpriteBox         = glfw_window_drawSpriteBox;
  pWindow->drawSpriteText        = glfw_window_drawSpriteText;
  pWindow->changeCursor          = glfw_changeCursor;
  pWindow->move                  = glfw_window_move;
  pWindow->getPos                = glfw_window_getPos;
  pWindow->setPos                = glfw_window_setPos;
  pWindow->minimize              = glfw_window_minimize;
  pWindow->maximize              = glfw_window_maximize;
  pWindow->restoreSize           = glfw_window_restoreSize;
  pWindow->getClipboard          = glfw_window_getClipboard;
  pWindow->setClipboard          = glfw_window_setClipboard;
  
  glfwSetWindowUserPointer(tWindow, pWindow);

  // finish setting up ogl params
  resize_callback(tWindow, position->w, position->h);
  
  // set up all event tree
  pWindow->event_handlers.onWheel = 0; // init onWheel
  glfwSetScrollCallback(tWindow, scroll_callback);
  glfwSetWindowSizeCallback(tWindow, resize_callback);
  //glfwSetFramebufferSizeCallback(tWindow, ); // just update glViewport
  glfwSetCursorPosCallback(tWindow, cursor_callback);
  glfwSetMouseButtonCallback(tWindow, mouseButton_callback);
  //glfwSetCharCallback
  glfwSetKeyCallback(tWindow, key_callback);
  
  return pWindow;
}

bool glfw_renderer_useWindow(const struct window *pWin) {
  glfwMakeContextCurrent(pWin->rawWindow);
  return true;
}

bool glfw_renderer_closeWindow(const struct window *pWin) {
  glfwDestroyWindow(pWin->rawWindow);
  return true;
}

#ifndef HAS_VULKAN
  bool eventWaitWarningShown = false;
#endif

bool glfw_renderer_eventsWait(const struct renderers *this, uint32_t wait) {
  if (wait) {
#ifdef HAS_VULKAN
    glfwWaitEventsTimeout(wait / 1000.0);
#else
    if (!eventWaitWarningShown) {
      printf("glfwWaitEventsTimeout not supported with your GLFW, consider upgrading our GLFW\n");
      eventWaitWarningShown = true;
    }
    // we could use a thread/timer to schedule a callback or abort this maybe?
    glfwWaitEvents();
#endif
  } else {
    glfwWaitEvents();
  }
  return true;
}

bool glfw_renderer_shouldQuit(const struct window *pWin) {
  return glfwWindowShouldClose(pWin->rawWindow);
}

uint64_t glfw_renderer_getTime() {
  // time since initialized (returns double)
  // in seconds
  // so we're returning MS precision
  return round(glfwGetTime() * 1000);
}

BaseRenderer renderer_glfw = {
  glfw_renderer_init,
  glfw_renderer_createWindow,
  glfw_renderer_useWindow,
  glfw_renderer_closeWindow,
  glfw_renderer_eventsWait,
  glfw_renderer_shouldQuit,
  glfw_renderer_getTime
};

