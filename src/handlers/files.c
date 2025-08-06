#include "handlers/files.h"
#include "headers.h"
#include "protocol.h"
#include "status.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool isPathSafe(const char *base_path, const char *requested_path);

void handle_post_file(request_t *req, response_t *res) {
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

    FILE *file = fopen(full_path, "w");
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

    if (req->body_len > 0) {
        size_t written = fwrite(req->body, 1, req->body_len, file);
        if (written != req->body_len) {
            res->status_code = STATUS_INTERNAL_SERVER_ERR;
            headers_add(&res->headers, "Content-Length", "0");
            fclose(file);
            return;
        }
    }

    fclose(file);
    res->status_code = STATUS_CREATED;
    headers_add(&res->headers, "Content-Length", "0");
}

void handle_get_file(request_t *req, response_t *res) {
    char *filename = req->url + 7;
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
        fclose(file);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size < 0 || file_size > MAX_BODY_SIZE) {
        fclose(file);
        res->status_code = STATUS_INTERNAL_SERVER_ERR;
        headers_add(&res->headers, "Content-Length", "0");
        return;
    }
    fseek(file, 0, SEEK_SET);

    size_t total_read = fread(res->body, 1, file_size, file);
    fclose(file);

    if (total_read != (size_t)file_size) {
        res->status_code = STATUS_INTERNAL_SERVER_ERR;
        headers_add(&res->headers, "Content-Length", "0");
        return;
    }

    res->status_code = STATUS_OK;
    res->body_len = total_read;

    char len_str[16];
    snprintf(len_str, sizeof(len_str), "%zu", res->body_len);
    headers_add(&res->headers, "Content-Length", len_str);
    headers_add(&res->headers, "Content-Type", "application/octet-stream");
}

bool isPathSafe(const char *base_path, const char *full_path) {
    char resolved_base[PATH_MAX];
    char resolved_target[PATH_MAX];

    if (!realpath(full_path, resolved_target)) {
        return false;
    }

    if (!realpath(base_path, resolved_base)) {
        return false;
    }

    // check if resolve_path starts with base_path
    size_t base_len = strlen(resolved_base);
    return strncmp(resolved_target, resolved_base, base_len) == 0 &&
           (resolved_target[base_len] == '/' ||
            resolved_target[base_len] == '\0');
}
