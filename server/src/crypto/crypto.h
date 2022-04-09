#pragma once

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <libmx.h>

typedef unsigned char BYTE; // 8-bit byte

/* base64 */

char *b64_encode(const BYTE *src, size_t len);

BYTE *b64_decode(const char *src, size_t *decoded_len);

char *b64_fix_padding(const char *src, size_t *new_len);

char *b64_remove_padding(const char *str, size_t *new_len);

/* sha512 */

#define SHA512_JWT_HEADER "{\
\"alg\":\"HS512\",\
\"typ\":\"JWT\"\
}"

#define USER_TOKEN_DATA "{\
\"user_id\":\"<user_id>\"\
}"


char *sha512_generate_token(const char *json_data);

char *sha512_decode_token(const char *token);

string_t hash_password(const char *pwd);
