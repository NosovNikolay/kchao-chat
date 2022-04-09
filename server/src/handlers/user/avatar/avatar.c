#include "avatar.h"

static int handler_user_avatar_delete(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;

    string_t user_avatar = prepare_string(bson_get_str(ctx->user, "avatar_id"));
    if (!user_avatar) {
        str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"Avatar not exist\"}", CONTENT_TYPE_application_json);
        return 0;
    }

    bson_t *b = update_user(ctx->minst->db, bson_get_str(ctx->user, "_id"), NULL, NULL, NULL, NULL, "", false);
    if (!b) {
        free(user_avatar);
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);
        return 1;
    }
    delete_file(user_avatar);
    free(user_avatar);
    str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"Avatar deleted\"}", CONTENT_TYPE_application_json);

    return 0;
}

static int handler_user_avatar_post(Request *req, Response *res, middleware_ctx_t *ctx) {
    if (res->body_len) {
        bson_respond_err(res, RESPONSE_400, RESPONSE_Bad_Request);
        return 1;
    }

    oracle_file_t *file = post_file((void *)req->body, req->body_len);
    if (!file) {
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);
        return 1;
    }

    bson_t *user = update_user(ctx->minst->db, bson_get_str(ctx->user, "_id"), NULL, NULL, NULL, NULL, file->id, false);
    destroy_oracle_file(file);

    if (user) {
        string_t prev_file = prepare_string(bson_get_str(ctx->user, "avatar_id"));
        if (prev_file) {
            delete_file(prev_file);
            free(prev_file);
        }
        bson_respond(res, RESPONSE_200, user);
        bson_destroy(user);
    } else
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);

    return 0;
}

void attach_handlers_user_avatar(void) {
    add_handler(REQUEST_DELETE, USER_AVATAR_PREFIX, API_CB(handler_user_avatar_delete));
    add_handler(REQUEST_POST, USER_AVATAR_PREFIX, API_CB(handler_user_avatar_post));
}
