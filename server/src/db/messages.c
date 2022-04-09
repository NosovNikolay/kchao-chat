#include "db.h"

mongoc_collection_t *get_chat_message_collection(mongoc_database_t* db, long int key_num) {
    char buff[30];
    sprintf(buff, "chat_%ld", key_num);
    return mongoc_database_get_collection(db, buff);
}

bson_t *get_message(mongoc_database_t* db, const char *msg_id, const char *chat_id) {
    bson_t *query = get_id_doc(msg_id);
    bson_t *chat = get_chat(db, chat_id);
    
    if (!chat) {
        bson_destroy(query);
        return NULL;
    }

    const bson_value_t *collection_v = bson_get(chat, "collection");

    mongoc_collection_t *col = get_chat_message_collection(db, collection_v->value.v_int64);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(col, query, NULL, NULL);
    mongoc_collection_destroy(col);

    const bson_t *tmp;
    bson_t *doc = NULL;

    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    bson_destroy(query);
    bson_destroy(chat);
    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *get_message_from_collection(mongoc_collection_t *col, const char *msg_id) {
    bson_t *query = get_id_doc(msg_id);

    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(col, query, NULL, NULL);

    const bson_t *tmp;
    bson_t *doc = NULL;

    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *send_message(mongoc_database_t* db, const char *from_id, const char *chat_id, const char *type, const char *text) {
    bson_t *chat = get_chat(db, chat_id);

    if (!chat) {
        return NULL;
    }

    if(!is_in_chat(chat, (char *)from_id)) {
        return NULL;
    }
    
    const bson_value_t *collection_v = bson_get(chat, "collection");
    mongoc_collection_t *col = get_chat_message_collection(db, collection_v->value.v_int64);
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(type),
        "text", BCON_UTF8(text),
        "edited", BCON_BOOL(false),
        "from_user", BCON_UTF8(from_id),
        "chat_id", BCON_UTF8(chat_id));
    bson_add_id(doc);
    bson_add_time(doc, true);

    bson_error_t error;
    bool res = mongoc_collection_insert_one(col, doc, NULL, NULL, &error);
    if (res) {
        bson_t *users = get_participants(chat);
        bson_iter_t iter;
        if (bson_iter_init(&iter, users)) {
            bson_t *event = create_new_message_event(doc);
            while (bson_iter_next(&iter))
                user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
            bson_destroy(event);
        }
        bson_destroy(users);
    }
    bson_destroy(chat);
    mongoc_collection_destroy(col);
    return doc;
}

bson_t *edit_message(mongoc_database_t* db, const char *msg_id, const char *chat_id, const char *text, const char *cur_id) {
    bson_t *query = get_id_doc(msg_id);
    bson_t *chat = get_chat(db, chat_id);
    
    if (!chat) {
        bson_destroy(query);
        return NULL;
    }

    const bson_value_t *collection_v = bson_get(chat, "collection");
    mongoc_collection_t *col = get_chat_message_collection(db, collection_v->value.v_int64);
    bson_t *doc = get_message_from_collection(col, msg_id);

    if (!doc) {
        bson_destroy(chat);
        bson_destroy(query);
        return NULL;
    }

    string_t owner = get_msg_owner(doc);
    
    if (!owner || strcmp(owner, cur_id) != 0) {
        bson_destroy(chat);
        bson_destroy(query);
        return NULL;
    }

    bson_t *update = BCON_NEW("$set", "{", "text", BCON_UTF8(text), "edited", BCON_BOOL(true), "}");

    bson_error_t error;
    bool res = mongoc_collection_update_one(col, query, update, NULL, NULL, &error);
    if (res) {
        if (!doc) {
            bson_destroy(chat);
            bson_destroy(query);
            bson_destroy(update);
            return NULL;
        }
        bson_t *users = get_participants(chat);
        bson_iter_t iter;
        if (bson_iter_init(&iter, users)) {
            bson_t *event = create_message_updated_event(doc);
            while (bson_iter_next(&iter))
                user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
            bson_destroy(event);
        }
        bson_destroy(users);
    }

    bson_destroy(chat);
    bson_destroy(query);
    bson_destroy(update);
    mongoc_collection_destroy(col);
    return doc;
}

bool delete_message(mongoc_database_t* db, const char *msg_id, const char *chat_id, const char *cur_id) {
    bson_t *doc = get_id_doc(msg_id);
    bson_t *chat = get_chat(db, chat_id);

    if (!chat) {
        bson_destroy(doc);
        return false; 
    }

    const bson_value_t *collection_v = bson_get(chat, "collection");
    mongoc_collection_t *col = get_chat_message_collection(db, collection_v->value.v_int64);

    if (!col) {
        bson_destroy(doc);
        bson_destroy(chat);
        mongoc_collection_destroy(col);
        return false;
    }
    
    if (!is_in_chat(chat, (char *)cur_id)) {
        bson_destroy(doc);
        bson_destroy(chat);
        mongoc_collection_destroy(col);
        return false;
    }
    
    bson_t *msg = get_message_from_collection(col, msg_id);
    
    if (!msg) {
        bson_destroy(doc);
        bson_destroy(chat);
        mongoc_collection_destroy(col);
        return false; 
    }
    
    string_t admin_id = prepare_string(bson_get_str(chat, "admin"));
    string_t owner_id = prepare_string(bson_get_str(msg, "from_user"));

    if (strcmp(owner_id, cur_id) != 0) {
        if (admin_id && strcmp(admin_id, cur_id) != 0) {
            bson_destroy(doc);
            bson_destroy(chat);
            mongoc_collection_destroy(col);
            return false; 
        }
    }

    free_strings(2, &owner_id, &admin_id);

    bson_error_t error;
    bool res = mongoc_collection_delete_one(col, doc, NULL, NULL, &error);
    bson_t *users = get_participants(chat);
    if (res) {
        bson_iter_t iter;
        if (bson_iter_init(&iter, users)) {
            bson_t *event = create_message_deleted_event(chat_id, msg_id);
            while (bson_iter_next(&iter))
                user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
            bson_destroy(event);
        } 
    }
    bson_destroy(users);
    bson_destroy(doc);
    bson_destroy(chat);
    mongoc_collection_destroy(col);
    return res;
}

bson_t *get_messages(mongoc_database_t* db, int64_t collection_number, int cur_pos, int quantity)  {
    mongoc_collection_t *col = get_chat_message_collection(db, collection_number);  
    
    if (!col) {
        return NULL;
    }

    const bson_t *doc = bson_new();
    bson_t *array = bson_new();
    bson_t *filter = bson_new();
    bson_t *opts = BCON_NEW ("limit", BCON_INT64 (quantity + 1), "skip", BCON_INT64 (cur_pos), "sort", "{", "created_at", BCON_INT32(-1) ,"}");
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(col, filter, opts, NULL);
    
    int64_t msgs_count = 0; 
    int64_t limit = 0;
    while (mongoc_cursor_next (cursor, &doc)) {
        limit++;
        if (limit == quantity + 1) {
            break;
        }
        string_t pos = mx_itoa(msgs_count);
        BSON_APPEND_DOCUMENT(array, pos, doc);
        bson_destroy((bson_t *)doc);
        msgs_count++;
        mx_strdel(&pos);
    }   
    bson_t *msgs = bson_new();
    BSON_APPEND_INT64(msgs, "messages count", msgs_count);
    limit > quantity ? BSON_APPEND_BOOL(msgs, "all", false) : BSON_APPEND_BOOL(msgs, "all", true);
    BSON_APPEND_ARRAY(msgs, "messages", array);
    
    bson_destroy(array);
    bson_destroy(filter);
    bson_destroy(opts);
    mongoc_cursor_destroy (cursor);
    return msgs;
}

string_t get_msg_owner(bson_t *msg) {
    if (!msg)
        return NULL;
    string_t owner_id = prepare_string(bson_get_str(msg, "from_user"));
    return owner_id;
}
