#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

typedef struct {
    char *method;
    char *url;
} request;

enum HttpStatus {
    STATUS_OK = 200,
    STATUS_NOT_FOUND = 404,
};

enum ParseError {
    PARSE_ERR_READ = -1,
    PARSE_ERR_CLOSED = -2,
    PARSE_ERR_INVALID = -3
};

int parse_request(int fd, request *req);
int send_response(int fd, int status_code);
const char *status_text(int status_code);
void cleanup(request *req);

int main() {
    // Disables output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    // opens a TCP socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("Socket creation failed: %s\n", strerror(errno));
        return 1;
    }

    // allows the server to reuse the port immediately after it restarts
    // ensures we don't run into "Already in use" error
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        printf("SO_REUSEADDR failed: %s\n", strerror(errno));
        return 1;
    }

    // defines an IPV4 socket address
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET, // sets the address family as IPV4
        .sin_port = htons(4221), // sets the port number to 4221, converted to network byte order(big-endian)
        .sin_addr = { htonl(INADDR_ANY) }, // binds to all network interface(0.0.0.0)
    };

    // binds the socket to IP + port
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
        printf("bind failed: %s\n", strerror(errno));
        return 1;
    }

    // sets the maximum number of pending connections
    int connection_backlog = 5;

    // marks the open socket as passive, meaning it is ready to receive incoming client connections
    if (listen(server_fd, connection_backlog) != 0) {
        printf("listen failed: %s\n", strerror(errno));
        return 1;
    }

    printf("Waiting for a client to connect...\n");
    client_addr_len = sizeof(client_addr);

    // blocks and wait until a client connect
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        printf("accept failed: %s\n", strerror(errno));
        return 1;
    }

	printf("Client connected\n");

    request req;
    int result = parse_request(client_fd, &req);
    if (result == PARSE_ERR_READ) {
        printf("failed to read client connection");
        return -1;
    }

    if (result == PARSE_ERR_CLOSED) {
        printf("connection closed\n");
        return -1;
    }

    if (result == PARSE_ERR_INVALID) {
        printf("invalid request");
        return -1;
    }

    if (strcmp(req.url, "/") == 0) {
        if (send_response(client_fd, STATUS_OK) == -1) {
            printf("write failed");
            return -1;
        }
    } else {
        if (send_response(client_fd, STATUS_NOT_FOUND) == -1) {
            printf("write failed");
            return -1;
        }
    }

    cleanup(&req);
    close(server_fd);
    close(client_fd);
    
    return 0;
}

int send_response(int fd, int status_code) {
    char buf[1024];
    int len = snprintf(
        buf, 
        sizeof(buf), 
        "HTTP/1.1 %d %s\r\n", 
        status_code, 
        status_text(status_code)
    );
    if (len < 0 || len >= sizeof(buf)) {
        return -1;
    }

	ssize_t total_sent = 0;
	while (total_sent < len) {
		ssize_t sent = write(fd, buf + total_sent, len - total_sent);
		if (sent < 0) {
            return -1;
		}

		total_sent += sent;
	}

    return 0;
}

const char *status_text(int status_code) {
    switch (status_code) {
        case STATUS_OK: return "OK";
        case STATUS_NOT_FOUND: return "Not Found";
        default: return "Unknown status code";
    }
}

int parse_request(int fd, request *req) {
    char status_line[1024];
    char buf[1];
    ssize_t n;
    int i = 0;

    // reads the status line
    while (true) {
        n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            return PARSE_ERR_READ;
        }

        if (n == 0) {
            return PARSE_ERR_CLOSED;
        }

        if (buf[0] == '\n') {
            break;
        }

        if (buf[0] == '\r') {
            continue;
        }

        status_line[i++] = buf[0];
    }

    status_line[i] = '\0';

    char *save_ptr;

    char *method = strtok_r(status_line, " ", &save_ptr);
    char *url = strtok_r(NULL, " ", &save_ptr);

    if (method == NULL || url == NULL) {
        return PARSE_ERR_INVALID;
    }

    req->method = strdup(method);
    req->url = strdup(url);

    return 0;
}

void cleanup(request *req) {
    free(req->method);
    free(req->url);
    
    req->method = NULL;
    req->url = NULL;
}