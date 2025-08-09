[![progress-banner](https://backend.codecrafters.io/progress/http-server/e590a1ed-5581-4c50-92c3-173fc54ce8b7)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

A minimal HTTP/1.1 server written from scratch in C.

## Why I built this

I built this project to practice C in a real-world context, working with socket I/O, file I/O, concurrency, and the standard library. It helped me deepen my understanding of memory management, focusing on safe design patterns, stack vs. heap trade-offs, and avoiding pitfalls like unsafe internal pointers.

I also explored multiple approaches to the same problem (e.g., `snprintf` vs. `sprintf`, `snprintf` vs. `itoa`, `strtok_r` vs. `strtok`, `memcpy`, `strdup` vs `strcpy`, `strncpy`) to understand why certain methods are preferred for safety and performance.

Finally, this was an exercise in writing idiomatic C, with a modular, maintainable code structure

## Architecture overview

This section outlines the main components of the program and how they interact — from the moment a client opens a TCP connection, through request parsing, routing, and response generation.

See the [sequence diagram](./docs/architecture.png) for a visual walkthrough of the process.

## Features

1. Manual HTTP/1.1 Request Parsing

   - a custom HTTP parser in C
   - Supports request line parsing, header parsing, and body extraction

2. Response generation pipeline

   - Dynamically builds and sends HTTP/1.1 responses with proper status lines, headers, and bodies
   - Supports both text and binary payloads

3. Routing System

   - Simple internal dispatcher for mapping routes to handlers
   - Supports static and dynamic routes (e.g., /echo/:string)

4. Persistent Connections

   - Reuses the same TCP connection for multiple HTTP requests when allowed by the client
   - Correctly handles Connection: close request

5. Static File Serving

   - Serves files directly from the filesystem with correct Content-Type detection
   - Efficiently streams large files without loading them fully into memory
   - Prevents directory traversal attacks by sanitizing and validating file paths

6. File Writing Support

   - Allows safe writing of uploaded data to disk
   - Enforces path restrictions to prevent overwriting or creating files outside allowed directories

7. Gzip Compression

   - Uses zlib to compress responses when the client supports Content-Encoding: gzip
   - Falls back to uncompressed content when unsupported

8. Error Handling & Status Codes

   - Returns appropriate HTTP error responses (400, 404, 500, etc.)
   - Handles malformed requests gracefully without crashing the server

9. Concurrent/Threaded Connection Handling
   - Each connection processed in its own thread for concurrent request handling

## How to run

1. Clone the repository

```bash
git clone git@github.com:raphico/http-server-c.git
cd http-server-c
```

2. Start server

```bash
./your_program.sh
```

3. Try

```bash
# Basic Request handling
curl -v http://localhost:4221

curl -v http://localhost:4221/echo/abc

curl -v --header "User-Agent: foobar/1.2.3" http://localhost:4221/user-agent

# Concurrent connections
(sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
(sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
(sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &

# File Reading/writing
echo -n 'Hello, World!' > /tmp/foo
curl -i http://localhost:4221/files/foo

curl -v --data "12345" -H "Content-Type: application/octet-stream" http://localhost:4221/files/file_123

# HTTP compression
curl -v -H "Accept-Encoding: gzip" http://localhost:4221/echo/abc | hexdump -C

# Persistent connections
curl --http1.1 -v http://localhost:4221/echo/banana --next http://localhost:4221/user-agent -H "User-Agent: blueberry/apple-blueberry"

curl --http1.1 -v http://localhost:4221/echo/orange --next http://localhost:4221/ -H "Connection: close"
```

## Project Structure

```bash
├── src/                         # Core source code for the HTTP server
│   ├── main.c                   # Program entry point
│   ├── server.c                 # Creates socket, binds, listens, and accepts client connections
│   ├── request.c                # Manual HTTP/1.1 request parsing
│   ├── response.c               # Constructs and sends HTTP responses to clients
│   ├── dispatcher.c             # Routes parsed requests to the appropriate handler
│   ├── compression.c            # Implements gzip compression using zlib
│   ├── utils.c                  # General helper and utility functions
│   ├── status.c                 # Defines HTTP status codes and their corresponding messages
│   ├── headers.c                # Handles HTTP header creation and management
│   └── handlers/                # Individual route handlers for specific endpoints
│
└── include/                     # Header files for modularity, type definitions, and forward declarations
```

## Try the Challenge Yourself

This project was built as part of the [Codecrafters "Build Your Own HTTP Server" Challenge](https://app.codecrafters.io/r/zealous-guineapig-988263). Use my link to get **7 days free** if you want to take the challenge and build your own server from scratch.
