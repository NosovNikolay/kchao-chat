#include "chat.h"

static int handler_chat_create(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;
    string_t name = prepare_string(bson_get_str(ctx->bson, "name"));
    string_t admin_id = prepare_string(bson_get_str(ctx->bson, "admin_id"));
    string_t type = prepare_string(bson_get_str(ctx->bson, "type"));
    string_t description = prepare_string(bson_get_str(ctx->bson, "description"));

    if (!admin_id || !type) {
        free_strings(4, &name, &admin_id, &type, &description);
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        return 1;
    }

    const bson_value_t *users_id = bson_get(ctx->bson, "participants"); 
    
    if (!users_id) {
        free_strings(4, &name, &admin_id, &type, &description);
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        return 1;
    }   

    bson_t *doc = bson_new_from_data((const uint8_t*)users_id->value.v_doc.data,
                                    users_id->value.v_doc.data_len);   
    
    BSON_APPEND_UTF8(doc, "0", admin_id);
    
    int users_count = 0;
    bson_t *real_users = get_real_users(ctx->minst->db, doc, &users_count);
    bson_destroy(doc);
   
    if (users_count == 0) {
        bson_destroy(real_users);
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        free_strings(4, &name, &admin_id, &type, &description);
        return 1;
    }

    bson_t *chat;

    if (!strcmp(type, "group")) {
        chat = create_group_chat(ctx->minst->db, name, admin_id, description, real_users);
    }
    else if (!strcmp(type, "private") && users_count < 3) {
        chat = create_private_chat(ctx->minst->db, real_users);
    } else {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        free_strings(4, &name, &admin_id, &type, &description);
        bson_destroy(real_users);
        return 1;
    }

    bson_destroy(real_users);
    free_strings(4, &name, &admin_id, &type, &description);

    if (!chat) {
        bson_respond_err(res, ERR_CHAT_CREATE, RESPONSE_400);
        return 1;
    }

    bson_respond(res, RESPONSE_200, chat);
    bson_destroy(chat);
    return 0;
}

static int handler_chat_get(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t chat_id = (string_t)dict_get(req->query, "id");

    if (!chat_id) {
        bson_respond_err(res, RESPONSE_400, RESPONSE_Bad_Request);
        return 1;
    }
    
    bson_t *chat = get_chat(ctx->minst->db, chat_id);

    if (!chat) {
        bson_respond_err(res, ERR_CHAT_NOT_FOUND, RESPONSE_Bad_Request);
        return 1; 
    }

    if(!is_in_chat(chat, bson_get_str(ctx->user, "_id"))) {
        bson_respond_err(res, RESPONSE_403, RESPONSE_Forbidden);
        bson_destroy(chat);
        return 1; 
    }
    
    bson_respond(res, RESPONSE_200, chat);
    bson_destroy(chat);
    return 0;
}

static int handler_chat_update(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t chat_id = (string_t)dict_get(req->query, "id");

    if (!chat_id) {
        bson_respond_err(res, RESPONSE_400, RESPONSE_Bad_Request);
        return 1;
    }

    string_t name = prepare_string(bson_get_str(ctx->bson, "name"));
    string_t description = prepare_string(bson_get_str(ctx->bson, "description"));
    string_t avatar_id = prepare_string(bson_get_str(ctx->bson, "avatar_id"));

    bson_t *updated = update_group_chat(ctx->minst->db, chat_id, name, description, avatar_id, bson_get_str(ctx->user, "_id")); 
    if (!updated) {
        bson_respond_err(res, ERR_CHAT_NOT_FOUND, RESPONSE_Bad_Request);
        free_strings(3, &name, &description, &avatar_id);
        return 1;
    }
    free_strings(3, &name, &description, &avatar_id);
    bson_respond(res, RESPONSE_200, updated);
    bson_destroy(updated);
    return 0;
}

static int handler_chat_delete(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t chat_id = (string_t)dict_get(req->query, "id");
    if (!chat_id) {
        bson_respond_err(res, RESPONSE_400, RESPONSE_Bad_Request);
        return 1;
    }
    
    string_t cur_id = prepare_string(bson_get_str(ctx->user, "_id")); 
    if(!delete_chat(ctx->minst->db, chat_id, cur_id)) {
        mx_strdel(&cur_id);
        bson_respond_err(res, ERR_CHAT_NOT_FOUND, RESPONSE_Bad_Request);
        return 1; 
    }

    str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"Chat deleted succesfully\"}", CONTENT_TYPE_application_json);
    return 0;
}

void attach_handlers_chat(void) {
    add_handler(REQUEST_POST, CHAT_PREFIX , API_CB(handler_chat_create));
    add_handler(REQUEST_GET, CHAT_PREFIX , API_CB(handler_chat_get));
    add_handler(REQUEST_PATCH, CHAT_PREFIX, API_CB(handler_chat_update));
    add_handler(REQUEST_DELETE, CHAT_PREFIX, API_CB(handler_chat_delete));
    attach_handlers_chat_member();
    attach_handlers_chat_avatar();
}
