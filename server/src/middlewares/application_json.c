#include "middlewares.h"

int json_middleware_before(Request *req, Response *res, middleware_ctx_t *ctx) {
    if (!is_json_req(req))
        return 0;

    set_header(res->headers, HEADER_CONTENT_TYPE, CONTENT_TYPE_application_json);
    bson_t *b;
    bson_error_t err_format;
    size_t err_offset = 0;
    char buff[128];
    if (!(b = bson_new_from_json((const uint8_t *)req->body, req->body_len, &err_format))) {
        sprintf(buff, "%" PRIu32, err_format.code);
        res->body = create_json_err(buff, err_format.message);
        res->body_len = strlen(res->body);
        res->status = RESPONSE_400;
        return 1;
    }
    if (!bson_validate(b,
                       BSON_VALIDATE_UTF8 |
                           BSON_VALIDATE_DOLLAR_KEYS |
                           BSON_VALIDATE_DOT_KEYS |
                           BSON_VALIDATE_EMPTY_KEYS,
                       &err_offset)) {
        sprintf(buff, "error at position %zu", err_offset);
        res->body = create_json_err("-1", buff);
        res->body_len = strlen(res->body);
        res->status = RESPONSE_400;
        bson_destroy(b);
        return 1;
    }
    ctx->bson = b;
    return 0;
}

struct req_arr_s {
    char *method;
    char *url;
};

int expect_json_middleware_before(Request *req, Response *res) {
    static struct req_arr_s skip[] = {
        // Here should be list of endpoints which shouldn`t be application/json type
        {REQUEST_GET, "*"},
        {REQUEST_DELETE, "*"},
        {REQUEST_HEAD, "*"},
        {REQUEST_POST, "/user/avatar"},
        {REQUEST_POST, "/chat/avatar"}, 
        {"", ""}};
    for (int i = 0; strlen(skip[i].url); i++)
        if ((mx_streq(skip[i].method, "*") || mx_streq(skip[i].method, req->type)) &&
            (mx_streq(skip[i].url, "*") || mx_streq(skip[i].url, req->location)))
            return 0;
    if (is_json_req(req))
        return 0;
    set_header(res->headers, HEADER_CONTENT_TYPE, CONTENT_TYPE_application_json);
    res->body = create_json_err("42", "Expected json body");
    res->body_len = strlen(res->body);
    res->status = RESPONSE_400;
    return 1;
}
