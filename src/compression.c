#include "compression.h"
#include "headers.h"
#include "protocol.h"
#include <string.h>
#include <zconf.h>
#include <zlib.h>

bool supports_gzip(request_t *req) {
    char *accept_encoding = headers_get(&req->headers, "accept-encoding");
    return accept_encoding && strstr(accept_encoding, "gzip");
}

int compress_body(response_t *res) {
    char compressed[MAX_BODY_SIZE];
    size_t compressed_len = sizeof(compressed);

    // creates and zero a compression stream
    // z_stream is used by zlib to keep track of compression state
    z_stream zs = {0};

    // initialises the stream for gzip compression
    deflateInit2(
        &zs, 
        Z_DEFAULT_COMPRESSION, // balances speed and size
        Z_DEFLATED, // compression algorithm
        15 + 16, // 15: use a 32KB window +16: output GZIP format, not raw
        8, // use 256 bytes for the internal buffer
        Z_DEFAULT_STRATEGY // works for both text and binary
    );

    zs.next_in = (Bytef *)res->body; // keeps track of the current position in the input
    zs.avail_in = res->body_len; // how bytes remain to compress

    zs.next_out = (Bytef *)compressed; // keeps track of the current write position in the output
    zs.avail_out = compressed_len; // how much space is available in the output buffer

    int ref = deflate(&zs, Z_FINISH); // performs the compression
    if (ref != Z_STREAM_END) {
        deflateEnd(&zs);
        return -2;
    }

    compressed_len = zs.total_out; // updates the actual size of the compressed data

    deflateEnd(&zs);

    memcpy(res->body, compressed, compressed_len);
    res->body_len = compressed_len;

    return 0;
}
