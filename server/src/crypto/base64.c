#include "crypto.h"

static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int b64invs[] = {62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
                        59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
                        6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                        21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
                        29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
                        43, 44, 45, 46, 47, 48, 49, 50, 51};

static size_t b64_encoded_size(size_t size) {
    size_t res;

    res = size;
    if (size % 3 != 0)
        res += 3 - (size % 3);
    res /= 3;
    res *= 4;

    return res;
}

static size_t b64_decoded_size(const char *src) {
    if (src == NULL)
        return 0;

    size_t len = strlen(src);
    size_t res = len / 4 * 3;

    for (int i = len; i-- > 0;)
        if (src[i] == '=')
            res--;
        else
            break;
    return res + 1;
}

static int b64_isvalidchar(char c) {
    if (c >= '0' && c <= '9')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c >= 'a' && c <= 'z')
        return 1;
    if (c == '+' || c == '/' || c == '=')
        return 1;
    return 0;
}

char *b64_encode(const BYTE *src, size_t len) {
    char *res;
    size_t res_size;

    if (src == NULL || len == 0)
        return NULL;

    res_size = b64_encoded_size(len);
    res = malloc(res_size + 1);
    res[res_size] = '\0';

    for (size_t i = 0, j = 0; i < len; i += 3, j += 4) {
        size_t s = src[i];
        s = i + 1 < len ? s << 8 | src[i + 1] : s << 8;
        s = i + 2 < len ? s << 8 | src[i + 2] : s << 8;
        res[j] = b64chars[(s >> 18) & 0x3F];
        res[j + 1] = b64chars[(s >> 12) & 0x3F];
        res[j + 2] = i + 1 < len ? b64chars[(s >> 6) & 0x3F] : '=';
        res[j + 3] = i + 2 < len ? b64chars[s & 0x3F] : '=';
    }

    return res;
}

BYTE *b64_decode(const char *src, size_t *decoded_len) {
    size_t d_len = 0;
    if (decoded_len) *decoded_len = 0;
    if (src == NULL)
        return NULL;
    size_t len = strlen(src);
    if (len % 4 != 0)
        return NULL;
    size_t i, j;
    for (i = 0; i < len; i++)
        if (!b64_isvalidchar(src[i]))
            return NULL;

    d_len = b64_decoded_size(src);
    BYTE *res = malloc(d_len);
    memset(res, 0, d_len);
    if (decoded_len) *decoded_len = d_len;

    for (i = 0, j = 0; i < len; i += 4, j += 3) {
        int s = b64invs[src[i] - 43];
        s = (s << 6) | b64invs[src[i + 1] - 43];
        s = src[i + 2] == '=' ? s << 6 : (s << 6) | b64invs[src[i + 2] - 43];
        s = src[i + 3] == '=' ? s << 6 : (s << 6) | b64invs[src[i + 3] - 43];

        res[j] = (s >> 16) & 0xFF;
        if (src[i + 2] != '=')
            res[j + 1] = (s >> 8) & 0xFF;
        if (src[i + 3] != '=')
            res[j + 2] = s & 0xFF;
    }

    return res;
}

char *b64_fix_padding(const char *src, size_t *new_len) {
    size_t len = strlen(src);
    if (len % 4 == 0) {
        if (new_len)
            *new_len = len;
        return mx_strdup(src);
    }
    size_t n_len = len;
    while (n_len % 4)
        n_len++;
    char *res = mx_strnew(n_len);
    memset(res, '=', n_len);
    memcpy(res, src, len);
    if (new_len)
        *new_len = n_len;
    return res;
}

char *b64_remove_padding(const char *str, size_t *new_len) {
    int idx = mx_get_char_index(str, '=');
    size_t len = strlen(str);
    if (idx == -1) {
        if (new_len)
            *new_len = len;
        return mx_strdup(str);
    }
    if (new_len)
        *new_len = idx;
    char *res = mx_strnew(idx);
    memcpy(res, str, idx);
    return res;
}
