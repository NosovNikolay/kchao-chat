#include "headers.h"

Headers *create_headers() {
    Headers *headers = malloc(sizeof(Headers));
    headers->len = 0;
    return headers;
}

static int _get_header_index(Headers *headers, const char *name) {
    for (int idx = 0; idx < headers->len; idx++)
        if (!mx_strcmpi(name, headers->headers[idx].name))
            return idx;
    return -1;
}

int set_header(Headers *headers, const char *name, const char *value) {
    if (headers->len == HEADERS_MAX_LEN || strlen(name) > HEADER_NAME_MAX_LEN || strlen(value) > HEADER_VALUE_MAX_LEN)
        return -1;
    int idx = _get_header_index(headers, name);
    if (idx > -1) {
        memset(headers->headers[idx].value, 0, HEADER_VALUE_MAX_LEN);
        strcpy(headers->headers[idx].value, value);
        return 0;
    }
    struct header_s header;
    memset(header.name, 0, HEADER_NAME_MAX_LEN);
    memset(header.value, 0, HEADER_VALUE_MAX_LEN);
    strcpy(header.name, name);
    strcpy(header.value, value);
    headers->headers[headers->len++] = header;
    return 1;
}

char *get_header(Headers *headers, const char *name) {
    int idx = _get_header_index(headers, name);
    if (idx == -1)
        return NULL;
    return headers->headers[idx].value;
}

int remove_header(Headers *headers, const char *name) {
    int idx = _get_header_index(headers, name);
    if (idx == -1)
        return 0;
    for (int i = idx; i < headers->len - 1; i++)
        headers->headers[i] = headers->headers[i + 1];
    headers->len--;
    return 1;
}

char *headers_2_str(Headers *headers) {
    char *buff = NULL;
    char *old_buff = NULL;
    for (int i = 0; i < headers->len; i++) {
        struct header_s h = headers->headers[i];
        old_buff = buff;
        char *header_str = mx_strnew(strlen(h.name) + strlen(h.value) + 5);
        sprintf(header_str, "%s: %s\r\n", h.name, h.value);
        buff = mx_strjoin(old_buff, header_str);
        free(old_buff);
        free(header_str);
    }
    old_buff = buff;
    buff = mx_strjoin(old_buff, "\r\n");
    free(old_buff);

    return buff;
}

static bool is_valid_header_name(const char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
        if (!isalnum(str[i]) && str[i] != '-' && str[i] != '_')
            return false;
    return true;
}

static bool is_valid_header_value(const char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
        if (!isalnum(str[i]) && !ispunct(str[i]) && str[i] != ' ')
            return false;
    return true;
}

Headers *parse_headers(const char *str, size_t len, int *err) {
    Headers *h = create_headers();
    char buff_name[HEADER_NAME_MAX_LEN + 1], buff_value[HEADER_VALUE_MAX_LEN + 1];
    size_t cur = 0;
    while (cur < len - 2) {
        memset(buff_name, 0, HEADER_NAME_MAX_LEN);
        memset(buff_value, 0, HEADER_VALUE_MAX_LEN);
        mx_memccpy(buff_name, str + cur, ':', HEADER_NAME_MAX_LEN + 1);
        buff_name[mx_get_char_index(buff_name, ':')] = 0;
        cur += strlen(buff_name) + 2;
        mx_memccpy(buff_value, str + cur, '\r', HEADER_VALUE_MAX_LEN + 1);
        buff_value[mx_get_char_index(buff_value, '\r')] = 0;
        cur += strlen(buff_value) + 2;
        if (!is_valid_header_name(buff_name) || !is_valid_header_value(buff_value)) {
            *err = 1;
            break;
        }
        set_header(h, buff_name, buff_value);
    }
    *err = 0;
    return h;
}
