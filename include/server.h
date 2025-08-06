#pragma once

typedef struct {
    char *directory;
} server_ctx_t;

void server_start(const int port, char *directory);
