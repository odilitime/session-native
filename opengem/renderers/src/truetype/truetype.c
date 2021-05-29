#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "truetype.h"
#include <unistd.h> // for getcwd

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

struct ttf_query {
  const char *buffer;
  uint32_t size;
};

// FIXME: font_load_request?
void ttf_load(struct ttf *font, const char *path, const unsigned int size, const unsigned int resolution, const bool bold) {
  // we don't need this
  //ttf_register();
  font->size = size;
#ifdef HAS_FT2
  FT_Init_FreeType(&font->lib);
  int errorCode = FT_New_Face(font->lib, path, 0, &font->face);
  if (errorCode) {
    font->face = 0;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
      printf("Current working directory: %s\n", cwd);
    }
    printf("Could not open font [%s][%d]\n", path, errorCode);
    return;
  }
  if (FT_Set_Char_Size(font->face, 0, size * 64, resolution, resolution)) {
    printf("Could not set font size [%s]\n", path);
    return;
  }
#else
  printf("No FreeType2 support\n");
#endif
}

// controls cursor height atm
uint32_t getFontHeight(struct ttf *font) {
#ifdef HAS_FT2
  // font->face->size->metrics.height >> 6 = 15
  return ((font->face->size->metrics.max_advance - font->face->size->metrics.descender) >> 6);
#else
  printf("No FreeType2 library compiled in - using default FontHeight\n");
  return 12;
#endif
}

/*
FT_BBox *compute_string_bbox(struct ttf_rasterization_request *request) {
  FT_BBox *abbox;
  FT_BBox bbox;
  bbox.xMin = bbox.yMin = 32000;
  bbox.xMax = bbox.yMax = -32000;
  uint32_t len = strlen(request->text);
  for(uint8_t n = 0; n < len; ++n) {
    FT_BBox glyph_bbox;
    //FT_Glyph_Get_CBox(request->text[n], BBOX, &glyph_bbox);
  }
  return abbox;
}
*/

// getSize
bool ttf_get_size(struct ttf_rasterization_request *request, struct ttf_size_response *res) {
#ifndef HAS_FT2
  printf("No FreeType2 library compiled in\n");
  return 0;
#else
  if (!request->font->face) {
    printf("No font face set\n");
    return false;
  }
  //1st - addWindow layout -> input_component_resize -> updateCursor
  //2nd - addWindow layout -> input_component_resize -> updateText
  //3rd - input_component_setup > resize > updateCursor
  //4th - input_component_setup > resize > updateText
  //printf("ttf_get_size[%s] sx[%d] aw[%d/%s] ssx[%d]\n", request->text, request->startX, request->availableWidth, request->noWrap?"singleline":"multiline", request->sourceStartX);
  if (!request->availableWidth) {
    printf("ttf_get_size not available width\n");
    free(res);
    return false;
  }
  dynList_init(&res->letter_sizes, sizeof(struct og_rect), "ttf::letter_sizes");
  res->glyphCount  = strlen(request->text);
  if (res->glyphCount) {
    dynList_resize(&res->letter_sizes, res->glyphCount); // inform how many letter sizes to expect
  } else {
    // maybe some other optimizations we can do?
  }
  //printf("GlyphCount [%d]\n", res->glyphCount);
  res->leftPadding = 0;
  res->y0max       = 0;
  res->wrapToX     = 0;
  res->lines       = 1;
  // figure out width/height
  int cx    = 0;
  int cy    = 0;
  int xmax  = 0;
  int y1max = 0;
  res->wrapped = false;
  int lineXStart = request->startX;
  //printf("startX [%d/%d]\n", request->startX, request->availableWidth);
  int maxy0 = 0;

  for(uint32_t i = 0; i <res->glyphCount; ++i) {
    FT_UInt glyph_index = FT_Get_Char_Index(request->font->face, request->text[i]);
    if (FT_Load_Glyph(request->font->face, glyph_index, FT_LOAD_DEFAULT)) {
      printf("Could not load glyph\n");
      free(res);
      return false;
    }
    const FT_GlyphSlot slot = request->font->face->glyph;
    if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL)) {
      printf("Could not render glyph\n");
      free(res);
      return false;
    }
    //printf("[%c] Yadv[%d] Ybear[%d] h[%d] top[%d] rows[%d]\n", request->text[i], slot->metrics.vertAdvance >> 6, slot->metrics.vertBearingY >> 6, slot->metrics.height >> 6, slot->bitmap_top, slot->bitmap.rows);

    struct og_rect *posSize = malloc(sizeof(struct og_rect));
    posSize->x = cx;
    posSize->y = cy;
    
    const uint8_t y0 = slot->bitmap_top;
    const uint8_t glyphHeight = slot->bitmap.rows;
    const int8_t bump = getFontHeight(request->font) - y0;

    maxy0 = max(y0, maxy0);
    // do we need to padding the texture to the left for any lines
    if (cx == 0) {
      if (slot->bitmap_left < 0) {
        // figure out max amount of padding we need
        res->leftPadding = max(res->leftPadding, -slot->bitmap_left);
      }
    }
    // manual wrap
    if (request->text[i] == '\n' || request->text[i] == '\r'){
      res->wrapped = true;
      if (request->noWrap) {
        res->glyphCount = i;
        // I don't think is an actual big deal, rn, maybe in the future...
        // we just truncate glyphCount
        printf("size: newline found, no wrap is on\n");
        free(posSize);
        break;
      } else {
        xmax = max(xmax, cx);
        cx = res->wrapToX;
        cy += ceil(1.2f * request->font->size);
        const uint8_t y1 = cy + bump + glyphHeight;
        y1max = max(y1max, y1);
        res->lines++;
        lineXStart = res->wrapToX;
        //free(posSize);
        // what are we preventing
        //continue;
      }
    }
    // auto wrap to next line on width
    if (cx + lineXStart >= request->availableWidth) {
      //printf("wrapping at [%d]\n", request->availableWidth);
      res->wrapped = true;
      if (request->noWrap) {
        res->glyphCount = i;
      } else {
        xmax = request->availableWidth - res->wrapToX; // the whole width of parent to the edge of windowWidth
        cx = res->wrapToX;
        cy += ceil(1.2f * request->font->size);

        const uint8_t y1 = cy + bump + glyphHeight;
        y1max = max(y1max, y1);

        //printf("cy is now [%d]\n", cy);
        res->lines++;
        lineXStart = res->wrapToX;
        // why aren't we continuing?
      }
    }
    //printf("[%dx%d] + adv[%d,%d] = ", cx, cy, slot->advance.x >> 6, slot->advance.y >> 6);
    cx += slot->advance.x >> 6;
    cy += slot->advance.y >> 6; // always basically 0
    

    //printf("[%dx%d]\n", cx, cy);
    
    // update glyph maxes
    // also should be ascent or bbox.yMax - bbox.yMin
    // we need to include the descent padding
    //printf("max_advance_height[%d]\n", request->font->face->max_advance_height);

    // so y1 now means distance between origin and advanced
    // so the glyph + it's vertical margins so it's centered
    const uint16_t y1 = cy + bump + glyphHeight;
    
    //printf("y [%d]\n", y1);

    posSize->w = cx - posSize->x;
    posSize->h = y1 - posSize->y;
    dynList_push(&res->letter_sizes, posSize);
    //printf("putting [%c] at [%d,%d] to [%d,%d]\n", request->text[i], posSize->x, posSize->y, posSize->w, posSize->h);
    
    res->y0max = max(res->y0max, y0); // FIXME: only needs to be updated when y0 changes...
    y1max = max(y1max, y1);

    // track new max width
    //printf("loop cx[%d]\n", cx);
    xmax = max(xmax, cx);
    
    // crop overflow
    if (request->cropHeight && cy > request->cropHeight) {
      break; // stop adding characters
    }
  }
  if (res->leftPadding) {
    xmax += res->leftPadding;
  }
  //printf("lines[%d] cy[%d] y1max[%d] lineH[%f]\n", res->lines, cy, y1max, ceil(1.2f * request->font->size));
  // FIXME: cy isn't right here (dgj interactions)
  //y1max = (request->font->face->bbox.yMax >> 6);
  //cy += ceil(1.2f * request->font->size);
  //printf("final [%dx%d]\n", cx ,cy);
  //printf("xmax [%d]\n", xmax);
  res->height = max(y1max, getFontHeight(request->font)); // at least one line tall
  res->width  = xmax;
  // make sure it's at least tH tall
  /*
  int textureHeight = (y1max - res->y0max) * res->lines;
  if (res->height < textureHeight) {
    printf("Adjusting height from [%d] to [%d]\n", res->height, textureHeight);
    res->height = textureHeight;
  }
  */
  //printf("SourceStart [%d, %d]\n", request->sourceStartX, request->sourceStartY);
  //printf("cx[%d] ssx[%d]\n", cx, request->sourceStartX);
  // printf("size cx[%d] ssx[%d]\n", cx, request->sourceStartX);

  res->endingX = cx - request->sourceStartX;
  res->endingY = request->sourceStartY + res->height - getFontHeight(request->font);
  //printf("text sz[%d,%d] endPos[%d, %d]\n", res->width, res->height, res->endingX, res->endingY);
  return true;
#endif
}

void *letter_sizes_destroy_iterator(const struct dynListItem *item, void *user) {
  struct og_rect *posSize = item->value;
  free(posSize);
  return user;
}

void ttf_size_response_destroy(struct ttf_size_response *res) {
  // clean lettersizes
  int cont[] = {1};
  dynList_iterator_const(&res->letter_sizes, letter_sizes_destroy_iterator, cont);
}

void ttf_rasterizationResponse_destroy(struct rasterizationResponse *res) {
  int cont[] = {1};
  dynList_iterator_const(&res->letter_sizes, letter_sizes_destroy_iterator, cont);
}

// rasterize
struct rasterizationResponse *ttf_rasterize(struct ttf_rasterization_request *request) {
#ifndef HAS_FT2
  printf("No FreeType2 library compiled in\n");
  return 0;
#else
  if (!request->font) {
    printf("opengem_renderer/fonts/truetype::ttf_rasterize - No font loaded\n");
    return 0;
  }
  //printf("ttf_rasterize[%s] sx[%d] aw[%d/%s] ssx[%d]\n", request->text, request->startX, request->availableWidth, request->noWrap?"singleline":"multiline", request->sourceStartX);
  struct rasterizationResponse *res = (struct rasterizationResponse *)malloc(sizeof(struct rasterizationResponse));
  if (!res) {
    printf("opengem_renderer/fonts/truetype::ttf_rasterize - Can't allocate rasterizationResponse\n");
    return 0;
  }
  res->glyphCount = strlen(request->text);
  //printf("noWrap? [%s]\n", request->noWrap?"noWrap":"wrap");
  struct ttf_size_response sizeResponse;
  if (!ttf_get_size(request, &sizeResponse)) {
    printf("ttf_rasterize - ttf_get_size returned nothing\n");
    return 0;
  }
  // and 0 size is valid
  res->width  = sizeResponse.width;
  res->height = sizeResponse.height;
  res->letter_sizes = sizeResponse.letter_sizes;
  //printf("Planning size around [%d, %d]\n", (int)res->width, (int)res->height);
  // adjust sourceStart
  res->width  -= request->sourceStartX;
  res->height -= request->sourceStartY;

  //uint8_t maxFontHeight = (request->font->face->bbox.yMax + request->font->face->bbox.yMin) >> 6;
  //printf("getFontHeight[%d], bboxHeight[%d]  metricHeight[%ld] | y0max[%d] texHeight[%d]\n", getFontHeight(request->font), maxFontHeight, (request->font->face->size->metrics.height >> 6), res->height, sizeResponse->y0max);
  // I think if bump is working, we don't need this
  /*
  // this needs to be per line... or just the top line...
  if (sizeResponse->lines == 1) {
    //printf("top padding\n");
    // font->face->size->metrics.height >> 6 = 15
   uint8_t maxFontHeight = (request->font->face->bbox.yMax + request->font->face->bbox.yMin) >> 6;
    res->topPad = getFontHeight(request->font) - res->height;
    printf("lines[%d] y0max[%d] getFontHeight[%d], bboxHeight[%d] texHeight[%d] metricHeight[%ld] topPadding[%d]\n", sizeResponse->lines, sizeResponse->y0max, getFontHeight(request->font), maxFontHeight, res->height, (request->font->face->size->metrics.height >> 6), res->topPad);
  } else {
    res->topPad = 0;
  }
  */
  res->topPad = 0;

  uint32_t size    = res->width * res->height;
  res->textureData = (unsigned char *)calloc(1, size); // may need to align up to 8 bytes
  if (!res->textureData) {
    printf("Failured to create ttf texture [%dx%d]\n", res->width, res->height);
    free(res);
    return 0;
  }
  
  const size_t glyphCount = strlen(request->text);
  uint16_t cx = 0;
  uint16_t cy = 0;
  uint16_t maxy0 = 0;
  for(uint64_t i = 0; i < glyphCount; ++i) {
    //printf("[%d,%d][%c]\n", cx, cy, request->text[i]);
    //printf("figuring [%c]\n", request->text[i]);
    FT_UInt glyph_index = FT_Get_Char_Index(request->font->face, request->text[i]);
    if (FT_Load_Glyph(request->font->face, glyph_index, FT_LOAD_DEFAULT)) {
      printf("Could not load glyph[%c]\n", request->text[i]);
      free(res->textureData);
      free(res);
      return 0;
    }
    const FT_GlyphSlot slot = request->font->face->glyph;
    if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL)) {
      printf("Could not render glyph[%c]\n", request->text[i]);
      free(res->textureData);
      free(res);
      return 0;
    }
    const uint8_t xa = slot->advance.x >> 6;
    //const uint8_t ya = slot->advance.y >> 6;

    //const uint8_t yo = 0; // slot->metrics.vertBearingY >> 6;
    //int y0 = yo + slot->bitmap_top;
    const uint8_t y0 = slot->bitmap_top;

    maxy0 = max(y0, maxy0);
    // if just a, y0max isn't going to be max char height
    // we need a static baseline
    const int8_t bump = getFontHeight(request->font) - y0;
    //printf("Bump[%d] bitmap_top[%d] y0max[%d]\n", bump, slot->bitmap_top, sizeResponse->y0max);

    const FT_Bitmap ftBitmap = slot->bitmap;
    
    // manual wrap
    if (request->text[i] == '\n' || request->text[i] == '\r') {
      res->wrapped = true;
      // maybe we should respect manual wrap requests...
      if (request->noWrap) {
        res->glyphCount = i;
        printf("rasterize: newline found, no wrap is on\n");
        break;
      } else {
        //xmax = max(xmax, cx);
        cx = sizeResponse.wrapToX;
        cy += ceil(1.2f * request->font->size);
        //res->lines++;
        //lineXStart =  res->wrapToX;
        continue;
      }
    }
    // auto wrap to next line on width
    if (!request->noWrap && cx + xa >= request->availableWidth) {
      //printf("autowrap\n");
      //xmax = request->availableWidth - res->wrapToX; // the whole width of parent to the edge of windowWidth
      cx = sizeResponse.wrapToX;
      cy += ceil(1.2f * request->font->size);
      //res->lines++;
      //lineXStart = res->wrapToX;
    }
    
    // crop overflow
    if (request->cropHeight && cy > request->cropHeight) {
      break; // stop adding characters
    }
    if (cx < request->sourceStartX) {
      // skip ahead
      cx += xa;
      continue;
    }
    if (cy < request->sourceStartY) {
      // skip ahead
      // seems to just an optimization
      //std::cout << "cy: " << cy << " < sourceStartY: " << request.sourceStartY << std::endl;
      continue;
    }
    
    for(unsigned int iy = 0; iy < ftBitmap.rows; ++iy) {
      uint32_t destPos = cx - request->sourceStartX + sizeResponse.leftPadding + slot->bitmap_left;
      uint32_t temp = (iy + cy - request->sourceStartY + bump) * res->width;
      //printf("temp[%d] == iy[%d] cy[%d] ssY[%d] bump[%d] width[%d]\n", temp, iy, cy, request->sourceStartY, bump, res->textureWidth);
      destPos += temp;
      if (destPos >= size) {
        continue;
      }
      unsigned char *src = ftBitmap.buffer + iy * ftBitmap.width;
      
      unsigned char *dest = res->textureData + destPos;
      uint16_t copyBytes = ftBitmap.width;
      if (destPos + ftBitmap.width > size) {
        copyBytes = size - destPos;
        // seems to be fine
        //printf("ttf_rasterize - Overriding bytes[%d + %d = %d]\n", destPos, copyBytes, size);
        printf("truetype.c::ttf_rasterize [%c] cropping y by [%d]\n", request->text[i], destPos + ftBitmap.width - size);
      }
      // copy row
      memcpy(dest, src, copyBytes);
    }
  
    cx += xa;
  }
  //printf("raster cx[%d] ssx[%d]\n", cx, request->sourceStartX);
  
  res->endingX = cx - request->sourceStartX;
  res->endingY = cy + maxy0 + request->sourceStartY;
  //printf("ending [%d,%d]\n", res->endingX, res->endingY);
  
  return res;
#endif
}

bool ttf_hl_rasterize(struct ttf_hl_rasterization_request *request, struct dynList *res) {
  //dynList_init(res, sizeof(struct rasterizationResponse), "ttf_hl_rasterize res");
  if (request->highlight) {
    dynList_resize(res, 3);
    size_t textLen = strlen(request->super->text);
    
    if (request->hlCharEnd > textLen) {
      printf("fonts/ttf_hl_rasterize - Highlight end [%zu] past end of string[%zu]\n", (size_t)request->hlCharEnd, textLen);
      return false;
    }
    
    // FIXME: checks on start / end vs text strlen
    uint16_t sz = (uint16_t)request->hlCharStart;
    char *first = malloc(sz + 1);
    memcpy(first, request->super->text, sz);
    first[sz] = 0;
    
    struct ttf_rasterization_request req = *request->super; // copy
    req.text = first;
    struct rasterizationResponse *rr = ttf_rasterize(&req);
    dynList_push(res, rr);
    
    sz = request->hlCharEnd - request->hlCharStart;
    char *highlighted = malloc(sz + 1);
    memcpy(highlighted, request->super->text + (uint16_t)request->hlCharStart, sz);
    highlighted[sz] = 0;

    req.text = highlighted;
    rr = ttf_rasterize(&req);
    dynList_push(res, rr);
    
    sz = textLen - ((uint16_t)request->hlCharEnd);
    char *last = malloc(sz + 1);
    memcpy(last, request->super->text + (uint16_t)request->hlCharEnd, sz);
    last[sz] = 0;

    req.text = last;
    rr = ttf_rasterize(&req);
    dynList_push(res, rr);
  } else {
    struct rasterizationResponse *rr = ttf_rasterize(request->super);
    dynList_push(res, rr);
  }
  return true;
}
