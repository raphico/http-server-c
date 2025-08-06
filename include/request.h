#pragma once

#include "headers.h"
#include "protocol.h"

typedef struct {
    headers_t headers;
    char *method;
    char *url;
    char body[MAX_BODY_SIZE];
    size_t body_len;
} request_t;

enum ParseError {
    PARSE_ERR_READ = -1,
    PARSE_ERR_CLOSED = -2,
    PARSE_ERR_INVALID = -3,
    PARSE_ERR_INTERNAL = -4,
};

void request_init(request_t *req);
int request_parse(int fd, request_t *req);
void request_cleanup(request_t *req);
