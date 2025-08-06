#include "handlers/echo.h"
#include "headers.h"
#include "status.h"
#include <stdio.h>
#include <string.h>

void handle_echo(request_t *req, response_t *res) {
    const char *echo = req->url + 6;

    int n = snprintf(res->body, sizeof(res->body), "%s", echo);
    if (n < 0 || n >= (int)sizeof(res->body)) {
        res->status_code = STATUS_INTERNAL_SERVER_ERR;
        headers_add(&res->headers, "Content-Length", "0");
        return;
    }

    res->status_code = STATUS_OK;
    res->body_len = (size_t)n;

    char len_str[16];
    snprintf(len_str, sizeof(len_str), "%zu", res->body_len);
    headers_add(&res->headers, "Content-Length", len_str);
    headers_add(&res->headers, "Content-Type", "text/plain");
}
