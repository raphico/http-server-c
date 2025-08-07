#include "handlers/user_agent.h"
#include "compression.h"
#include "headers.h"
#include "request.h"
#include "response.h"
#include "status.h"
#include <stdio.h>

void handle_user_agent(request_t *req, response_t *res) {
    char *ua = headers_get(&req->headers, "user-agent");
    if (!ua) {
        res->status_code = STATUS_BAD_REQUEST;
        res->body_len = 0;
        return;
    }

    int n = snprintf(res->body, sizeof(res->body), "%s", ua);
    if (n < 0 || n > (int)sizeof(res->body)) {
        res->status_code = STATUS_INTERNAL_SERVER_ERR;
        res->body_len = 0;
        return;
    }

    res->content_encoding =
        supports_gzip(req) ? ENCODING_GZIP : ENCODING_IDENTITY;

    res->status_code = STATUS_OK;
    res->body_len = (size_t)n;

    headers_add(&res->headers, "Content-Type", "text/plain");
}
