#include "jsonrpc.h"
#include <stdio.h> // for printf

#include "include/opengem/network/http/http.h" // for makeUrlRequest
#include "include/opengem/parsers/scripting/json.h" // parseJSON
#include "include/opengem/network/http/header.h" // parseHeaders

char *getChunkSize(char *buffer) {
  char *chunkSizeStr = strtok(buffer, "\r"); // chunkSize
  size_t bufSize = strlen(buffer);
  return chunkSizeStr;
}

char *handle_chunk(char *buffer) {
  // the number of bytes varies...
  // maybe \r would be better
  char *chunkSizeStr = strtok(buffer, "\r"); // chunkSize
  // this is a hex string
  unsigned int chunkSize = strtol(chunkSizeStr, NULL, 16);
  //printf("handle_chunk[%s] [%d]\n", chunkSizeStr, chunkSize);
  if (!chunkSize) return 0;
  size_t bufSize = strlen(buffer);
  size_t chunkStrChars = strlen(chunkSizeStr);
  char *chunkStart = buffer + chunkStrChars + 2; // 2 is the \r\n
  char *buf = malloc(chunkSize + 1);
  strncpy(buf, chunkStart, chunkSize);
  buf[chunkSize] = 0;
  return buf;
}

char *handle_xferenc_response(char *xferEnc, char *body) {
  if (strcmp(xferEnc, "chunked") != 0) {
    printf("Unknown encoding [%s]\n", xferEnc);
    return NULL;
  }
  // the number of bytes varies...
  char *buf;
  char *out = malloc(1); out[0] = 0;
  size_t pos = 0;
  while((buf = handle_chunk(body + pos))) {
    // get the chunk size
    char *sizeStr = getChunkSize(body + pos);
    printf("chunk buf[%s] [%zu] hexsize[%s][%zu]bytes\n", buf, strlen(buf), sizeStr, strlen(sizeStr));
    pos += strlen(buf) + 4 + strlen(sizeStr);
    out = string_concat(out, buf);
    free(buf);
  }
  printf("unchunkedBody[%s]\n", out);
  return out;
}

void handle_jsonrpc_response(const struct http_request *const req, struct http_response *const resp) {
  printf("handle_jsonrpc_response Got [%zu]bytes\n", strlen(resp->body));
  if (resp->complete) {
    printf("handle_jsonrpc_response[%s]\n", resp->body);

    char *hasBody = strstr(resp->body, "\r\n\r\n");
    hasBody += 4;
    // hasbody starts at where the header ends...
    
    // parse header
    struct dynList header;
    dynList_init(&header, sizeof(char*), "http header");
    parseHeaders(resp->body, &header);

    char *xferEncHeader = "Transfer-Encoding";
    char *xferEnc = getHeader(&header, xferEncHeader);
    // has a transfer encoding?
    if (xferEnc != xferEncHeader) {
      printf("Decoding transfer-encoding\n");
      hasBody = handle_xferenc_response(xferEnc, hasBody);
      if (!hasBody) {
        printf("Could not parse xferEnc[%s] body[%s]\n", xferEnc, hasBody);
        return;
      }
    }
    struct jsonrpc_request *jrreq = req->user;
    jrreq->cb(hasBody, jrreq);
  }
}

bool jsonrpc(struct jsonrpc_request *request) {
  char *paramsStr = json_stringify(request->params);
  char postData[128 + strlen(request->method) + strlen(paramsStr)];
  // {\"active_only\":true,\"fields\":{\"public_ip\":true,\"storage_port\":true},\"limit\":5}
  sprintf(postData, "{\"jsonrpc\":\"2.0\",\"id\":\"0\",\"method\":\"%s\",\"params\":%s}", request->method, paramsStr);
  struct dynList *headers = malloc(sizeof(struct dynList));
  dynList_init(headers,"",sizeof(struct keyValue));
  struct keyValue *cthdr = malloc(sizeof(struct keyValue));
  cthdr->key = "Content-Type";
  cthdr->value = "application/json";
  dynList_push(headers, cthdr);
  
  struct keyValue *clhdr = malloc(sizeof(struct keyValue));
  clhdr->key = "Content-Length";
  char strLen[17];
  sprintf(strLen, "%lu", strlen(postData));
  clhdr->value = strdup(strLen);
  dynList_push(headers, clhdr);
  
  struct loadWebsite_t *lw = makeUrlRequest(request->url, postData, headers, request, &handle_jsonrpc_response);
}
