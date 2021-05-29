#include "sdl2.h"
#include "include/opengem/renderer/renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include "src/include/opengem_datastructures.h"

void sdl2_window_clear(const struct window *const pWin) {
  SDL_Renderer *ren = (SDL_Renderer *)pWin->rawWindow;
  SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
  SDL_RenderClear(ren);
}

void sdl2_window_swap(const struct window *const pWin) {
  SDL_Renderer *ren = (SDL_Renderer *)pWin->rawWindow;
  SDL_RenderPresent(ren);
}

struct sprite* sdl2_window_createSprite(const unsigned char *texture, uint16_t w, uint16_t h) {
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  spr->number = 0;
  spr->color = 0;
  return spr;
}

struct sprite *sdl2_window_createSpriteFromColor(const uint32_t color) {
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  spr->number = 0;
  spr->color  = color;
  return spr;
}

uint32_t sdlTextNum = 0;

struct sprite *sdl2_window_createTextSprite(const struct window *pWin, const unsigned char *texture, const uint16_t p_w, const uint16_t p_h) {
  struct sprite *spr = (struct sprite *)malloc(sizeof(struct sprite));
  sdlTextNum++;
  // set minimums
  sizes w = p_w;
  sizes h = p_h;
  if (!w) w = 1;
  if (!h) h = 1;
  spr->number  = sdlTextNum;
  spr->texture = texture;
  spr->s0 = (GLfloat)w;
  spr->t0 = (GLfloat)h;
  return spr;
}

void sdl2_window_drawSpriteBox(const struct window *pWin, const struct sprite *texture, const struct og_rect *position) {
  if (texture->number == 0) {
    SDL_Rect dstrect;
    dstrect.x = position->x;
    dstrect.y = position->y;
    dstrect.w = position->w;
    dstrect.h = position->h;
    
    unsigned char channels[4];
    channels[0]=(texture->color >> 24) & 0xFF;
    channels[1]=(texture->color >> 16) & 0xFF;
    channels[2]=(texture->color >>  8) & 0xFF;
    channels[3]=(texture->color >>  0) & 0xFF;
    SDL_SetRenderDrawColor( pWin->rawWindow, channels[0], channels[1], channels[2], channels[3]);
    //printf("sdl2_window_drawSpriteBox drawing box [%d,%d]-[%d,%d]\n", dstrect.x, dstrect.y, dstrect.w, dstrect.h);
    SDL_RenderFillRect(pWin->rawWindow, &dstrect);
  } else {
    printf("Unknown sprite type[%d]\n", texture->number);
  }
}

struct textSpiteCacheEntry {
  SDL_Texture *sprite;
  uint32_t spriteNum;
  uint32_t color;
};
struct dynList textSprites;

void *texture_search_callback(struct dynListItem *const item, void *user) {
  struct textSpiteCacheEntry *search = user;
  if (!search->sprite) {
    struct textSpiteCacheEntry *entry = item->value;
    //printf("[%d_%d] vs [%d_%d]\n", entry->spriteNum, entry->color, search->spriteNum, search->color);
    if (entry->spriteNum == search->spriteNum && entry->color == search->color) {
      //printf("Found, passing back [%x]\n", entry->sprite);
      search->sprite = entry->sprite;
    }
  }
  return user;
}

void sdl2_window_drawSpriteText(const struct window *pWin, const struct sprite *texture, uint32_t color, const struct og_rect *position) {
  if (!texture->number) {
    return;
  }
  
  SDL_Renderer *ren = (SDL_Renderer *)pWin->rawWindow;
  //SDL_Surface *screen;

  SDL_Rect dstrect;
  dstrect.x = position->x;
  dstrect.y = position->y;
  dstrect.w = position->w;
  dstrect.h = position->h;
  //printf("sdl2_window_drawSpriteText text [%d,%d]-[%d,%d]\n", dstrect.x, dstrect.y, dstrect.w, dstrect.h);
  
  SDL_Rect srcrect;
  srcrect.x = 0;
  srcrect.y = 0;
  srcrect.w = texture->s0;
  srcrect.h = texture->t0;
  /*
  if (srcrect.w > pWin->width) {
    srcrect.w = pWin->width;
  }
  if (srcrect.h > pWin->height) {
    srcrect.h = pWin->height;
  }
  //printf("src sz[%d, %d]\n", srcrect.w, srcrect.h);
   */
  
  dstrect.w = srcrect.w;
  dstrect.h = srcrect.h;
  if (dstrect.x + dstrect.w > pWin->width) {
    dstrect.w = pWin->width - dstrect.x;
  }
  //printf("sdl2_window_drawSpriteText srcrect [%d,%d]-[%d,%d]\n", srcrect.x, srcrect.y, srcrect.w, srcrect.h);
  //printf("sdl2_window_drawSpriteText dstrect [%d,%d]-[%d,%d]\n", dstrect.x, dstrect.y, dstrect.w, dstrect.h);
  /*
  if (dstrect.y + dstrect.h > pWin->height) {
    dstrect.h = pWin->height - dstrect.y;
  }
  */

  struct textSpiteCacheEntry search;
  search.color = color;
  search.spriteNum = texture->number; // texture number
  search.sprite = 0;
  
  dynList_iterator(&textSprites, texture_search_callback, &search);
  if (search.sprite) {
    // printf("SDL2: Using cache key[%d_%d]\n", color, texture->number);
    //SDL_BlitSurface(search.sprite, &srcrect, screen, &dstrect);
    SDL_RenderCopy(ren, search.sprite, &srcrect, &dstrect);
    return;
  }

  unsigned char channels[4];
  channels[0]=(color >> 24) & 0xFF;
  channels[1]=(color >> 16) & 0xFF;
  channels[2]=(color >>  8) & 0xFF;
  channels[3]=(color >>  0) & 0xFF;
  
  //printf("sz [%f,%f]\n", texture->s0, texture->t0);

  SDL_Surface *text = SDL_CreateRGBSurfaceWithFormatFrom((void *)texture->texture, texture->s0, texture->t0, 0, texture->s0, SDL_PIXELFORMAT_INDEX8);
  if (!text) {
    printf("SDL_CreateRGBSurfaceFrom failure[%s]\n", SDL_GetError());
    return;
  }
  SDL_Palette *palette = text->format->palette;
  // create background color
  palette->colors[0].r = 0;
  palette->colors[0].g = 0;
  palette->colors[0].b = 0;
  palette->colors[1].r = 255;
  palette->colors[1].g = 255;
  palette->colors[1].b = 255;
  // set SDL_RLEACCEL & SDL_SRCCOLORKEY = SDL_TRUE
  SDL_SetColorKey(text, SDL_TRUE, 0);

  SDL_Texture *sdlTexture = SDL_CreateTextureFromSurface(ren, text);
  if (!sdlTexture) {
    printf("CreateTexture Failure [%s]\n", SDL_GetError());
    return;
  }
  SDL_FreeSurface(text);
  SDL_SetTextureColorMod(sdlTexture, channels[0], channels[1], channels[2]);
  //printf("any error [%s]\n", SDL_GetError());
  SDL_RenderCopy(ren, sdlTexture, &srcrect, &dstrect);
  
  // add it to our cache
  if (0) {
    printf("SDL2: Building cache item key[%d_%d] with [%x]\n", color, texture->number, (int)sdlTexture);
    struct textSpiteCacheEntry *cache = malloc(sizeof(struct textSpiteCacheEntry));
    cache->color = color;
    cache->spriteNum = texture->number;
    cache->sprite = sdlTexture;
    dynList_push(&textSprites, cache);
  } else {
    SDL_DestroyTexture(sdlTexture);
  }
  
  /*
  dstrect.w = 1;
  dstrect.h = 1;
  for(uint16_t y = 0; y < srcrect.h; ++y) {
    for(uint16_t x = 0; x < srcrect.w; ++x) {
      uint32_t pos = x + y * srcrect.w;
      const unsigned char *ref = texture->texture + pos;
      const unsigned char chr = *ref;
      if (chr) {
        dstrect.x = position->x + x;
        dstrect.y = position->y + y;
        SDL_SetRenderDrawColor( pWin->rawWindow, channels[0], channels[1], channels[2], channels[3]);
        SDL_RenderFillRect(pWin->rawWindow, &dstrect);
      }
    }
  }
  */
}

bool sdl2_renderer_init() {
  dynList_init(&textSprites, sizeof(struct textSpiteCacheEntry), "textSprites");
  // JOYSTICK causes issue on sun
  if (SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO|SDL_INIT_TIMER)) {
    printf("sdl init failed\n");
    // Initialization failed
    return false;
  }
  SDL_Init(SDL_INIT_JOYSTICK); // who cares if it fails
  return true;
}

void sdl2_changeCursor(const struct window *pWin, uint8_t type) {
  switch(type) {
    case 0:
      //glfwSetCursor(pWin->rawWindow, cursorArrow);
      break;
    case 1:
      //glfwSetCursor(pWin->rawWindow, cursorHand);
      break;
    case 2:
      //glfwSetCursor(pWin->rawWindow, cursorIbeam);
      break;
  }
}

struct window *g_window; // hack for now

struct window* sdl2_renderer_createWindow(const char *title, const struct og_rect *position, const unsigned int flags) {
  printf("Requesting [%d,%d]-[%d,%d]\n", position->x, position->y, position->w, position->h);
  SDL_Window *win = SDL_CreateWindow(title, position->x, position->y, position->w, position->h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  if ( win == NULL ) {
    printf("SDL_CreateWindow Error: [%s]\n", SDL_GetError());
  }
  // try for hw
  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren){
    // fall back to sw
    printf("Falling back to SDL2 software\n");
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
  }
  if (!ren){
    SDL_DestroyWindow(win);
    printf("SDL_CreateRenderer Error: [%s]", SDL_GetError());
    SDL_Quit();
    return 0;
  }
  
  struct window *pWindow = (struct window *)malloc(sizeof(struct window));
  window_init(pWindow);
  pWindow->rawWindow = ren;
  // how can we verify what size we got...
  int w, h;
  SDL_GetRendererOutputSize(ren, &w, &h); // 640x480...
  printf("Got [%d, %d] window size\n", w, h);
  
  if (0) {
    // keep same canvas and just upscale
    pWindow->width = position->w;
    pWindow->height = position->h;
    const float scaleX = (float)w / (float)position->w;
    const float scaleY = (float)h / (float)position->h;
    SDL_RenderSetScale(ren, scaleX, scaleY);
    pWindow->textScaleX = 1.0;
    pWindow->textScaleY = 1.0;
  } else {
    // Do the HiDPI thing
    pWindow->width = w;
    pWindow->height = h;
    pWindow->textScaleX = (float)w / (float)position->w;
    pWindow->textScaleY = (float)h / (float)position->h;
  }
  
  pWindow->clear = sdl2_window_clear;
  pWindow->swap  = sdl2_window_swap;
  pWindow->createSprite          = sdl2_window_createSprite;
  pWindow->createTextSprite      = sdl2_window_createTextSprite;
  pWindow->createSpriteFromColor = sdl2_window_createSpriteFromColor;
  pWindow->drawSpriteBox         = sdl2_window_drawSpriteBox;
  pWindow->drawSpriteText        = sdl2_window_drawSpriteText;
  pWindow->changeCursor          = sdl2_changeCursor;
  g_window = pWindow;

  return pWindow;
}

bool sdl2_renderer_useWindow(const struct window *pWin) {
  return true;
}

bool sdl2_renderer_closeWindow(const struct window *pWin) {
  return true;
}

bool shouldQuit = false;
bool sdl2_renderer_eventsWait(const struct renderers *this, uint32_t wait) {
  SDL_Event event;
  bool res;
  if (wait) {
    res = SDL_WaitEventTimeout(&event, wait)?true:false;
  } else {
    res = SDL_WaitEvent(&event)?true:false;
  }
  switch(event.type) {
    case  SDL_QUIT:
      shouldQuit = true;
      break;
    case SDL_WINDOWEVENT:
      //printf("SDL2 detect window event [%d]\n", event.window.event);
      //SDL_WINDOWEVENT_CLOSE
      if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
        //printf("SDL2 detect window resize [%d,%d]\n", event.window.data1, event.window.data2);
        //g_window->renderDirty = true;
        g_window->width  = event.window.data1;
        g_window->height = event.window.data2;
        g_window->delayResize = 1;
        if (g_window->event_handlers.onResize) {
          g_window->event_handlers.onResize(g_window, event.window.data1, event.window.data2, 0);
        }
      }
      break;
    case SDL_KEYUP:
      if (g_window->event_handlers.onKeyUp) {
        // printf("[%d, %d] mods[%d]\n", event.key.keysym.sym, event.key.keysym.scancode, event.key.keysym.mod);
        // 1 or 2
        if (event.key.keysym.mod & KMOD_LSHIFT || event.key.keysym.mod & KMOD_RSHIFT) {
          int key = event.key.keysym.sym;
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
          event.key.keysym.sym = key;
        }
        g_window->event_handlers.onKeyUp(g_window, event.key.keysym.sym, event.key.keysym.scancode, event.key.keysym.mod, g_window->event_handlers.onKeyUpUser);
      }
      break;
    case SDL_KEYDOWN:
      if (g_window->event_handlers.onKeyDown) {
        g_window->event_handlers.onKeyDown(g_window, event.key.keysym.sym, event.key.keysym.scancode, event.key.keysym.mod, 0);
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (g_window->event_handlers.onMouseUp) {
        g_window->event_handlers.onMouseUp(g_window, event.button.button, event.button.type, g_window->event_handlers.onMouseUpUser);
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (g_window->event_handlers.onMouseDown) {
        g_window->event_handlers.onMouseDown(g_window, event.button.button, event.button.type, g_window->event_handlers.onMouseDownUser);
      }
      break;
    case SDL_MOUSEMOTION:
      // we need to store this for mouseDown
      g_window->cursorX = event.motion.x * g_window->textScaleX; // scale the position for HiDPI
      g_window->cursorY = event.motion.y * g_window->textScaleX; // scale the position for HiDPI
      if (g_window->event_handlers.onMouseMove) {
        g_window->event_handlers.onMouseMove(g_window, event.motion.x, event.motion.y, g_window->event_handlers.onMouseMoveUser);
      }
      break;
    case SDL_MOUSEWHEEL:
      //printf("[%d] = [%d,%d]\n", event.wheel.windowID , event.wheel.x, event.wheel.y);
      if (g_window->event_handlers.onWheel) {
        g_window->event_handlers.onWheel(g_window, event.wheel.x, event.wheel.y * 10, 0);
      }
      break;
  }
  return res;
}

bool sdl2_renderer_shouldQuit(const struct window *pWin) {
  return shouldQuit;
}

uint64_t sdl2_renderer_getTime() {
  return SDL_GetTicks();
}

BaseRenderer renderer_sdl2 = {
  sdl2_renderer_init,
  sdl2_renderer_createWindow,
  sdl2_renderer_useWindow,
  sdl2_renderer_closeWindow,
  sdl2_renderer_eventsWait,
  sdl2_renderer_shouldQuit,
  sdl2_renderer_getTime
};



