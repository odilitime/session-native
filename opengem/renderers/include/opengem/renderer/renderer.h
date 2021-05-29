#ifndef RENDERER_H
#define RENDERER_H

#include "src/include/opengem_datastructures.h"

#define OG_RENDERER_WINDOW_NODECORATION 1

#ifdef HAS_OGL
  #if __APPLE__ && __MACH__
    #include <OpenGL/gl.h>
  #else
    #include <GL/gl.h>
  #endif
#else
typedef float GLfloat;
typedef unsigned int GLuint;
#endif

#include <inttypes.h>
#include <stdbool.h>

// configure some coordinate types

#ifdef FLOATCOORDS
typedef double coordinates;
typedef double sizes;
#else
typedef int16_t coordinates; // -32k to 32k
typedef uint16_t sizes; // 0 to 32k
#endif

struct textureMap {
  float map[4];
};

// FIXME: a rect_print func pls
struct og_rect {
  coordinates x, y;
  sizes w, h;
};

/*
struct ll_sprites {
  struct sprite *cur;
  struct ll_sprites *next;
};
*/

struct sprite {
  // glfw
  GLuint number;
  GLfloat s0;
  GLfloat t0;
  GLfloat s1;
  GLfloat t1;
  // what is this used for? I don't think it is...
  struct og_rect last;
  // sdl1
  uint32_t color;
  const unsigned char *texture;
};

struct window; // fwd declr

// scroll is relative, so we need the sign atm
// can always have an internal number that we return...
typedef void(coord_func)(struct window *const, int16_t, int16_t, void *);
typedef void(click_func)(struct window *const, int, int, void *);
typedef void(keyb_func)(struct window *const, int, int, int, void *);
typedef void(paste_func)(struct window *const, const char *, void *);
// could make void * an app_window * here... but that has some interesting prereqs...
typedef void(generic_func)(struct window *const, void *);

// I think UI needs it's own event_tree and leave this one alone...
// and then you can use app_window instead of window there all you want..
/// common event hooks
struct event_tree {
  // onResize
  coord_func *onResize; // physical event (could be synethesized?)
  // onMouseDown / up / move /wheel
  click_func *onMouseDown; // physical event
  void *onMouseDownUser;
  click_func *onMouseUp; // physical event
  void *onMouseUpUser;
  generic_func *onFocus; // synthesized event
  void *onFocusUser;
  generic_func *onBlur; // synthesized event
  void *onBlurUser;
  coord_func *onMouseMove;  // physical event
  void *onMouseMoveUser;
  click_func *onMouseOver; // synthesized event
  click_func *onMouseOut; // synthesized event
  coord_func *onWheel;  // physical event
  void *onWheelUser;
  // k/b up/repeat/down/press
  keyb_func *onKeyUp;  // physical event
  void *onKeyUpUser;
  keyb_func *onKeyRepeat; // physical event (could be synethesized?)
  keyb_func *onKeyDown; // physical event
  void(*onKeyPress)(unsigned int);
  
  paste_func *onTextPaste; // physical event (could be synethesized?)
  void *onTextPasteUser; // don't need this
};

void event_tree_init(struct event_tree *this);

struct window; // fwd declr

typedef void(window_clear)(const struct window *const);
// make textures
typedef struct sprite* (window_createSprite)(const unsigned char *texture, uint16_t w, uint16_t h);
typedef struct sprite* (window_createTextSprite)(const struct window *pWin, const unsigned char *texture, uint16_t w, uint16_t h);
typedef struct sprite* (window_createSpriteFromColor)(uint32_t color);
// use textures
typedef void (window_drawSpriteBox)(const struct window *, const struct sprite *texture, const struct og_rect *position);
typedef void (window_drawSpriteText)(const struct window *, const struct sprite *texture, uint32_t color, const struct og_rect *position);

typedef void (window_changeCursor)(const struct window *, uint8_t);
typedef bool (window_move)(const struct window *, int16_t, int16_t);
typedef bool (window_getPos)(const struct window *, int *, int *);
typedef bool (window_setPos)(const struct window *, int, int);
typedef bool (window_minimize)(const struct window *);
typedef bool (window_maximize)(const struct window *);
typedef bool (window_restoreSize)(const struct window *);
typedef const char * (window_getClipboard_func)(const struct window *);
typedef void (window_setClipboard_func)(const struct window *, char *);

/// data structure of a window
struct window {
  window_clear *clear; // clear the screen
  window_clear *swap; // swap the double buffer

  window_createSprite          *createSprite;
  window_createTextSprite      *createTextSprite;
  window_createSpriteFromColor *createSpriteFromColor;
  window_drawSpriteBox         *drawSpriteBox;
  window_drawSpriteText        *drawSpriteText;
  window_changeCursor          *changeCursor;
  window_move                  *move;
  window_getPos                *getPos;
  window_setPos                *setPos;
  window_minimize              *minimize;
  window_maximize              *maximize;
  window_restoreSize           *restoreSize;
  window_getClipboard_func        *getClipboard;
  window_setClipboard_func        *setClipboard;

  //
  // properties
  //
  
  void *rawWindow;
  //App *app;
  uint16_t width;
  uint16_t height;
  double textScaleX;
  double textScaleY;
  double cursorX;
  double cursorY;
  uint16_t delayResize;
  // the root multiComponent will have this..
  //bool renderDirty;
  // *ui multicomponet and selectionList these makes renderers dependant on ui
  struct renderers *renderer;
  uint32_t maxTextureSize;
  struct event_tree event_handlers;
  uint32_t clearColor;
};

void window_init(struct window *const pWin);

struct renderers;

typedef bool (base_renderer_init)();
typedef struct window* (base_renderer_createWindow)(const char *title, const struct og_rect *position, unsigned int flags);
typedef bool (base_renderer_useWindow)(const struct window *);
// return bool is if there was any errors
typedef bool (base_renderer_eventsWait)(const struct renderers *, uint32_t);
typedef bool (base_renderer_shouldQuit)(const struct window *);
// this is microseconds
typedef uint64_t (base_renderer_getTime)();

// maybe convert to interface?
typedef struct renderers {
  base_renderer_init *init;
  base_renderer_createWindow *createWindow;
  base_renderer_useWindow *useWindow;
  base_renderer_useWindow *closeWindow;
  base_renderer_eventsWait *eventsWait;
  base_renderer_shouldQuit *shouldQuit;
  base_renderer_getTime *getTime;
  void *cursors[3]; // hand, arrow, ibeam
} BaseRenderer;

BaseRenderer *og_get_renderer();

#endif
