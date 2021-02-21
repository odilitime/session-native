#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for strdup

// for getline (10.6)
#include <errno.h>   // errno
ssize_t getline(char **linep, size_t *np, FILE *stream);

#include <sodium.h>

#include "include/opengem/network/http/http.h" // for makeUrlRequest
#include "include/opengem/network/protocols.h"

#include "include/opengem/ui/components/component_input.h"
#include "include/opengem/ui/app.h"

#include "include/opengem/session/snode.h"
//#include "include/opengem/session/session.h"
#include "include/opengem/session/send.h"

/*
- text selection
- text copy/paste
- text rendering speed (flashing issue, may have to do with flipping)
- order of messages (bottom should be latest)
- scroll past top
- jump to bottom button
(fix backend, so we can see if polling works)
*/

extern struct dynList snodeURLs;

struct app session;
//struct input_component *textarea = 0;
struct input_component *sendBar = 0;

/// handle scroll events
void onscroll(struct window *win, int16_t x, int16_t y, void *user) {
  //printf("in[%d,%d]\n", x, y);
  // should we flag which layers are scrollable
  // and then we need to actually scroll all the scrollable layers
  // FIXME: iterator over all the layers
  // FIXME: shouldn't each window only have one scroll x/y?
  // individual div has scroll bars too
  
  // reverse find which window this is... in app->windwos
  //struct llLayerInstance *lInst = (struct llLayerInstance *)dynList_getValue(&win->ui->layers, 0);
  struct app_window *firstWindow = (struct app_window *)session.windows.head->value;
  struct llLayerInstance *lInst = (struct llLayerInstance *)dynList_getValue(&firstWindow->rootComponent->layers, 0);
  //struct dynListItem *item = dynList_getItem(&win->ui->layers, 0);
  //struct llLayerInstance *lInst = (struct llLayerInstance *)item->value;
  if (!lInst) {
    printf("onscroll failed to get first layer");
    return;
  }
  double lastScrollY = lInst->scrollY;
  lInst->scrollY -= (double)y / 1.0;
  if (lInst->scrollY < lInst->miny) {
    lInst->scrollY = lInst->miny;
  }
  if (lInst->scrollY > lInst->maxy) {
    lInst->scrollY = lInst->maxy;
  }
  //printf("Y [%d < %f < %d]\n", lInst->miny, lInst->scrollY, lInst->maxy);
  //printf("out[%d]+[%d]\n", y, (int)lInst->scrollY);
  //lInst->scrollX += (double)x / 1.0;
  if (lInst->scrollY != lastScrollY) {
    // if we moved anything, redraw it
    win->renderDirty = true;
  }
}

struct component *hoverComp;
struct component *focusComp = 0;

void onmousemove(struct window *win, int16_t x, int16_t y, void *user) {
  // scan top layer first
  //struct llLayerInstance *uiLayer = (struct llLayerInstance *)dynList_getValue(&win->ui->layers, 1);
  struct app_window *firstWindow = (struct app_window *)session.windows.head->value;
  struct llLayerInstance *uiLayer = (struct llLayerInstance *)dynList_getValue(&firstWindow->rootComponent->layers, 1);
  
  struct pick_request request;
  request.x = (int)x;
  request.y = (int)y;
  //printf("mouse moved to [%d,%d]\n", request.x, request.y);
  request.result = uiLayer->rootComponent;
  hoverComp = component_pick(&request);
  win->changeCursor(win, 0);
  if (hoverComp) {
    //printf("mouse over ui[%s] [%x]\n", hoverComp->name, (int)hoverComp);
    win->changeCursor(win, 2);
  } else {
    // not on top layer, check bottom layer
    //struct llLayerInstance *contentLayer = (struct llLayerInstance *)dynList_getValue(&win->ui->layers, 0);
    struct llLayerInstance *contentLayer = (struct llLayerInstance *)dynList_getValue(&firstWindow->rootComponent->layers, 0);
    
    request.result = contentLayer->rootComponent;
    hoverComp = component_pick(&request);
    if (hoverComp) {
      //printf("mouse over content[%s] [%x]\n", hoverComp->name, (int)hoverComp);
      win->changeCursor(win, 0);
    }
  }
}

void onmouseup(struct window *win, int16_t x, int16_t y, void *user) {
  focusComp = hoverComp;
  if (!hoverComp) return;
  if (hoverComp->event_handlers) {
    if (hoverComp->event_handlers->onMouseUp) {
      hoverComp->event_handlers->onMouseUp(win, x, y, hoverComp);
    }
  }
}
void onmousedown(struct window *win, int16_t x, int16_t y, void *user) {
  if (!hoverComp) return;
  if (hoverComp->event_handlers) {
    if (hoverComp->event_handlers->onMouseDown) {
      hoverComp->event_handlers->onMouseDown(win, x, y, hoverComp);
    }
  }
}

void onkeyup(struct window *win, int key, int scancode, int mod, void *user) {
  //printf("onkeyup key[%d] scan[%d] mod[%d]\n", key, scancode, mod);
  if (focusComp) {
    if (focusComp->event_handlers) {
      //printf("onkeyup key[%d] scan[%d] mod[%d]\n", key, scancode, mod);
      if (key == 13 && sendBar && focusComp == &sendBar->super.super) {
        char *value = textblock_getValue(&sendBar->text);
        printf("Send [%s]\n", value);
        //sendTextToChannel(activeChannel, value);
        free(value);
        input_component_setValue(sendBar, "");
        return;
      }
      if (focusComp->event_handlers->onKeyUp) {
        focusComp->event_handlers->onKeyUp(win, key, scancode, mod, focusComp);
      }
    }
  }
  /*
  if (key == 'd') {
    struct app_window *firstWindow = (struct app_window *)tap.windows.head->value;
    int level = 1;
    //multiComponent_layout(firstWindow->rootComponent, firstWindow->win);
    multiComponent_print(firstWindow->rootComponent, &level);
  }
  if (key == 'r') {
    struct app_window *firstWindow = (struct app_window *)tap.windows.head->value;
    multiComponent_layout(firstWindow->rootComponent, firstWindow->win);
  }
  if (key == 'e') {
    struct app_window *firstWindow = (struct app_window *)tap.windows.head->value;
    firstWindow->win->renderDirty = true;
  }
  */
}

int main(int argc, char *argv[]) {
  time_t t;
  srand((unsigned) time(&t));

  if (sodium_init() == -1) {
    return 1;
  }
  
  thread_spawn();
  if (app_init(&session)) {
    printf("compiled with no renders\n");
    return 1;
  }
  goSnodes();
  printf("Bootstrapped[%zu]\n", (size_t)snodeURLs.count);
  
  FILE *fp = fopen("token.txt", "r");
  char *token = 0;
  if (fp) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1) {
      //printf("Retrieved line of length %zu :\n", read);
      //printf("%s\n", line);
      token = strdup(line);
      free(line);
    }
    fclose(fp);
  }
  if (token) {

  } else {
    //createLoginWindow(&tap);
  }
  
  struct session_keypair skp;
  //crypto_box_keypair(skp.pk, skp.sk);
  crypto_sign_ed25519_keypair(skp.epk, skp.esk);
  crypto_sign_ed25519_sk_to_curve25519(skp.sk, skp.esk);
  crypto_sign_ed25519_pk_to_curve25519(skp.pk, skp.epk);
  
  const char *pubKeyHexStr = "05e308f32ab4bcb9dae4cdd8ebdc912396cd3570832e6a8215e13720c6b0088a3e";
  const char *pos = pubKeyHexStr;
  unsigned char val[32];
  for (size_t count = 0; count < sizeof val/sizeof *val; count++) {
    sscanf(pos, "%2hhx", &val[count]);
    pos += 2;
  }
  
  printf("Pubkey: ");
  for(size_t count = 0; count < sizeof val/sizeof *val; count++)
    printf("%02x", val[count]);
  printf("\n");
  
  //getSwarmsnodeUrl(pubKeyHexStr);
  send("05e308f32ab4bcb9dae4cdd8ebdc912396cd3570832e6a8215e13720c6b0088a3e", &skp, "Hi", 0);
  
  printf("Start loop\n");
  session.loop((struct app *)&session);
  
  return 0;
}

ssize_t
getline(char **linep, size_t *np, FILE *stream)
{
  char *p = NULL;
  size_t i = 0;
  
  if (!linep || !np) {
    errno = EINVAL;
    return -1;
  }
  
  if (!(*linep) || !(*np)) {
    *np = 120;
    *linep = (char *)malloc(*np);
    if (!(*linep)) {
      return -1;
    }
  }
  
  flockfile(stream);
  
  p = *linep;
  for (int ch = 0; (ch = getc_unlocked(stream)) != EOF;) {
    if (i > *np) {
      /* Grow *linep. */
      size_t m = *np * 2;
      char *s = (char *)realloc(*linep, m);
      
      if (!s) {
        int error = errno;
        funlockfile(stream);
        errno = error;
        return -1;
      }
      
      *linep = s;
      *np = m;
    }
    
    p[i] = ch;
    if ('\n' == ch) break;
    i += 1;
  }
  funlockfile(stream);
  
  /* Null-terminate the string. */
  if (i > *np) {
    /* Grow *linep. */
    size_t m = *np * 2;
    char *s = (char *)realloc(*linep, m);
    
    if (!s) {
      return -1;
    }
    
    *linep = s;
    *np = m;
  }
  
  p[i + 1] = '\0';
  return ((i > 0)? i : -1);
}
