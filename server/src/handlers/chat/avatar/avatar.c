#include "avatar.h"

static int handler_chat_avatar_delete(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;

    string_t chat_id = prepare_string((char *)dict_get(req->query, "id"));
    if (!chat_id) {
        bson_respond_err(res, "400 Chat id not found", RESPONSE_Bad_Request);
        return 1;
    }

    bson_t *chat = get_chat(ctx->minst->db, chat_id);

    if (!chat) {
        free(chat_id);
        bson_respond_err(res, "400 Chat not found", RESPONSE_Bad_Request);
        return 1;
    }

    string_t avatar_id = prepare_string(bson_get_str(chat, "avatar_id"));

    bson_destroy(chat);

    if (!avatar_id) {
        free(chat_id);
        bson_respond_err(res, "304 Avatar not exist", RESPONSE_304);
        return 0;
    }

    chat = update_group_chat(ctx->minst->db, chat_id, NULL, NULL, "", bson_get_str(ctx->user, "_id"));
    free(chat_id);

    if (!chat) {
        free(avatar_id);
        bson_respond_err(res, RESPONSE_304, RESPONSE_Not_Modified);
        return 0;
    }
    bson_destroy(chat);

    bool del_res = delete_file(avatar_id);
    free(avatar_id);

    if (del_res) {
        str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"Avatar deleted\"}", CONTENT_TYPE_application_json);
    } else
        bson_respond_err(res, "304 Avatar not exist", RESPONSE_304);

    return 0;
}

static int handler_chat_avatar_post(Request *req, Response *res, middleware_ctx_t *ctx) {
    if (res->body_len) {
        bson_respond_err(res, "400 Empty request body", RESPONSE_Bad_Request);
        return 1;
    }
    string_t chat_id = prepare_string((char *)dict_get(req->query, "id"));
    if (!chat_id) {
        bson_respond_err(res, "400 Chat id not found", RESPONSE_Bad_Request);
        return 1;
    }

    string_t user_id = prepare_string(bson_get_str(ctx->user, "_id"));

    bson_t *chat = get_chat(ctx->minst->db, chat_id);
    
    if (!chat) {
        free_strings(2, &user_id, &chat_id);
        bson_respond_err(res, "400 Chat id not found", RESPONSE_Bad_Request);
        return 1;
    }

    string_t admin_id = prepare_string(bson_get_str(chat, "admin"));
    string_t prev_file = prepare_string(bson_get_str(chat, "avatar_id"));
    bson_destroy(chat);
    if (!mx_streq(admin_id, user_id)) {
        free(chat_id);
        free(user_id);
        free(admin_id);
        if (prev_file)
            free(prev_file);
        bson_respond_err(res, RESPONSE_Forbidden, RESPONSE_403);
        return 1;
    }
    free(admin_id);
    free(user_id);
    if (prev_file) {
        delete_file(prev_file);
        free(prev_file);
    }

    oracle_file_t *file = post_file((void *)req->body, req->body_len);
    if (!file) {
        free(chat_id);
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);
        return 1;
    }

    chat = update_group_chat(ctx->minst->db, chat_id, NULL, NULL, file->id, bson_get_str(ctx->user, "_id"));
    free(chat_id);

    if (chat) {
        bson_respond(res, RESPONSE_200, chat);
        bson_destroy(chat);
    } else {
        bson_respond_err(res, RESPONSE_304, RESPONSE_Not_Modified);
        delete_file(file->id);
    }

    destroy_oracle_file(file);

    return 0;
}

void attach_handlers_chat_avatar(void) {
    add_handler(REQUEST_DELETE, CHAT_AVATAR_PREFIX, API_CB(handler_chat_avatar_delete));
    add_handler(REQUEST_POST, CHAT_AVATAR_PREFIX, API_CB(handler_chat_avatar_post));
}
