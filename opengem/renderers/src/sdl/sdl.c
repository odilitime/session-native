#include "sdl.h"
#include "include/opengem/renderer/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include "src/include/opengem_datastructures.h"

void sdl_window_clear(const struct window *const pWin) {
  //glClear(GL_COLOR_BUFFER_BIT);
  SDL_Rect dstrect;
  dstrect.x = 0;
  dstrect.y = 0;
  dstrect.w = pWin->width;
  dstrect.h = pWin->height;
  SDL_Surface *screen = (SDL_Surface *)pWin->rawWindow;
  Uint32 black = SDL_MapRGB(screen->format, 0, 0, 0); // black
  SDL_FillRect(pWin->rawWindow, &dstrect, black);
}

void sdl_window_swap(const struct window *const pWin) {
  //SDL_UpdateRect(pWin->rawWindow, 0, 0, pWin->width, pWin->height);
  SDL_Flip(pWin->rawWindow);
}

struct sprite* sdl_window_createSprite(const unsigned char *texture, uint16_t w, uint16_t h) {
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  spr->number = 0;
  spr->color = 0;
  return spr;
}

struct sprite* sdl_window_createSpriteFromColor(const uint32_t color) {
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  spr->number = 0;
  spr->color = color;
  return spr;
}

uint32_t sdlTextNum = 0;

struct sprite *sdl_window_createTextSprite(const struct window *pWin, const unsigned char *texture, const uint16_t w, const uint16_t h) {
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  sdlTextNum++;
  spr->number  = sdlTextNum;
  spr->texture = texture;
  spr->s0 = (GLfloat)w;
  spr->t0 = (GLfloat)h;
  spr->color = 1;
  // conver to a surface
  /*
  SDL_Surface *text2 = SDL_CreateRGBSurfaceFrom(texture, w, h, 8, w, 0x0000ffff, 0x0000ffff, 0x0000ffff);
  //text2->flags &= ~SDL_PREALLOC;
  //text2->flag |= SDL_SIMD_ALIGNED:
  SDL_Palette *palette = text2->format->palette;
  // create background color
  palette->colors[0].r = 0;
  palette->colors[0].g = 0;
  palette->colors[0].b = 0;
  SDL_SetColorKey(text2, SDL_TRUE, 0);
   */
  /*
  SDL_Surface *text = SDL_CreateRGBSurface(texture, w, h, 8, w, 0x0000ffff, 0x0000ffff, 0x0000ffff);
  SDL_SetPalette(text, 0, 0, 0, 0);
  //SDL_SetColorKey(text, SDL_SRCCOLORKEY, 0);
  //SDL_Surface *copy = SDL_DisplayFormatAlpha(text);
  //SDL_BlitSurface(copy, &srcrect, screen, &dstrect);
  //SDL_Color *palette;
  for(uint16_t y = 0; y < srcrect.h; ++y) {
    for(uint16_t x = 0; x < srcrect.w; ++x) {
      uint32_t pos = x + y * srcrect.w;
      const unsigned char *ref = texture + pos;
      unsigned char chr = *ref;
      if (chr) {
        Uint32 *pixels = text->pixels;
        int offset = (text->pitch / sizeof(Uint32)) * y + x;
        *(pixels + offset) = SDL_MapRGB(const SDL_PixelFormat *const format, const Uint8 r, const Uint8 g, const Uint8 b)
      }
    }
  }
  */
  return spr;
}

void sdl_window_drawSpriteBox(const struct window *pWin, const struct sprite *texture, const struct og_rect *position) {
  SDL_Rect dstrect;
  dstrect.x = position->x;
  dstrect.y = position->y;
  dstrect.w = position->w;
  dstrect.h = position->h;
  SDL_Surface *screen = (SDL_Surface *)pWin->rawWindow;

  if (texture->number == 0) {

    unsigned char channels[4];
    channels[0]=(texture->color >> 24) & 0xFF;
    channels[1]=(texture->color >> 16) & 0xFF;
    channels[2]=(texture->color >>  8) & 0xFF;
    channels[3]=(texture->color >>  0) & 0xFF;

    SDL_FillRect((SDL_Surface *)pWin->rawWindow, &dstrect, SDL_MapRGB(screen->format, channels[0], channels[1], channels[2]));
  } else {
    printf("You shouldn't be rendering text with drawSpriteBox\n");
    /*
    // text texture
    SDL_Rect srcrect;
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.w = (uint16_t)texture->s0;
    srcrect.h = (uint16_t)texture->t1;
    SDL_Surface *text = SDL_CreateRGBSurface(0, texture->s0, texture->t1, 1, texture->s0, 0x0000ff, 0x0000ff, 0x0000ff);
    SDL_SetColorKey(text, SDL_SRCCOLORKEY, 0);
    SDL_Surface *copy = SDL_DisplayFormatAlpha(text);
    printf("wew\n");
    SDL_BlitSurface(copy, &srcrect, screen, &dstrect);
    */
  }
}

struct textSpiteCacheEntry {
  SDL_Surface *sprite;
  uint32_t spriteNum;
  uint32_t color;
};
struct dynList textSprites;

void *texture_search_callback(struct dynListItem *const item, void *user) {
  struct textSpiteCacheEntry *search = user;
  if (!search->sprite) {
    struct textSpiteCacheEntry *entry = item->value;
    if (entry->spriteNum == search->spriteNum && entry->color == search->color) {
      search->sprite = entry->sprite;
    }
  }
  return user;
}

void sdl_window_drawSpriteText(const struct window *pWin, const struct sprite *texture, const uint32_t color, const struct og_rect *position) {
  SDL_Rect dstrect;
  /*
  dstrect.x = position->x;
  dstrect.y = position->y;
  dstrect.w = position->w;
  dstrect.h = position->h;
  */
  SDL_Surface *screen = (SDL_Surface *)pWin->rawWindow;
  
  if (texture->number) {
    
    SDL_Rect srcrect;
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.w = (uint16_t)texture->s0;
    srcrect.h = (uint16_t)texture->t0;
    
    dstrect.x = position->x;
    dstrect.y = position->y;
    dstrect.w = srcrect.w;
    dstrect.h = srcrect.h;
    if (dstrect.x + dstrect.w > screen->w) {
      dstrect.w = screen->w - dstrect.x;
    }
    if (dstrect.y + dstrect.h > screen->h) {
      dstrect.h = screen->h - dstrect.y;
    }

    struct textSpiteCacheEntry search;
    search.color = color;
    search.spriteNum = texture->number; // texture number
    search.sprite = 0;
    
    dynList_iterator(&textSprites, texture_search_callback, &search);
    if (search.sprite) {
      //printf("Using cache\n");
      SDL_BlitSurface(search.sprite, &srcrect, screen, &dstrect);
      return;
    }

    unsigned char channels[4];
    channels[0]=(color >> 24) & 0xFF;
    channels[1]=(color >> 16) & 0xFF;
    channels[2]=(color >>  8) & 0xFF;
    channels[3]=(color >>  0) & 0xFF;
    
    printf("pos0 [%d]\n", *texture->texture);
    
    SDL_Surface *text = SDL_CreateRGBSurfaceFrom((void *)texture->texture, texture->s0, texture->t0, 8, texture->s0, 0x0000ffff, 0x0000ffff, 0x0000ffff, 0);
    //text2->flags &= ~SDL_PREALLOC;
    //text2->flag |= SDL_SIMD_ALIGNED:
    SDL_Palette *palette = text->format->palette;
    // create background color
    palette->colors[0].r = 0;
    palette->colors[0].g = 0;
    palette->colors[0].b = 0;
    // set SDL_RLEACCEL & SDL_SRCCOLORKEY = SDL_TRUE
    // SDL_MapRGB( text->format, 0, 0, 0 )
    SDL_SetColorKey(text, SDL_TRUE, 0);
    //SDL_SetAlpha(text, 0, 128);
    // try to optimize for display
    //SDL_Surface *copy = SDL_DisplayFormatAlpha(text);
    SDL_Surface *copy = text;
    

    SDL_BlitSurface(copy, &srcrect, screen, &dstrect);
    
    printf("Building cache item\n");
    
    // add it to our cache
    struct textSpiteCacheEntry *cache = malloc(sizeof(struct textSpiteCacheEntry));
    cache->color = color;
    cache->spriteNum = texture->number;
    cache->sprite = copy;
    dynList_push(&textSprites, cache);
    
    /*
    dstrect.w = 1;
    dstrect.h = 1;
    //printf("[%dx%d]\n", srcrect.w, srcrect.h);
    for(uint16_t y = 0; y < srcrect.h; ++y) {
      for(uint16_t x = 0; x < srcrect.w; ++x) {
        uint32_t pos = x + y * srcrect.w;
        const unsigned char *ref = texture->texture + pos;
        unsigned char chr = *ref;
        if (chr) {
          dstrect.x = position->x + x;
          dstrect.y = position->y + y;
          if (dstrect.y < 0) continue;
          int res = SDL_FillRect((SDL_Surface *)pWin->rawWindow, &dstrect, SDL_MapRGB(screen->format, channels[0], channels[1], channels[2]));
          if (res == -1) {
            printf("couldn't draw[%d, %d]\n", dstrect.x, dstrect.y);
          }
        }
      }
    }
     */
    /*
    SDL_Surface *text = SDL_CreateRGBSurface(texture->texture, srcrect.w, srcrect.h, 8, texture->s0, 0x0000ffff, 0x0000ffff, 0x0000ffff);
    SDL_SetColorKey(text, SDL_SRCCOLORKEY, 0);
    SDL_Surface *copy = SDL_DisplayFormatAlpha(text);
    SDL_BlitSurface(copy, &srcrect, screen, &dstrect);
    */
  } else {
    printf("Unknown sprite type[%d]\n", texture->number);
  }
}

bool sdl_renderer_init() {
  dynList_init(&textSprites, sizeof(struct textSpiteCacheEntry), "textSprites");
  // JOYSTICK causes issue on sun
  // SDL_INIT_EVENTTHREAD ?
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)) {
    printf("sdl init failed\n");
    // Initialization failed
    return false;
  }
  SDL_Init(SDL_INIT_JOYSTICK); // who cares if it fails
  return true;
}

void sdl_changeCursor(const struct window *pWin, uint8_t type) {
  switch(type) {
    case 0:
      //glfwSetCursor(pWin->rawWindow, cursorArrow);
      break;
    case 1:
      //glfwSetCursor(pWin->rawWindow, cursorHand);
      break;
    case 2:
      // 𝙸 or
      //glfwSetCursor(pWin->rawWindow, cursorIbeam);
      break;
  }
}

// SDL1 hack
struct window *g_window;

// FIXME: we can only have one window in SDL1
struct window* sdl_renderer_createWindow(const char *title, const struct og_rect *position, const unsigned int flags) {
  SDL_Surface *screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE|SDL_RESIZABLE|SDL_DOUBLEBUF);
  if ( screen == NULL ) {
    fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n",
            SDL_GetError());
    exit(1);
  }
  // FIXME: 2nd param is supposed to be an icon
  SDL_WM_SetCaption(title, title);
  struct window *pWindow = (struct window *)malloc(sizeof(struct window));
  window_init(pWindow);
  pWindow->rawWindow = screen;
  pWindow->width = position->w;
  pWindow->height = position->h;
  
  pWindow->clear = sdl_window_clear;
  pWindow->swap  = sdl_window_swap;
  pWindow->createSprite          = sdl_window_createSprite;
  pWindow->createSpriteFromColor = sdl_window_createSpriteFromColor;
  pWindow->createTextSprite      = sdl_window_createTextSprite;
  pWindow->drawSpriteBox         = sdl_window_drawSpriteBox;
  pWindow->drawSpriteText        = sdl_window_drawSpriteText;
  pWindow->changeCursor          = sdl_changeCursor;
  g_window = pWindow;
  return pWindow;
}

bool sdl_renderer_useWindow(const struct window *pWin) {
  return true;
}

bool sdl_renderer_closeWindow(const struct window *pWin) {
  return true;
}

bool shouldQuit = false;
void handle_event(const struct renderers *this, SDL_Event *event) {
  //printf("event type[%d]\n", event->type);
  switch(event->type) {
    case SDL_QUIT:
      shouldQuit = true;
      break;
    case SDL_ACTIVEEVENT:
      // window selected
      break;
    case SDL_KEYUP:
      if (g_window->event_handlers.onKeyUp) {
        int key = event->key.keysym.sym;
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
        
        // uppercase case it by subtracting 32
        if (key >= 'a' && key <= 'z') {
          // printf("uppercasing\n");
          key -= 'a' - 'A';
        }
        event->key.keysym.sym = key;

        // type is up or down
        g_window->event_handlers.onKeyUp(g_window, event->key.keysym.sym, event->key.keysym.scancode, event->key.keysym.mod, g_window->event_handlers.onKeyUpUser);
      }
      break;
    case SDL_KEYDOWN:
      if (g_window->event_handlers.onKeyDown) {
        g_window->event_handlers.onKeyDown(g_window, event->key.keysym.sym, event->key.keysym.scancode,  event->key.keysym.mod, 0);
      }
      break;
    case SDL_MOUSEMOTION:
      // we need to store this for mouseDown
      g_window->cursorX = event->motion.x;
      g_window->cursorY = event->motion.y;
      if (g_window->event_handlers.onMouseMove) {
        g_window->event_handlers.onMouseMove(g_window, event->motion.x, event->motion.y, g_window->event_handlers.onMouseMoveUser);
      }
      break;
    case SDL_MOUSEBUTTONUP:
      //printf("mouse button up\n");
      switch(event->button.button) {
        case SDL_BUTTON_LEFT:
          if (g_window->event_handlers.onMouseUp) {
            g_window->event_handlers.onMouseUp(g_window, 0, -10, g_window->event_handlers.onMouseUpUser);
          }
          break;
        case SDL_BUTTON_WHEELDOWN:
          // how do we tell which window we send this too
          if (g_window->event_handlers.onWheel) {
            g_window->event_handlers.onWheel(g_window, 0, -10, 0);
          }
          break;
        case SDL_BUTTON_WHEELUP:
          if (g_window->event_handlers.onWheel) {
            g_window->event_handlers.onWheel(g_window, 0, 10, 0);
          }
          break;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      //printf("mouse button down\n");
      switch(event->button.button) {
        case SDL_BUTTON_LEFT:
          if (g_window->event_handlers.onMouseDown) {
            g_window->event_handlers.onMouseDown(g_window, 0, -10, g_window->event_handlers.onMouseDownUser);
          }
          break;
        case SDL_BUTTON_WHEELDOWN:
          break;
        case SDL_BUTTON_WHEELUP:
          break;
      }
      break;
  }
}

bool sdl_renderer_eventsWait(const struct renderers *this, uint32_t wait) {
  SDL_Event event;
  uint32_t exp;
  if (wait) {
    exp = SDL_GetTicks() + wait;
  }
  while(1) {
    SDL_PumpEvents();
    int eventCount = SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_ALLEVENTS);
    switch (eventCount) {
      case -1:
        // error
        return false;
      case 0:
        // no events
        if (wait == 0 || (wait > 0 && SDL_GetTicks() >= exp))
          return true;
        else
          SDL_Delay(10);
      default:
        // one event or more
        if (eventCount > 1) printf("found [%d] events\n", eventCount);
        for(int i = 0; i < eventCount; i++) {
          SDL_Event *curEvt = &event;
          curEvt += i;
          handle_event(this, curEvt);
        }
        return true;
    }
  }
  return true;
}

bool sdl_renderer_shouldQuit(const struct window *pWin) {
  return shouldQuit;
}

uint64_t sdl_renderer_getTime() {
  return SDL_GetTicks();
}

BaseRenderer renderer_sdl = {
  sdl_renderer_init,
  sdl_renderer_createWindow,
  sdl_renderer_useWindow,
  sdl_renderer_closeWindow,
  sdl_renderer_eventsWait,
  sdl_renderer_shouldQuit,
  sdl_renderer_getTime
};

