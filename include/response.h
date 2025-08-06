#pragma once

#include "compression.h"
#include "headers.h"
#include "protocol.h"
#include "server.h"
#include <stddef.h>

typedef struct {
    char body[MAX_BODY_SIZE];
    int status_code;
    size_t body_len;
    headers_t headers;
    const server_ctx_t *config;
    content_encoding_t content_encoding;
} response_t;

int response_init(response_t *res, const server_ctx_t *ctx);
void response_cleanup(response_t *res);
void response_send_status(int fd, int status_code);
int response_send(int fd, response_t *res);
