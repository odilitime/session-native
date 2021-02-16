#include "src/include/opengem_datastructures.h"

struct jsonrpc_request; // fwd dclr

typedef void (jsonrpc_callback)(const char *, struct jsonrpc_request *);

struct jsonrpc_request {
  char *url;
  char *method;
  struct dynList *params;
  jsonrpc_callback *cb;
  void *user;
};

bool jsonrpc(struct jsonrpc_request *request);
