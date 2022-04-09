#include "root.h"

static int handler_all(Request *req, Response *res) {
    bson_t headers_bson = BSON_INITIALIZER;
    for (int i = 0; i < req->headers->len; i++)
        BSON_APPEND_UTF8(&headers_bson, req->headers->headers[i].name, req->headers->headers[i].value);

    bson_t query_bson = BSON_INITIALIZER;
    for (int i = 0; i < req->query->len; i++)
        BSON_APPEND_UTF8(&query_bson, req->query->keys[i], (char *)req->query->values[i]);

    bson_t request_bson = BSON_INITIALIZER;
    BSON_APPEND_UTF8(&request_bson, "type", req->type);
    BSON_APPEND_BOOL(&request_bson, "use_ssl", req->ctx->use_ssl);
    BSON_APPEND_UTF8(&request_bson, "location", req->location);
    BSON_APPEND_DOCUMENT(&request_bson, "headers", &headers_bson);
    BSON_APPEND_DOCUMENT(&request_bson, "query", &query_bson);

    res->body = bson_as_json(&request_bson, &res->body_len);
    set_header(res->headers, HEADER_CONTENT_TYPE, CONTENT_TYPE_application_json "; charset=utf-8");
    return 0;
}

static int handler_update_get(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;
    string_t user_id = prepare_string(bson_get_str(ctx->user, "_id"));
    bson_t *events = subscribe_updates_poll(user_id);
    free(user_id);

    if (events) {
        bson_respond(res, RESPONSE_200, events);
        bson_destroy(events);
    } else
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);

    return 0;
}

static int handler_file_get(Request *req, Response *res) {
    oracle_file_t *file = get_file((const char *)dict_get(req->query, "file_id"));
    if (!file) {
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);
        return 1;
    }

    res->body = malloc(file->data_len);
    memcpy(res->body, file->data, file->data_len);
    res->body_len = file->data_len;

    destroy_oracle_file(file);

    return 0;
}

static int handler_search_get(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t search = prepare_string((const char *)dict_get(req->query, "data"));
    if (!search) {
        bson_t *plhdr = BCON_NEW("results", "[", "]");
        bson_respond(res, RESPONSE_200, plhdr);
        bson_destroy(plhdr);
        return 0;
    }

    string_t *search_parts = mx_strdivide(search, ':');
    char *prefix = search_parts[0];

    char *key = NULL;
    char *data = search_parts[1];
    search_func searcher = NULL;

    if (mx_streqi(prefix, "u"))
        searcher = search_users;
    else if (mx_streqi(prefix, "c"))
        searcher = search_chats;

    if (searcher) {
        string_t *tmp = mx_strdivide(data, ':');
        free(search);
        search = mx_strdup(search_parts[1]);
        mx_del_strarr(&search_parts);
        search_parts = tmp;
        prefix = search_parts[0];
        data = search_parts[1];
    } else {
        searcher = search_users;
    }

    if (mx_streqi(prefix, "x")) {
        key = "_id";
    } else if (mx_streqi(prefix, "n")) {
        key = "name";
    } else if (mx_streqi(prefix, "@") && (searcher == search_users || !searcher)) {
        key = "nickname";
    } else if (mx_streqi(prefix, "e") && (searcher == search_users || !searcher)) {
        key = "email";
    } else if (mx_streqi(prefix, "t") && (searcher == search_chats || !searcher)) {
        key = "type";
    } else {
        data = search;
    }

    bson_t *ret = NULL;
    if (searcher)
        ret = searcher(ctx->minst->db, bson_get_str(ctx->user, "_id"), key, data);

    mx_del_strarr(&search_parts);
    free(search);
    if (!ret) {
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);
        return 1;
    }

    bson_respond(res, RESPONSE_200, ret);
    bson_destroy(ret);

    return 0;
}

void attach_handlers(void) {
    add_handler(REQUEST_ALL, "*", API_CB(handler_all));
    add_handler(REQUEST_GET, "/updates", API_CB(handler_update_get));
    add_handler(REQUEST_GET, "/file", API_CB(handler_file_get));
    add_handler(REQUEST_GET, "/search", API_CB(handler_search_get));
    attach_handlers_auth();
    attach_handlers_chat();
    attach_handlers_user();
    attach_handlers_message();
}
