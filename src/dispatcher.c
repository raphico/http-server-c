#include "dispatcher.h"
#include "handlers/echo.h"
#include "handlers/files.h"
#include "handlers/home.h"
#include "handlers/not_found.h"
#include "handlers/user_agent.h"
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

    if (strncmp(req->url, "/files/", 7) == 0) {
        return handle_get_file(req, res);
    }

    return handle_not_found(req, res);
}
