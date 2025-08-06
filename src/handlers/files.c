#include "handlers/files.h"
#include "headers.h"
#include "status.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool isPathSafe(const char *base_path, const char *requested_path);

void handle_get_file(request_t *req, response_t *res) {
    char *filename = req->url + 7; // skips "/files/"
    char *base_path = res->config->directory;

    char full_path[PATH_MAX];
    int n =
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, filename);
    if (n < 0 || n >= (int)sizeof(full_path)) {
        res->status_code = STATUS_INTERNAL_SERVER_ERR;
        headers_add(&res->headers, "Content-Length", "0");
        return;
    }

    FILE *file = fopen(full_path, "rb");
    if (!file) {
        res->status_code = STATUS_NOT_FOUND;
        headers_add(&res->headers, "Content-Length", "0");
        return;
    }

    if (!isPathSafe(base_path, full_path)) {
        res->status_code = STATUS_BAD_REQUEST;
        headers_add(&res->headers, "Content-Length", "0");
        return;
    }

    size_t bytes_read = fread(res->body, 1, sizeof(res->body), file);
    fclose(file);

    res->status_code = STATUS_OK;
    res->body_len = bytes_read;

    char len_str[16];
    snprintf(len_str, sizeof(len_str), "%zu", res->body_len);
    headers_add(&res->headers, "Content-Length", len_str);
    headers_add(&res->headers, "Content-Type", "application/octet-stream");
}

bool isPathSafe(const char *base_path, const char *full_path) {
    // resolve any symlinks(e.g .., .)
    char resolved_path[PATH_MAX];
    if (!realpath(full_path, resolved_path)) {
        return false;
    }

    // check if resolve_path starts with base_path
    size_t base_len = strlen(base_path);
    return strncmp(resolved_path, base_path, base_len) == 0 &&
           (resolved_path[base_len] == '/' || resolved_path[base_len] == '\0');
}
