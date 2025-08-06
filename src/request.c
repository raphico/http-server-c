#include "request.h"
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

void request_init(request_t *req) {
    if (!req) {
        return;
    }

    req->method = NULL;
    req->url = NULL;
}

int request_parse(int fd, request_t *req) {
    char request_line[1024];
    ssize_t n;
    int i = 0;

    // read request line
    while (i < (int)(sizeof(request_line) - 1)) {
        char ch;
        n = recv(fd, &ch, 1, 0);
        if (n < 0) {
            perror("recv");
            return PARSE_ERR_READ;
        }

        if (n == 0) {
            return PARSE_ERR_CLOSED;
        }

        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            break;
        }

        request_line[i++] = ch;
    }

    request_line[i] = '\0';

    char *save_ptr;
    char *method = strtok_r(request_line, " ", &save_ptr);
    char *url = strtok_r(NULL, " ", &save_ptr);

    if (!method || !url) {
        return PARSE_ERR_INVALID;
    }

    // read headers
    headers_t headers = {0};
    if (headers_init(&headers) < 0) {
        return PARSE_ERR_INTERNAL;
    }

    char buf[1024];
    i = 0;
    while (1) {
        char ch;
        n = recv(fd, &ch, 1, 0);
        if (n < 0) {
            perror("recv");
            return PARSE_ERR_READ;
        }

        if (n == 0) {
            return PARSE_ERR_CLOSED;
        }

        if (ch == '\r') {
            continue;
        }

        if (ch != '\n') {
            buf[i++] = ch;
            continue;
        }

        // end of headers
        if (i == 0) {
            break;
        }

        buf[i] = '\0';

        char *save_ptr;
        char *key = strtok_r(buf, ":", &save_ptr);
        char *value = strtok_r(NULL, ":", &save_ptr);

        if (!key || !value) {
            headers_cleanup(&headers);
            return PARSE_ERR_INVALID;
        }

        // trim any leading whitespace
        while (*value == ' ')
            value++;

        headers_add(&headers, key, value);
        i = 0;
    }

    req->method = strdup(method);
    req->url = strdup(url);
    req->headers = headers;
    if (!req->method || !req->url) {
        request_cleanup(req);
        return PARSE_ERR_INTERNAL;
    }

    return 0;
}

void request_cleanup(request_t *req) {
    free(req->method);
    free(req->url);
    headers_cleanup(&req->headers);

    req->method = NULL;
    req->url = NULL;
}
