#include "middlewares.h"

int auth_middleware_before(Request *req, Response *res, middleware_ctx_t *ctx) {
    if (strncmp(req->location, "/auth", 5) == 0 || strcmp(req->location, "/") == 0)
        return 0;

    char *auth_header = get_header(req->headers, HEADER_AUTHORIZATION);
    if (!auth_header) {
        bson_respond_err(res, RESPONSE_Unauthorized, RESPONSE_Unauthorized);
        return 1;
    }

    bson_t *b = NULL;
    bson_error_t err;
    string_t *auth_parts = mx_strdivide(auth_header, ' ');
    string_t token_data = sha512_decode_token(auth_parts[1]);
    mx_del_strarr(&auth_parts);
    if (!token_data || !(b = bson_new_from_json((uint8_t *)token_data, strlen(token_data), &err))) {
        if (token_data)
            free(token_data);
        bson_respond_err(res, RESPONSE_Unauthorized, RESPONSE_Unauthorized);
        return 1;
    }
    free(token_data);
    char *user_id = prepare_string(bson_get_str(b, "user_id"));
    bson_destroy(b);
    b = NULL;
    if (!user_id || !(b = get_user(ctx->minst->db, user_id))) {
        free(user_id);
        bson_respond_err(res, RESPONSE_Unauthorized, RESPONSE_Unauthorized);
        return 1;
    }
    free(user_id);
    ctx->user = b;
    return 0;
}
