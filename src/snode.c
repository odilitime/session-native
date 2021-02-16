#include "jsonrpc.h"
#include "include/opengem/network/http/http.h" // for makeUrlRequest
#include "src/include/opengem_datastructures.h"
#include <stdio.h> // for printf

#include "include/opengem/parsers/scripting/json.h" // parseJSON
#include "include/opengem/network/http/header.h" // parseHeaders

extern bool ssl_verify_cert;

const char *secureSeedNodes[] = {
  "https://storage.seed1.loki.network/json_rpc",
  "https://storage.seed3.loki.network/json_rpc",
  "https://public.loki.foundation/json_rpc",
};

const char *blockBusterSeedNodes[] = {
  "http://116.203.53.213/json_rpc",
  "http://212.199.114.66/json_rpc",
  "http://144.76.164.202/json_rpc",
};

struct dynList snodeURLs;
struct dynList swarmMap;

char *parse_snodeUrl(const char *snodeJson) {
  printf("Parsing [%s]\n", snodeJson);
  struct dynList *snodeData = malloc(sizeof(struct dynList));
  dynList_init(snodeData, sizeof(struct keyValue), "json snode list");
  parseJSON(snodeJson, strlen(snodeJson), snodeData);
  
  char *searchIp = "public_ip";
  char *searchIpRes = dynList_iterator(snodeData, findKey_iterator, searchIp);
  if (searchIpRes == searchIp) {
    printf("Can't find %s [%s]\n", searchIp, snodeJson);
    return 0;
  }
  char *searchPort = "storage_port";
  char *searchPortRes = dynList_iterator(snodeData, findKey_iterator, searchPort);
  if (searchPortRes == searchPort) {
    printf("Can't find %s [%s]\n", searchPort, snodeJson);
    return 0;
  }
  char *url = malloc(128);
  sprintf(url, "https://%s:%s/storage_rpc/v1", searchIpRes, searchPortRes);
  return url;
}

void *addSnode_iterator(const struct dynListItem *item, void *user) {
  const struct keyValue *kv = item->value;
  char *snodeJson = kv->value;
  char *url = parse_snodeUrl(snodeJson);
  dynList_push(&snodeURLs, url);
  return user;
}

void handle_snode_result(const char *result, struct jsonrpc_request *request) {

  struct dynList *list = malloc(sizeof(struct dynList));
  dynList_init(list, sizeof(struct keyValue), "jsonrpc result");
  parseJSON(result, strlen(result), list);
  char *search = "result";
  char *searchRes = dynList_iterator(list, findKey_iterator, search);
  if (searchRes == search) {
    printf("Can't find %s [%s]\n", search, result);
    return;
  }
  
  // FIXME: deallocate old list
  // freeJsonParseIterator
  dynList_reset(list);
  parseJSON(searchRes, strlen(searchRes), list);
  char *search2 = "service_node_states";
  char *search2Res = dynList_iterator(list, findKey_iterator, search2);
  if (search2Res == search2) {
    printf("Can't find %s [%s]\n", search2, result);
    return;
  }

  // FIXME: deallocate old list
  // freeJsonParseIterator
  dynList_reset(list);
  parseJSON(search2Res, strlen(search2Res), list);
  
  // make them active
  dynList_iterator_const(list, addSnode_iterator, 1);
}

char *getRandomSnodeURL() {
  if (!snodeURLs.count) {
    return 0;
  }
  printf("getRandomSnodeURL - Have snodes\n");
  uint16_t snode = rand() % snodeURLs.count;
  struct dynListItem *item = dynList_getItem(&snodeURLs, snode);
  return item->value;
}

void goSnodes() {
  // reset snodeURLs
  dynList_init(&snodeURLs, 128, "list of snode URLs");
  dynList_init(&swarmMap, sizeof(struct keyValue), "list of pubkeys to swarm URLs");
  // build params for jsonrpc
  struct dynList *params = malloc(sizeof(struct dynList));
  dynList_init(params, sizeof(struct keyValue), "goSNodes jsonrpc params");
  struct keyValue kv1;
  kv1.key = "active_only"; kv1.value = "true";
  dynList_push(params, &kv1);
  struct keyValue kv2;
  kv2.key = "fields"; kv2.value = "{\"public_ip\":true,\"storage_port\":true}";
  dynList_push(params, &kv2);
  struct keyValue kv3;
  kv3.key = "limit"; kv3.value = "5";
  dynList_push(params, &kv3);
  // figure out what seed node to use
  uint8_t seedNodeCount = sizeof(secureSeedNodes) / sizeof(secureSeedNodes[0]);
  uint8_t seedNode = rand() % seedNodeCount;
  // make the call
  ssl_verify_cert = true;
  
  struct jsonrpc_request request;
  request.url = (char *)secureSeedNodes[seedNode];
  request.method = "get_n_service_nodes";
  request.params = params;
  request.cb = handle_snode_result;
  jsonrpc(&request);
}

char *parseSwarmsnodeUrl(const char *json) {
  printf("Parsing [%s]\n", json);
  struct dynList *snodeData = malloc(sizeof(struct dynList));
  dynList_init(snodeData, sizeof(struct keyValue), "json snode list");
  parseJSON(json, strlen(json), snodeData);
  
  char *searchIp = "ip";
  char *searchIpRes = dynList_iterator(snodeData, findKey_iterator, searchIp);
  if (searchIpRes == searchIp) {
    printf("Can't find %s [%s]\n", searchIp, json);
    return 0;
  }
  char *searchPort = "port";
  char *searchPortRes = dynList_iterator(snodeData, findKey_iterator, searchPort);
  if (searchPortRes == searchPort) {
    printf("Can't find %s [%s]\n", searchPort, json);
    return 0;
  }
  char *url = malloc(128);
  sprintf(url, "https://%s:%s/storage_rpc/v1", searchIpRes, searchPortRes);
  return url;
}

void *addSwarmSnode_iterator(const struct dynListItem *item, void *user) {
  struct keyValue *kv = item->value;
  struct jsonrpc_request *request = user;
  char *json = kv->value;
  //printf("addSwarmSnode_iterator json[%s]\n", json);
  char *url = parseSwarmsnodeUrl(json);
  printf("adding [%s] for [%s]\n", url, request->user);
  // 
  char *searchPubkeyRes = dynList_iterator(&swarmMap, findKey_iterator, request->user);
  if (searchPubkeyRes == request->user) {
    // create pubKey
    struct dynList *pkList = malloc(sizeof(struct dynList));
    dynList_push(&swarmMap, pkList);
  } else {
    // update pubKey
    
  }
  //swarmMap
  
  return user;
}

void handle_pubKey_result(const char *result, struct jsonrpc_request *request) {
  // does it have a snodes key
  //printf("handle_pubKey_result[%s]\n", result);

  struct dynList *list = malloc(sizeof(struct dynList));
  dynList_init(list, sizeof(struct keyValue), "jsonrpc result");
  parseJSON(result, strlen(result), list);
  char *search = "snodes";
  char *searchRes = dynList_iterator(list, findKey_iterator, search);
  if (searchRes == search) {
    // no snodes...
    printf("Can't find %s [%s]\n", search, result);
    return;
  }
  dynList_reset(list);
  parseJSON(searchRes, strlen(searchRes), list);

  // process swarm updates
  // make them active
  dynList_iterator_const(list, addSwarmSnode_iterator, request);
}

// handle swarm reorgs
char *pubKeyAsk(const char *url, const char *method, const char *pubKey, struct dynList *params) {
  if (params == NULL) {
    params = malloc(sizeof(struct dynList));
    dynList_init(params, 128, "pubKeyAsk params list");
  }
  // add pubKey to the parmas
  struct keyValue kv1;
  kv1.key = "pubKey"; kv1.value = (char *)pubKey;
  dynList_push(params, &kv1);
  // make jsonrpc call
  ssl_verify_cert = false;

  struct jsonrpc_request request;
  request.url = (char *)url;
  request.method = (char *)method;
  request.params = params;
  request.cb = handle_pubKey_result;
  request.user = pubKey;
  jsonrpc(&request);

  return 0;
}

char *getSwarmsnodeUrl(const char *pubkey) {
  const char *nodeURL = getRandomSnodeURL();
  if (!nodeURL) {
    printf("no random snodes yet\n");
    return 0;
  }
  // this should update the swarmMap
  const char *snodeData = pubKeyAsk(nodeURL, "get_snodes_for_pubkey", pubkey, NULL);
  // iterate to find key
  return NULL;
}
