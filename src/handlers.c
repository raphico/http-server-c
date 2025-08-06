#include "handlers.h"
#include "header.h"
#include "status.h"
#include <stdio.h>
#include <string.h>

void handlers_handle_route(request_t *req, response_t *res) {
    if (strcmp(req->url, "/") == 0) {
        res->status_code = STATUS_OK;
        headers_add(&res->headers, "Content-Length", "0");
        return;
    }

    if (strncmp(req->url, "/echo/", 6) == 0) {
        const char *echo = req->url + 6;

        int len = snprintf(res->body, sizeof(res->body), "%s", echo);
        if (len < 0 || len >= (int)sizeof(res->body)) {
            res->status_code = STATUS_INTERNAL_SERVER_ERR;
            headers_add(&res->headers, "Content-Length", "0");
            return;
        }

        res->status_code = STATUS_OK;
        res->body_len = (size_t)len;

        char len_str[16];
        snprintf(len_str, sizeof(len_str), "%zu", res->body_len);
        headers_add(&res->headers, "Content-Length", len_str);
        headers_add(&res->headers, "Content-Type", "text/plain");
        return;
    }

    if (strcmp(req->url, "/user-agent") == 0) {
        char *ua = headers_get(&req->headers, "user-agent");
        if (ua) {
            int len = snprintf(res->body, sizeof(res->body), "%s", ua);
            if (len < 0 || len >= (int)sizeof(res->body)) {
                res->status_code = STATUS_INTERNAL_SERVER_ERR;
                headers_add(&res->headers, "Content-Length", "0");
                return;
            }

            res->status_code = STATUS_OK;
            res->body_len = (size_t)len;
            char len_str[16];
            snprintf(len_str, sizeof(len_str), "%zu", res->body_len);
            headers_add(&res->headers, "Content-Length", len_str);
            headers_add(&res->headers, "Content-Type", "text/plain");
            return;
        }

        res->status_code = STATUS_BAD_REQUEST;
        headers_add(&res->headers, "Content-Length", "0");
        return;
    }

    res->status_code = STATUS_NOT_FOUND;
    headers_add(&res->headers, "Content-Length", "0");
}
