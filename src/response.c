#include "response.h"
#include "header.h"
#include "status.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>

int response_init(response_t *res) {
    if (!res) {
        return -1;
    }

    res->status_code = 0;
    res->body[0] = '\0';
    res->body_len = 0;
    return headers_init(&res->headers);
}

void response_cleanup(response_t *res) {
    if (!res) {
        return;
    }

    headers_cleanup(&res->headers);
}

void response_send_status(int fd, int status_code) {
    char buf[128];
    int len = snprintf(buf, sizeof(buf),
                       "HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n",
                       status_code, status_text(status_code));
    send(fd, buf, len, 0);
}

int response_send(int fd, response_t *res) {
    char header[MAX_HEADER_SIZE];
    size_t offset = 0;

    // Add status line
    int n = snprintf(header, sizeof(header), "HTTP/1.1 %d %s\r\n",
                     res->status_code, status_text(res->status_code));
    if (n < 0 || (size_t)n >= sizeof(header)) {
        return -1;
    }
    size_t len = (size_t)n;
    offset += len;

    // Add headers
    for (int i = 0, count = res->headers.count; i < count; i++) {
        char *key = res->headers.items[i].key;
        char *value = res->headers.items[i].value;

        n = snprintf(header + offset, sizeof(header) - offset, "%s: %s\r\n",
                     key, value);
        if (n < 0 || (size_t)n >= sizeof(header) - offset) {
            return -1;
        }
        len = (size_t)n;
        offset += len;
    }

    // End headers section
    if (offset + 2 >= sizeof(header)) {
        return -1;
    }
    header[offset++] = '\r';
    header[offset++] = '\n';

    // Send headers
    ssize_t total_sent = 0;
    while ((size_t)total_sent < offset) {
        ssize_t sent =
            send(fd, header + total_sent, offset - total_sent, MSG_NOSIGNAL);
        if (sent < 0) {
            return -1;
        }
        total_sent += sent;
    }

    // Send body
    total_sent = 0;
    while ((size_t)total_sent < res->body_len) {
        ssize_t sent = send(fd, res->body + total_sent,
                            res->body_len - total_sent, MSG_NOSIGNAL);
        if (sent < 0) {
            return -1;
        }
        total_sent += sent;
    }

    return 0;
}
