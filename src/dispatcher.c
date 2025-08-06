#include "dispatcher.h"
#include "handlers/echo.h"
#include "handlers/home.h"
#include "handlers/user_agent.h"
#include "headers.h"
#include "status.h"
#include <string.h>

void dispatcher(request_t *req, response_t *res) {
    if (strcmp(req->url, "/") == 0) {
        return handle_home(req, res);
    }

    if (strncmp(req->url, "/echo/", 6) == 0) {
        return handle_echo(req, res);
    }

    if (strcmp(req->url, "/user-agent") == 0) {
        return handle_user_agent(req, res);
    }

    res->status_code = STATUS_NOT_FOUND;
    headers_add(&res->headers, "Content-Length", "0");
}
