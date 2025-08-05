#pragma

enum HttpStatus {
    STATUS_OK = 200,
    STATUS_NOT_FOUND = 404,
    STATUS_BAD_REQUEST = 400,
    STATUS_INTERNAL_SERVER_ERR = 500,
};

const char *status_text(int status_code);
