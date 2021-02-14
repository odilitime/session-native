#include "include/opengem/network/http/http.h" // for makeUrlRequest
#include "src/include/opengem_datastructures.h"
#include <stdio.h> // for printf

#include "include/opengem/parsers/scripting/json.h"

const char *seedNode = "https://storage.seed1.loki.network/json_rpc";

void handle_http(const struct http_request *const req, struct http_response *const resp) {
  if (resp->complete) {
    printf("Handler[%s]\n", resp->body);
    printf("Got [%u]bytes\n", strlen(resp->body));
    char *hasBody = strstr(resp->body, "\r\n\r\n");

    struct dynList *list = malloc(sizeof(struct dynList));
    dynList_init(list, sizeof(struct keyValue), "json list");
    parseJSON(hasBody + 4, strlen(hasBody) - 4, list);
    char *search = "result";
    char *searchRes = dynList_iterator(list, findKey_iterator, search);
    if (searchRes == search) {
      printf("Can't find data [%s]\n", hasBody + 4);
      return;
    }

  }
}

void goSnodes() {
  // ,\"params\":\"{}\"
  const char *postData = "{\"jsonrpc\":\"2.0\",\"id\":\"0\",\"method\":\"get_n_service_nodes\"}";
  struct dynList *headers = malloc(sizeof(struct dynList));
  dynList_init(headers,"",sizeof(struct keyValue));
  struct keyValue *cthdr = malloc(sizeof(struct keyValue));
  cthdr->key = "Content-Type";
  cthdr->value = "application/json";
  dynList_push(headers, cthdr);

  struct keyValue *clhdr = malloc(sizeof(struct keyValue));
  clhdr->key = "Content-Length";
  char strLen[1024];
  sprintf(strLen, "%lu", strlen(postData));
  clhdr->value = strdup(strLen);
  dynList_push(headers, clhdr);
  
  makeUrlRequest(seedNode, postData, headers, 0, &handle_http);
}
