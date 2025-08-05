#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
  char *method;
  char *url;
} request;

enum HttpStatus {
  STATUS_OK = 200,
  STATUS_NOT_FOUND = 404,
  STATUS_BAD_REQUEST = 400,
  STATUS_INTERNAL_SERVER_ERR = 500,
};

enum ParseError {
  PARSE_ERR_READ = -1,
  PARSE_ERR_CLOSED = -2,
  PARSE_ERR_INVALID = -3,
  PARSE_ERR_INTERNAL = -4,
};

int parse_request(int fd, request *req);
int send_response(int fd, int status_code, const char *body, size_t body_len);
const char *status_text(int status_code);
void cleanup(request *req);
__attribute__((noreturn)) void panic(const char *msg);

int main() {
  // Disables output buffering
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  socklen_t client_addr_len;
  struct sockaddr_in client_addr;

  // opens a TCP socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    goto cleanup;
  }

  // allows the server to reuse the port immediately after it restarts
  // ensures we don't run into "Already in use" error
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    perror("setsockopt");
    goto cleanup;
  }

  // defines an IPV4 socket address
  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET,   // sets the address family as IPV4
      .sin_port = htons(4221), // sets the port number to 4221, converted to
                               // network byte order(big-endian)
      .sin_addr = {htonl(
          INADDR_ANY)}, // binds to all network interface(0.0.0.0)
  };

  // binds the socket to IP + port
  if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
    perror("bind");
    goto cleanup;
  }

  // sets the maximum number of pending connections
  int connection_backlog = 5;

  // marks the open socket as passive, meaning it is ready to receive incoming
  // client connections
  if (listen(server_fd, connection_backlog) != 0) {
    perror("listen");
    goto cleanup;
  }

  printf("Waiting for a client to connect...\n");
  client_addr_len = sizeof(client_addr);

  // blocks and wait until a client connect
  int client_fd =
      accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd < 0) {
    perror("accept");
    goto cleanup;
  }

  printf("Client connected\n");

  request req = {0};
  int status_code;
  char body[4096];
  size_t body_len = 0;

  int result = parse_request(client_fd, &req);
  if (result < 0) {
    body[0] = '\0';
    if (result == PARSE_ERR_INTERNAL) {
      fprintf(stderr, "Memory allocation failed\n");
      send_response(client_fd, STATUS_INTERNAL_SERVER_ERR, body, body_len);
    } else {
      switch (result) {
      case PARSE_ERR_READ:
        fprintf(stderr, "Failed to read from client (I/O error)\n");
        break;
      case PARSE_ERR_CLOSED:
        fprintf(stderr, "Client closed connection unexpectedly\n");
        break;
      case PARSE_ERR_INVALID:
        fprintf(stderr, "Malformed HTTP request\n");
        break;
      default:
        fprintf(stderr, "Unknown parsing error code: %d\n", result);
        break;
      }
      send_response(client_fd, STATUS_BAD_REQUEST, body, body_len);
    }
    goto cleanup;
  }

  if (strcmp(req.url, "/") == 0) {
    status_code = STATUS_OK;
    body[0] = '\0';
  } else if (strstr(req.url, "echo") != NULL) {
    char *save_ptr;
    char *parts[2];
    int count = 0;

    char *token = strtok_r(req.url, "/", &save_ptr);
    while (token != NULL && count < 2) {
      parts[count++] = token;
      token = strtok_r(NULL, "/", &save_ptr);
    }

    if (count == 2 && strtok_r(NULL, "/", &save_ptr) == NULL) {
      status_code = STATUS_OK;

      int len = snprintf(body, sizeof(body), "%s", parts[1]);
      if (len < 0 || len >= (int)sizeof(body)) {
        status_code = STATUS_INTERNAL_SERVER_ERR;
        body_len = 0;
      } else {
        body_len = (size_t)len;
      }
    } else {
      status_code = STATUS_BAD_REQUEST;
    }
  } else {
    status_code = STATUS_NOT_FOUND;
  }

  if (send_response(client_fd, status_code, body, body_len) == -1) {
    perror("send_response");
    goto cleanup;
  }

cleanup:
  cleanup(&req);
  if (client_fd != -1)
    close(client_fd);
  if (server_fd != -1)
    close(server_fd);
  return 0;
}

int send_response(int fd, int status_code, const char *body, size_t body_len) {
  char header[1024];
  int len = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: %zu\r\n"
                     "\r\n",
                     status_code, status_text(status_code), body_len);
  if (len < 0 || len >= sizeof(header)) {
    return -1;
  }

  // send header
  ssize_t total_sent = 0;
  while (total_sent < len) {
    ssize_t sent =
        send(fd, header + total_sent, len - total_sent, MSG_NOSIGNAL);
    if (sent < 0) {
      return -1;
    }

    total_sent += sent;
  }

  // send body
  total_sent = 0;
  while (total_sent < body_len) {
    ssize_t sent =
        send(fd, body + total_sent, body_len - total_sent, MSG_NOSIGNAL);
    if (sent < 0) {
      return -1;
    }

    total_sent += sent;
  }

  return 0;
}

const char *status_text(int status_code) {
  switch (status_code) {
  case STATUS_OK:
    return "OK";
  case STATUS_NOT_FOUND:
    return "Not Found";
  case STATUS_BAD_REQUEST:
    return "Bad Request";
  case STATUS_INTERNAL_SERVER_ERR:
    return "Internal Server Error";
  default:
    panic("Unexpected status code");
  }
}

int parse_request(int fd, request *req) {
  char request_line[4096];
  ssize_t n;
  int i = 0;

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

  req->method = strdup(method);
  req->url = strdup(url);
  if (!req->method || !req->url) {
    cleanup(req);
    return PARSE_ERR_INTERNAL;
  }

  return 0;
}

void cleanup(request *req) {
  free(req->method);
  free(req->url);

  req->method = NULL;
  req->url = NULL;
}

void panic(const char *msg) {
  fprintf(stderr, "PANIC: %s\n", msg);
  abort();
}
