#pragma once

#include "src/include/opengem_datastructures.h"
#include "include/opengem/renderer/renderer.h"

#ifdef HAS_FT2
  #include <ft2build.h>
  #include FT_FREETYPE_H
#else
  typedef void* FT_Library;
  typedef void* FT_Face;
  typedef unsigned int FT_UInt;
#endif

#include <stdbool.h>
#include <inttypes.h>

struct ttf {
  unsigned int size;
  FT_Library lib;
  FT_Face    face;
  // getSize
  // rasterize
};

// not used?
struct ttf_glyph {
  float x0;
  float y0;
  float x1;
  float y1;
  float s0;
  float t0;
  float s1;
  float t1;
  unsigned int textureWidth;
  unsigned int textureHeight;
  unsigned char* textureData;
};

// we don't need x,y here except we do need to know how much width we have available
// we need to know x and y it left off at
// x in case we line wrap, so we know where we left off for inline
// y in case of multiline wrap, where the last line ended for inline
// and we'll need to know the starting x
// we don't need the starting y tbh
// even if windowWidth isn't a windowWidth, we still need some type of point to wrap on
struct ttf_rasterization_request {
  struct ttf *font;
  const char *text;
  int availableWidth; // (relative to 0)
  // not availableHeight because we're either going to wrap or not
  // is X relative to parent or absolute to window? well since no wrapTo, it's relative to parent
  int startX; // starting x point (relative to 0) this is currently
  //int wrapToX; // next line starts at X (was always 0)
  
  // scrolling basically:
  int sourceStartX; // start reading text source at and place at destination 0
  int sourceStartY;
  // overflow (0 means no limit)
  int cropWidth;
  int cropHeight;
  bool noWrap; // different than overflow but related
  // since we're colorless, no highlighting for us
  // but we could split up our response into 3 textures...
  /*
  // highlight
  struct og_rect highlight;
  uint16_t hlStart, hlEnd;
  */
};

struct ttf_size_response {
  sizes width;
  sizes height;
  int glyphCount;
  struct dynList letter_sizes;
  // is this start + size = ending?
  // no because size is about texture and ending is about cursor
  // x is probably more important for this... y maybe always match?
  // is y the top or bottom of that last line?
  // going to say top because we can use startY + height to get the last
  uint16_t endingX;
  uint16_t endingY;
  bool wrapped;
  int leftPadding;
  int y0max;
  int wrapToX;
  unsigned int lines;
  struct ttf *font;
};

struct rasterizationResponse {
  uint16_t width;
  uint16_t height;
  uint32_t glyphCount;
  uint16_t endingX;
  uint16_t endingY;
  uint16_t topPad; // keep "a" from being higher than "d"
  bool wrapped;
  unsigned char* textureData;
  struct dynList letter_sizes;
};

void ttf_load(struct ttf *font, const char *path, unsigned int size, unsigned int resolution, bool bold);
uint32_t getFontHeight(struct ttf *font);
// FIXME: split into width / height calcs
// width is much faster if wrap is on
// and sometimes we just need to place the cursor on a specific line
bool ttf_get_size(struct ttf_rasterization_request *request, struct ttf_size_response *res);
void ttf_size_response_destroy(struct ttf_size_response *res);
struct rasterizationResponse *ttf_rasterize(struct ttf_rasterization_request *request);
void ttf_rasterizationResponse_destroy(struct rasterizationResponse *res);

//void ttf_register (void) __attribute__ ((constructor (104)));
void ttf_register (void);

struct ttf_hl_rasterization_request {
  struct ttf_rasterization_request *super;
  struct og_rect *highlight;
  uint16_t hlCharStart, hlCharEnd;
};

bool ttf_hl_rasterize(struct ttf_hl_rasterization_request *request, struct dynList *res);
