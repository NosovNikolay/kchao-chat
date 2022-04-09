#include "api.h"

static string_t _hostname = NULL;
static string_t _authorization = NULL;

void init_api(const char *port, const char *auth_token) {
    int _port = port ? atoi(port) : 0;
    if (!_port)
        _hostname = mx_strdup(API_URL);
    else {
        char buff[12];
        sprintf(buff, "%d", _port);
        _hostname = mx_strjoin("localhost:", buff);
    }

    if (auth_token)
        _authorization = mx_strjoin("Bearer ", auth_token);
}

void destroy_api() {
    if (_hostname)
        free(_hostname);
    if (_authorization)
        free(_authorization);
    _hostname = NULL;
    _authorization = NULL;
}

void api_authorize(const char* auth_token) {
    if (_authorization)
        free(_authorization);
    if (!auth_token)
        _authorization = NULL;
    else
        _authorization = mx_strjoin("Bearer ", auth_token);
}

char *get_hostname() {
    return _hostname;
}

string_t create_url(const char *path) {
    return mx_strjoin(_hostname, path);
}

RCHeaders *get_default_headers() {
    RCHeaders *headers = RC_create_headers();
    if (_authorization)
        RC_set_header(headers, "Authorization", _authorization);
    return headers;
}

RCResponse *api_send_json(const char *method, const char *path, bson_t *obj) {
    RCHeaders *headers = get_default_headers();
    if (obj)
        RC_set_header(headers, "Content-Type", "application/json");
    string_t json = NULL;
    if (obj)
        json = bson_as_canonical_extended_json(obj, NULL);
    string_t url = create_url(path);
    RCResponse *res = RC_send_request(method, url, headers, json, obj ? strlen(json) : 0);
    free(url);
    if (obj)
        free(json);
    RC_destroy_headers(headers);
    return res;
}

RCResponse *api_send_data(const char *method, const char *path, void *data, size_t len) {
    RCHeaders *headers = get_default_headers();
    if (data)
        RC_set_header(headers, "Content-Type", "image");
    string_t url = create_url(path);
    RCResponse *res = RC_send_request(method, url, headers, data, data ? len : 0);
    free(url);
    RC_destroy_headers(headers);
    return res;
}

static RCResponse *api_send_json_ctx(async_json_ctx *ctx) {
    RCResponse *res = api_send_json(ctx->method, ctx->path, ctx->obj);
    if (ctx->method)
        free(ctx->method);
    if (ctx->path)
        free(ctx->path);
    if (ctx->obj)
        bson_destroy(ctx->obj);
    free(ctx);
    return res;
}

static RCResponse *api_send_data_ctx(async_data_ctx *ctx) {
    RCResponse *res = api_send_data(ctx->method, ctx->path, ctx->data, ctx->len);
    if (ctx->method)
        free(ctx->method);
    if (ctx->path)
        free(ctx->path);
    free(ctx);
    return res;
}

GThread *api_send_json_async(const char *method, const char *path, bson_t *obj, void (*cb)(RCResponse *, void *), void *cb_data) {
    async_json_ctx *ctx = malloc(sizeof(async_json_ctx));
    ctx->method = method ? mx_strdup(method) : NULL;
    ctx->path = path ? mx_strdup(path) : NULL;
    ctx->obj = obj ? bson_copy(obj) : NULL;
    return PROMISE(api_send_json_ctx, cb, ctx, cb_data);
}

GThread *api_send_data_async(const char *method, const char *path, void *data, size_t len, void (*cb)(RCResponse *, void *), void *cb_data) {
    async_data_ctx *ctx = malloc(sizeof(async_data_ctx));
    ctx->method = method ? mx_strdup(method) : NULL;
    ctx->path = path ? mx_strdup(path) : NULL;
    ctx->data = data ? data : NULL;
    ctx->len = len;
    return PROMISE(api_send_data_ctx, cb, ctx, cb_data);
}
