#pragma once

#include "header.h"
#include <stddef.h>

enum {
    MAX_BODY_SIZE = 4096,
    MAX_HEADER_SIZE = 1024,
};

typedef struct {
    char body[MAX_BODY_SIZE];
    int status_code;
    size_t body_len;
    headers_t headers;
} response_t;

int response_init(response_t *res);
void response_cleanup(response_t *res);
void response_send_status(int fd, int status_code);
int response_send(int fd, response_t *res);
