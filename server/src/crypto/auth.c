#include "crypto.h"

static char *dot_concat(const char *a, const char *b) {
    char *res = mx_strnew(strlen(a) + 1 + strlen(b));
    strcat(res, a);
    strcat(res, ".");
    strcat(res, b);
    return res;
}

static char *remove_illegal_chars(const char *str, size_t *new_len) {
    char *no_eq = mx_replace_substr(str, "=", "");
    char *no_pl = mx_replace_substr(no_eq, "+", "-");
    char *no_sl = mx_replace_substr(no_pl, "/", "_");
    free(no_eq);
    free(no_pl);
    if (new_len)
        *new_len = strlen(no_sl);
    return no_sl;
}

static BYTE *hmac_sha512(const void *key, int key_len, const BYTE *data, size_t data_len, BYTE *result, uint32_t *result_len) {
    return HMAC(EVP_sha512(), key, key_len, data, data_len, result, result_len);
}

static char *generate_sha512_signature(const char *payload) {
    BYTE *signature = NULL;
    uint32_t signature_len = -1;
    char *kwd = getenv("AUTH_KEY");
    if (!kwd)
        kwd = "keyword";
    signature = hmac_sha512((const void *)kwd, (int)strlen(kwd), (const BYTE *)payload, strlen(payload), (BYTE *)signature, &signature_len);
    char *signature_b64 = b64_encode((const BYTE *)signature, signature_len);
    char *fixed_signature = remove_illegal_chars(signature_b64, NULL);
    free(signature_b64);
    return fixed_signature;
}

static char *prepare_payload(const char *header, const char *data) {
    char *header_b64 = b64_encode((const BYTE *)header, strlen(header));
    char *data_b64 = b64_encode((const BYTE *)data, strlen(data));
    char *payload = dot_concat(header_b64, data_b64);
    char *fixed_payload = remove_illegal_chars(payload, NULL);

    free(header_b64);
    free(data_b64);
    free(payload);

    return fixed_payload;
}

char *sha512_generate_token(const char *json_data) {
    char *payload = prepare_payload(SHA512_JWT_HEADER, json_data);
    char *signature_b64 = generate_sha512_signature(payload);

    char *result = dot_concat(payload, signature_b64);

    free(payload);
    free(signature_b64);
    return result;
}

char *sha512_decode_token(const char *token) {
    string_t *token_parts = mx_strsplit(token, '.');
    if (!token_parts[0] || !token_parts[1] || !token_parts[2]) {
        mx_del_strarr(&token_parts);
        return NULL;
    }
    char *payload = dot_concat(token_parts[0], token_parts[1]);
    char *test_signature = generate_sha512_signature(payload);
    if (strcmp(token_parts[2], test_signature)) {
        mx_del_strarr(&token_parts);
        free(payload);
        free(test_signature);
        return NULL;
    }

    size_t h_len;
    char *fixed_header = b64_fix_padding(token_parts[0], NULL);
    char *header_decoded = (char *)b64_decode(fixed_header, &h_len);
    if (strcmp(SHA512_JWT_HEADER, header_decoded)) {
        mx_del_strarr(&token_parts);
        free(payload);
        free(test_signature);
        free(fixed_header);
        free(header_decoded);
        return NULL;
    }

    size_t d_len;
    char *fixed_data = b64_fix_padding(token_parts[1], NULL);
    char *data = (char *)b64_decode(fixed_data, &d_len);

    mx_del_strarr(&token_parts);
    free(payload);
    free(test_signature);
    free(fixed_header);
    free(header_decoded);
    free(fixed_data);

    return data;
}

string_t hash_password(const char *pwd) {
    char *salt = "solomon";
    uint32_t hash_len;
    BYTE *hash = NULL;
    hash = HMAC(EVP_sha256(),(const void *)salt, (int)strlen(salt), (const BYTE *)pwd, strlen(pwd), (BYTE *)hash, &hash_len);
    size_t hash_str_len = hash_len * 2;
    string_t hash_str = malloc(hash_str_len + 1);
    memset(hash_str, 0, hash_str_len + 1);
    for (uint32_t i = 0; i < hash_len; i++)
        sprintf(hash_str + (i * 2), "%02hhX", hash[i]);
    return hash_str;
}
