#pragma once

#include <ctype.h>
#include <libmx.h>
#include <string.h>

#define HEADERS_MAX_LEN 100
#define HEADER_NAME_MAX_LEN 100
#define HEADER_VALUE_MAX_LEN 1024

struct header_s {
    char name[HEADER_NAME_MAX_LEN];
    char value[HEADER_VALUE_MAX_LEN];
};

typedef struct headers_s {
    struct header_s headers[HEADERS_MAX_LEN];
    int len;
} Headers;

Headers *create_headers();

int set_header(Headers *headers, const char *name, const char *value);

char *get_header(Headers *headers, const char *name);

int remove_header(Headers *headers, const char *name);

char *headers_2_str(Headers *headers);

Headers *parse_headers(const char *str, size_t len, int *err);
