#include "status.h"
#include "util.h"

const char *status_text(int status_code) {
    switch (status_code) {
    case STATUS_OK:
        return "OK";
    case STATUS_NOT_FOUND:
        return "Not Found";
    case STATUS_CREATED:
        return "Created";
    case STATUS_BAD_REQUEST:
        return "Bad Request";
    case STATUS_INTERNAL_SERVER_ERR:
        return "Internal Server Error";
    default:
        panic("Unexpected status code");
    }
}
