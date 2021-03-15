#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for strdup

ssize_t getline(char **linep, size_t *np, FILE *stream);

#include <sodium.h>

#include "include/opengem/network/http/http.h" // for makeUrlRequest
#include "include/opengem/network/protocols.h"

#include "include/opengem/ui/components/component_button.h"
#include "include/opengem/ui/components/component_input.h"
#include "include/opengem/ui/components/component_tab.h"
#include "include/opengem/ui/app.h"

#include "include/opengem/session/snode.h"
//#include "include/opengem/session/session.h"
#include "include/opengem/session/send.h"
#include "include/opengem/session/recv.h"

#include "include/opengem/timer/scheduler.h"
//#include "include/opengem/ui/textblock.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

// Access MacOS APIs from c/c++
// https://codereview.stackexchange.com/a/95909
/*
- initial UI
- open group support
- profile support
- avatar / attachment support
- mutlithreading
- encrypted database
- onion routing support / lokinet support
- closed group support
*/

/*
LEAKS
- openssl, reuse more...
- openssl, clean up more...
 */

extern struct dynList snodeURLs;

struct app session;
struct tabbed_component *convoTab;
struct tabbed_component *messagesTab;
struct text_component *identityText;
struct input_component *input;
struct button_component *sendButton;
struct text_component *statusBar;
struct llLayerInstance *newConvoLayer;
struct input_component *idInput;

struct dynList conversations;
struct session_keypair current_skp;

struct conversation {
  char *pubkeyStr;
  struct tab *tab;
  uint16_t badgeCount;
};

#define HEAP_ALLOCATE(TYPE,VAR) TYPE *VAR = malloc(sizeof(TYPE));

void selectConversation(struct tab *tab) {
  // unselect old one
  //convoTab->selectedTabId = convoId;
  convoTab->selectedTab = tab;
  // find messages tabs
  struct llLayerInstance *ll = messagesTab->super.layers.head->value;
  // erase them
  dynList_reset(&ll->rootComponent->children);
  ll->rootComponent->renderDirty = true;
  if (tab) {
    printf("selectConversation - Enabling input\n");
    input->super.super.disabled = false;
  } else {
    input->super.super.disabled = true;
    sendButton->super.disabled = true;
  }
  convoTab->super.super.renderDirty = true;
}

void *find_conversation_iterator(const struct dynListItem *item, void *user) {
  struct conversation *convo = item->value;
  if (strcmp(user, convo->pubkeyStr) == 0) {
    return convo;
  }
  return user;
}

void *recv_callback_iter(const struct dynListItem *item, void *user) {
  // event system?
  struct handleUnidentifiedMessageType_result *result = item->value;
  //struct session_recv_io *io = user;
  char *from = malloc(67);
  pkToString(result->source, from); // convert to hexstring
  if (result->content->datamessage) {
    printf("%s: %s\n", from, result->content->datamessage->body);
    char *useFrom = from;
    if (strcmp(from, current_skp.pkStr) == 0) {
      useFrom = "Note to self";
    }
    // we do have a conversation for this from?
    void *searchRes = dynList_iterator_const(&conversations, find_conversation_iterator, useFrom);
    if (searchRes == from) {
      // if not create it
      HEAP_ALLOCATE(struct conversation, conf)
      conf->pubkeyStr = strdup(from);
      conf->badgeCount = 0;
      conf->tab = tab_add(convoTab, conf->pubkeyStr, session.activeAppWindow->win);
      selectConversation(conf->tab);
      dynList_push(&conversations, conf);
      searchRes = conf;
    }
    struct conversation *convo = searchRes;
    //strcmp(convo->source, from) == 0
    // is the selected conversation == from
    if (convoTab->selectedTab == convo->tab) {
      // if so add it to the existing list
      tab_add(messagesTab, result->content->datamessage->body, session.activeAppWindow->win);
    } else {
      // update badge
      char *label = malloc(strlen(convo->tab->titleBox.text) + 8);
      sprintf(label, "%s (%d)", convo->tab->titleBox.text, convo->badgeCount);
      //char *oldLabel = convo->tab->titleBox.text;
      free((char *)convo->tab->titleBox.text);
      convo->tab->titleBox.text = label;
      convo->badgeCount++;
      convoTab->super.super.renderDirty = true;
    }
  } else {
    // non-data message
  }
  free(from);
  return user;
}

struct recv_callback_data {
  struct session_keypair *skp;
  char lastHash[129]; // hexstring version
};

void updateStatusBar(char *status) {
  text_component_setText(statusBar, status);
  app_window_render(session.activeAppWindow); // draw it to the screen
  // we can't make setText auto draw because it doesn't know we're done making change
}

bool recv_callback(struct md_timer *timer, double now) {
  updateStatusBar("Checking for new messages");
  struct recv_callback_data *recv_data = timer->user;
  struct session_keypair *skp = recv_data->skp;
  struct session_recv_io io;
  io.kp = skp;
  io.contents = 0;
  io.lastHash = recv_data->lastHash;
  session_recv(&io);
  updateStatusBar("Updating UI...");
  if (io.contents) {
    printf("Found [%zu] messages\n", (size_t)io.contents->count);
    dynList_iterator_const(io.contents, recv_callback_iter, &io);
    free(io.contents);
  }
  // if changed...
  if (io.lastHash != recv_data->lastHash) {
    // pass lastHash to next call
    strncpy(recv_data->lastHash, io.lastHash, MIN(strlen(io.lastHash), 129));
  }
  text_component_setText(statusBar, "Ready");
  return true;
}

void *getComponentById(char *id) {
  void *res = app_window_group_instance_getElementById(session.activeAppWindowGroup, id);
  if (!res) {
    printf("Cannot find %s\n", id);
    return 0;
  };
  return res;
}

void eventHandler(struct app_window *appwin, struct component *comp, const char *event) {
  printf("eventHandler - event[%s]\n", event);
  bool found = false;
  switch(event[0]) {
    case 'c': {
      // copy
      if (strcmp(event, "copy") == 0) {
        struct text_component *text = (struct text_component *)comp;
        appwin->win->setClipboard(appwin->win, (char *)text->text);
        text_component_setText(statusBar, "Identity copied to clipboard");
        found = true;
      } else
      if (strcmp(event, "cancelConvo") == 0) {
        newConvoLayer->hidden = true;
        newConvoLayer->rootComponent->renderDirty = true;
        found = true;
      } else
      if (strcmp(event, "createConvo") == 0) {
        char *newId = textblock_getValue(&idInput->text);;
        if (strlen(newId) < 64 || strlen(newId) > 66) {
          printf("Invalid SessionID size\n");
          text_component_setText(statusBar, "Invalid SessionID size");
          return;
        }
        HEAP_ALLOCATE(struct conversation, conf)
        conf->pubkeyStr = newId;
        if (strcmp(conf->pubkeyStr, identityText->text) == 0) {
          conf->pubkeyStr = strdup("Note to self");
        }
        conf->badgeCount = 0;
        conf->tab = tab_add(convoTab, conf->pubkeyStr, session.activeAppWindow->win);
        selectConversation(conf->tab);
        dynList_push(&conversations, conf);
        text_component_setText(statusBar, "conversation created!");
        input_component_setValue(idInput, "");

        newConvoLayer->hidden = true;
        newConvoLayer->rootComponent->renderDirty = true;
        component_setBlur(&idInput->super.super, appwin);

        found = true;
      }
      break;
    }
    case 'n': {
      // newConvo
      // check clipboard for one sessionid
      const char *clipboard = appwin->win->getClipboard(appwin->win);
      if (strlen(clipboard) == 66) {
        printf("May have sessionID in clipboard\n");
        // set the value...
        input_component_setValue(idInput, clipboard);
      }
      newConvoLayer->hidden = false;
      newConvoLayer->rootComponent->renderDirty = true;
      component_setFocus(&idInput->super.super, appwin);
      found = true;
      break;
    }
    case 's': {
      // send
      char *msg = textblock_getValue(&input->text);
      if (!strlen(msg)) {
        text_component_setText(statusBar, "No message!");
        return;
      }
      // to who?
      if (!convoTab->selectedTab) {
        text_component_setText(statusBar, "No conversation selected!");
        return;
      }
      // blocking action incoming
      input_component_setValue(input, "");
      sendButton->super.disabled = true;
      updateStatusBar("Sending...");
      const char *dest = convoTab->selectedTab->titleBox.text;
      if (strcmp(dest, "Note to self") == 0) {
        dest = identityText->text;
      }
      //printf("Sending [%s] to [%s]\n", msg, dest);
      session_send(dest, &current_skp, msg, 0);
      free(msg);
      text_component_setText(statusBar, "Ready");
      found = true;
      break;
    }
    default:
      printf("eventHandler - unknown event[%s]\n", event);
      found = true;
      break;
  }
  if (!found) {
    printf("eventHandler - unknown event[%s]\n", event);
  }
}

struct component *lastTab = 0;
// defeat dragging to sort tho..
bool tabMouse = false;
void tab_onMouseUp(struct window *win, int button, int mods, void *user) {
  struct app_window *const appwin = user;
  //printf("tab_onMouseUp [%s] hover[%s]\n", appwin->rootComponent.super.name, appwin->hoverComponent->name);
  tabMouse = false;
  if (!lastTab || !lastTab->parent) {
    // click on tabbed control but not a tab...
    // maybe try reseting X and trying again..
    return;
  }
  struct tab *tab = lastTab->parent->pickUser;
  selectConversation(tab); // actually make it active...
  tabbed_component_selectTab((struct tabbed_component *)appwin->hoverComponent, tab);
}

void tab_onMouseDown(struct window *win, int x, int y, void *user) {
  struct app_window *const appwin = user;
  // pick
  // translate tab pos into pick
  // and then which
  //printf("tab_onMouseDown\n");
  tabMouse = true;
  if (!lastTab || !lastTab->parent) {
    // click on tabbed control but not a tab...
    // maybe try reseting X and trying again..
    return;
  }
  struct tab *tab = lastTab->parent->pickUser;
  struct tabbed_component *tComp = (struct tabbed_component *)appwin->hoverComponent;
  tabbed_component_selectTab(tComp, tab);
}

void tab_onMouseMove(struct window *win, int16_t x, int16_t y, void *user) {
  //printf("tab_onMouseMove [%d,%d]\n", x, y);
  struct app_window *const appwin = user;
  // feels a tad sluggish
  // we could do component_pick but it just calls this anyways
  struct component *pick = multiComponent_pick(appwin->hoverComponent, appwin->win->cursorX, appwin->win->cursorY);
  lastTab = pick;
  if (!pick) {
    appwin->win->changeCursor(appwin->win, 0);
    return;
  }
  appwin->win->changeCursor(appwin->win, pick->hoverCursorType);
  // is mouse down?
  if (tabMouse) {
    if (!lastTab->parent) {
      // click on tabbed control but not a tab...
      // maybe try reseting X and trying again..
      printf("no parent[%s]\n", lastTab->name);
      return;
    }
    struct tab *tab = lastTab->parent->pickUser;
    tabbed_component_selectTab((struct tabbed_component *)appwin->hoverComponent, tab);
  } else {
    //printf("Mouse is up\n");
  }
}

void idInput_onKeyUp(struct window *win, int key, int scancode, int mod, void *user) {
  //printf("toInput_onKeyUp\n");
  if (key == 13) {
    //printf("toInput_onKeyUp enter\n");
    eventHandler(user, 0, "createConvo");
    return;
  }
  input_component_keyUp(win, key, scancode, mod, idInput);
}

void input_onKeyUp(struct window *win, int key, int scancode, int mod, void *user) {
  //printf("input_onKeyUp\n");
  if (key == 13) {
    //printf("input_onKeyUp enter\n");
    eventHandler(user, 0, "send");
    return;
  }
  input_component_keyUp(win, key, scancode, mod, input);
  char *msg = textblock_getValue(&input->text);
  if (strlen(msg)) {
    sendButton->super.disabled = false;
  } else {
    sendButton->super.disabled = true;
  }
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
  
  // create app
  struct app_window_group_template wgSession;
  app_window_group_template_init(&wgSession, &session);
  // load ntr into app
  struct app_window_template wtSession;
  app_window_template_init(&wtSession, "Resources/session-native.ntrml");
  // we would set winPos
  wtSession.title = "session-native";
  //wtBrowser.winPos.w = 640;
  //wtBrowser.winPos.h = 480;
  //wtBrowser.desiredFPS = 60;
  app_window_group_template_addWindowTemplate(&wgSession, &wtSession);
  wgSession.leadWindow = &wtSession;
  wgSession.eventHandler = eventHandler;
  
  // start app
  //struct app_window_group_instance *wingrp_inst =
  app_window_group_template_spawn(&wgSession, session.renderer);
  
  // wire up interface
  identityText = getComponentById("identity");
  // (struct tabbed_component *)
  convoTab = getComponentById("conversations");
  messagesTab = getComponentById("messages");
  input = getComponentById("sendbar");
  sendButton = getComponentById("sendBut");
  statusBar = getComponentById("statusBar");
  idInput = getComponentById("toIdentity");
  struct component *ncLater = getComponentById("newConvo");
  newConvoLayer = multiComponent_layer_findByComponent(&session.activeAppWindow->rootComponent, ncLater);
  if (!newConvoLayer) {
    printf("Can't find layer newConvo\n");
  }
  if (!(identityText && convoTab && messagesTab && input && sendButton && statusBar && newConvoLayer && idInput)) {
    return 1;
  }
  updateStatusBar("Starting...");
  
  dynList_init(&conversations, sizeof(struct conversation), "conversations");
  
  goSnodes();
  while(!snodeURLs.count) {
    printf("Bootstrapping\n");
    goSnodes();  
  }
  printf("Bootstrapped[%zu]\n", (size_t)snodeURLs.count);
  
  //crypto_box_keypair(skp.pk, skp.sk);
  crypto_sign_ed25519_keypair(current_skp.epk, current_skp.esk);
  crypto_sign_ed25519_sk_to_curve25519(current_skp.sk, current_skp.esk);
  int res = crypto_sign_ed25519_pk_to_curve25519(current_skp.pk, current_skp.epk);
  // res is usually zero
  if (res) {
    printf("Signing ED result[%d]\n", res);
    return 1;
  }
  current_skp.pkStr = malloc(67);
  pkToString(current_skp.pk, current_skp.pkStr); // convert to hexstring
  printf("Session started for [%s]\n", current_skp.pkStr);
  
  text_component_setText(identityText, current_skp.pkStr);
  input->super.super.event_handlers->onKeyUp = input_onKeyUp;
  //input->super.super.event_handlers->onKeyUpUser = input;
  if (!convoTab->super.super.event_handlers) {
    convoTab->super.super.event_handlers = malloc(sizeof(struct event_tree));
    event_tree_init(convoTab->super.super.event_handlers);
  }
  convoTab->super.super.event_handlers->onMouseDown = tab_onMouseDown;
  convoTab->super.super.event_handlers->onMouseMove = tab_onMouseMove;
  convoTab->super.super.event_handlers->onMouseUp = tab_onMouseUp;
  input->super.super.disabled = true;
  sendButton->super.disabled = true;
  messagesTab->selectColor = 0x000000FF;
  tabbed_component_updateColors(messagesTab, session.activeAppWindow->win);
  idInput->super.super.event_handlers->onKeyUp = idInput_onKeyUp;
  
  //getSwarmsnodeUrl(pubKeyHexStr);
  //send("05e308f32ab4bcb9dae4cdd8ebdc912396cd3570832e6a8215e13720c6b0088a3e", &skp, "Hi", 0);
  
  struct recv_callback_data recv_data;
  strcpy(recv_data.lastHash, "undefined");
  recv_data.skp = &current_skp;
  if (1) {
    struct md_timer *reciever = setInterval(recv_callback, 10000);
    reciever->user = &recv_data;
  }
  
  text_component_setText(statusBar, "Ready");

  printf("Start loop\n");
  session.loop(&session);
  free(current_skp.pkStr);
  return 0;
}

#include "getline.c"
