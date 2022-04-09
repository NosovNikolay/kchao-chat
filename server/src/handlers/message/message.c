#include "message.h"

static int handler_message_create(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;
 
    string_t sender_id = prepare_string(bson_get_str(ctx->user, "_id"));
    string_t text = prepare_string(bson_get_str(ctx->bson, "text"));
    string_t type = prepare_string(bson_get_str(ctx->bson, "type"));
    
    if (!type) {
        type = mx_strdup("text");
    }

    string_t chat_id = prepare_string(bson_get_str(ctx->bson, "chat_id"));

    if (!sender_id || !text || !chat_id) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        free_strings(4, &sender_id, &text, &type, &chat_id);
        return 1;
    }

    bson_t *message = send_message(ctx->minst->db, sender_id, chat_id, type, text);
    free_strings(4, &sender_id, &text, &type, &chat_id);

    if (!message) {
        bson_respond_err(res, "400 Can't send message", RESPONSE_400);
        return 1;
    }

    bson_respond(res, RESPONSE_200, message);
    bson_destroy(message);

    return 0;
}

static int handler_message_update(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t chat_id = (string_t)dict_get(req->query, "chat_id");
    string_t msg_id = (string_t)dict_get(req->query, "id");
    string_t text = prepare_string(bson_get_str(ctx->bson, "text"));
    
    if (!chat_id || !msg_id || !text) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        mx_strdel(&text);
        return 1;
    }
    string_t cur_id = prepare_string(bson_get_str(ctx->user, "_id"));
    
    if (!cur_id) {
        bson_respond_err(res, RESPONSE_Unauthorized, RESPONSE_401);
        mx_strdel(&text);
        return 1;
    }
    bson_t *edited = edit_message(ctx->minst->db, msg_id, chat_id, text, cur_id);
    free_strings(2, &cur_id, &text);
    
    if (!edited) {
        bson_respond_err(res, "400 Can't edit message", RESPONSE_400);
        return 1;
    }
    
    bson_respond(res, RESPONSE_200, edited);
    bson_destroy(edited);
    return 0;
}

static int handler_message_delete(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t chat_id = (string_t)dict_get(req->query, "chat_id");
    string_t msg_id = (string_t)dict_get(req->query, "id");

    if (!chat_id || !msg_id) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        return 1;
    }
    
    if(!delete_message(ctx->minst->db, msg_id, chat_id,bson_get_str(ctx->user, "_id"))) {
        bson_respond_err(res, "400 Can't delete message", RESPONSE_400);
        return 1;
    }

    str_respond(res, RESPONSE_200, RESPONSE_OK, CONTENT_TYPE_application_json);
    return 0;
}


static int handler_message_get(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t chat_id = (string_t)dict_get(req->query, "chat_id"); 
    string_t cur_msg = (string_t)dict_get(req->query, "cur"); 
    string_t quantity = (string_t)dict_get(req->query, "quantity");
    
    if (!quantity) {
        quantity = "100";
    }

    if (!chat_id || !cur_msg) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        return 1;
    }

    bson_t *chat = get_chat(ctx->minst->db, chat_id);

    if (!chat) {
        bson_respond_err(res, ERR_CHAT_NOT_FOUND, RESPONSE_400);
        return 1;
    }

    if (!is_in_chat(chat, bson_get_str(ctx->user, "_id"))) {
        bson_destroy(chat);
        bson_respond_err(res, RESPONSE_Forbidden, RESPONSE_400);
        return 1;
    }

    const bson_value_t *collection_number = bson_get(chat, "collection");
    bson_destroy(chat);

    if (!collection_number) {
        bson_respond_err(res, ERR_CHAT_NOT_FOUND, RESPONSE_400);
        return 1;   
    }

    bson_t *messages = get_messages(ctx->minst->db, collection_number->value.v_int64, atoi(cur_msg), atoi(quantity));
    
    if (!messages) {
        bson_respond_err(res, ERR_CHAT_NOT_FOUND, RESPONSE_400);
        return 1; 
    }

    bson_respond(res, RESPONSE_200, messages);
    bson_destroy(messages);
    return 0;
}

void attach_handlers_message(void) {
    add_handler(REQUEST_POST, MESSAGE_PREFIX "/sendText", API_CB(handler_message_create));
    add_handler(REQUEST_PATCH, MESSAGE_PREFIX, API_CB(handler_message_update));
    add_handler(REQUEST_DELETE, MESSAGE_PREFIX, API_CB(handler_message_delete));
    add_handler(REQUEST_GET, MESSAGE_PREFIX, API_CB(handler_message_get));
}
