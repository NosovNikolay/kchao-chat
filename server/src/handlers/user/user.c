#include "user.h"

static int handler_user_get(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;

    const char *user_id = bson_get_str(ctx->user, "_id");
    bson_t *user = get_full_user(ctx->minst->db, user_id);
    // if no user it will fail before this line couse of middleware

    if (user) {
        clear_user_events(ctx->minst->db, user_id);
        bson_respond(res, RESPONSE_200, user);
        bson_destroy(user);
    } else
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);

    return 0;
}

static int handler_user_by_id_get(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;

    string_t user_id = prepare_string((const char *)dict_get(req->query, "user_id"));
    if (!user_id) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_Bad_Request);
        return 1;
    }

    bson_t *user = get_user_min(ctx->minst->db, user_id);

    if (user) {
        bson_respond(res, RESPONSE_200, user);
        bson_destroy(user);
    } else
        bson_respond_err(res, RESPONSE_Not_Found, RESPONSE_404);

    return 0;
}


static int handler_user_update(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;

    bson_t *user = update_user(
        ctx->minst->db,
        bson_get_str(ctx->user, "_id"),
        bson_get_str(ctx->bson, "name"),
        NULL,
        bson_get_str(ctx->bson, "nickname"),
        bson_get_str(ctx->bson, "bio"),
        bson_get_str(ctx->bson, "avatar_id"),
        bson_get_bool(ctx->user, "reseting_password"));

    if (user) {
        bson_respond(res, RESPONSE_200, user);
        bson_destroy(user);
    } else
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);

    return 0;
}

void attach_handlers_user(void) {
    add_handler(REQUEST_GET, USER_PREFIX, API_CB(handler_user_get));
    add_handler(REQUEST_GET, USER_PREFIX "/id", API_CB(handler_user_by_id_get));
    add_handler(REQUEST_PATCH, USER_PREFIX, API_CB(handler_user_update));
    attach_handlers_user_avatar();
}
