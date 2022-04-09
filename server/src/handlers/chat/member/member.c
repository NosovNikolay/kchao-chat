#include "member.h"

static int handler_chat_member_add(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t user_id = prepare_string(bson_get_str(ctx->bson, "user_id"));
    string_t chat_id = (char *)dict_get(req->query, "chat_id");
    
    if (!user_id || !chat_id) {
        bson_respond_err(res, RESPONSE_400, RESPONSE_Bad_Request);
        mx_strdel(&user_id);
        return 1;
    }

    bson_t *user = get_user(ctx->minst->db, user_id);
    
    if (!user) {
        bson_respond_err(res, ERR_USER_NOT_FOUND, RESPONSE_Bad_Request);
        mx_strdel(&user_id);
        return 1;
    }

    bson_destroy(user);
    
    if (!group_chat_add_user(ctx->minst->db, chat_id, user_id, bson_get_str(ctx->user, "_id"))) {
        bson_respond_err(res, ERR_CHAT_MEMBER_NOT_ADDED, RESPONSE_Bad_Request);
        mx_strdel(&user_id);
        return 1;
    }

    mx_strdel(&user_id);
    str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"User added chat succesfully\"}", CONTENT_TYPE_application_json);
    return 0;
}

static int handler_chat_member_remove(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t cur_user_id = prepare_string(bson_get_str(ctx->user, "_id"));
    string_t rm_member_id = (char *)dict_get(req->query, "member_id");
    string_t chat_id = (char *)dict_get(req->query, "chat_id");

    if (!cur_user_id || !rm_member_id || !chat_id) {
        mx_strdel(&cur_user_id);
        bson_respond_err(res, RESPONSE_400, RESPONSE_Bad_Request);
        return 1;
    }

    if (!group_chat_remove_user(ctx->minst->db, chat_id, rm_member_id, bson_get_str(ctx->user, "_id"))) {
        bson_respond_err(res, ERR_CHAT_MEMBER_NOT_REMOVED, RESPONSE_Bad_Request);
        return 1;
    }

    str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"User removed chat succesfully\"}", CONTENT_TYPE_application_json);
    return 0;
}

void attach_handlers_chat_member(void) {
    add_handler(REQUEST_PUT, CHAT_MEMBER_PREFIX, API_CB(handler_chat_member_add));
    add_handler(REQUEST_DELETE, CHAT_MEMBER_PREFIX, API_CB(handler_chat_member_remove));
}
