#include "db.h"

mongoc_collection_t *get_chat_collection(mongoc_database_t *db) {
    return mongoc_database_get_collection(db, "chats");
}

bson_t *create_private_chat(mongoc_database_t* db, bson_t *participants_array) {
    mongoc_cursor_t *cursor;
    const bson_t *tmp;
    bson_t *filter = BCON_NEW("type", BCON_UTF8("private"), "participants", BCON_ARRAY(participants_array));
    mongoc_collection_t *chats_collection = get_chat_collection(db);
    cursor = mongoc_collection_find_with_opts (chats_collection, filter, NULL, NULL);
    bson_destroy(filter);
    if (mongoc_cursor_next (cursor, &tmp)) {
        bson_destroy((bson_t *)tmp);
        mongoc_cursor_destroy (cursor);
        return false;
    }
    bson_destroy((bson_t *)tmp);
    mongoc_cursor_destroy (cursor);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(CHAT_TYPE_PRIVATE),
        "collection", BCON_INT64(tv.tv_usec));
    bson_add_id(doc);
    bson_add_time(doc, true);
    bson_append_array(doc, "participants", strlen("participants"), participants_array);
    
    bson_error_t error;
    bool res = mongoc_collection_insert_one(chats_collection, doc, NULL, NULL, &error);
    mongoc_collection_destroy(chats_collection);
    if (!res) {
        bson_destroy(doc);
        return NULL;
    }

    string_t oid = bson_get_str(doc, "_id");
    bson_t *event = create_chat_added_event(doc);
    bson_iter_t iter;
    if (bson_iter_init(&iter, participants_array)) {
        while (bson_iter_next(&iter)) {
            user_add_chat(db, bson_iter_value(&iter)->value.v_utf8.str, oid);
            user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
        }
    }
    bson_destroy(event);
    return doc;
}

bson_t *create_group_chat(mongoc_database_t* db, const char *name, const char *admin_id, const char *description, bson_t *participants_array) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(CHAT_TYPE_GROUP),
        "name", BCON_UTF8(name),
        "collection", BCON_INT64(tv.tv_usec),
        "avatar_id", BCON_UTF8(""),
        "description", BCON_UTF8(description),
        "admin", BCON_UTF8(admin_id));
    bson_add_id(doc);
    bson_add_time(doc, true);
    bson_append_array(doc, "participants", strlen("participants"), participants_array);
    bson_error_t error;
    mongoc_collection_t *chats_collection = get_chat_collection(db);
    bool res = mongoc_collection_insert_one(chats_collection, doc, NULL, NULL, &error);
    mongoc_collection_destroy(chats_collection);
    if (!res) {
        bson_destroy(doc);
        return NULL;
    }
    string_t oid = bson_get_str(doc, "_id");
    bson_t *event = create_chat_added_event(doc);
    bson_iter_t iter;
    if (bson_iter_init(&iter, participants_array)) {
        while (bson_iter_next(&iter)) {
            user_add_chat(db, bson_iter_value(&iter)->value.v_utf8.str, oid);
            user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
        }
    }

    bson_destroy(event);

    return doc;
}

void notify_chat_participants(mongoc_database_t* db, bson_t *chat, bson_t *event) {
    bson_t *users = get_participants(chat);
    bson_iter_t iter;
    if (bson_iter_init(&iter, users)) {
        while (bson_iter_next(&iter))
            user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
    }
    bson_destroy(users);
}

bson_t *update_group_chat(mongoc_database_t* db, const char *chat_id, 
                        const char *name, const char *description, 
                        const char *avatar_id, const char *cur_user) {
    bson_t *query = get_id_doc(chat_id);
    BSON_APPEND_UTF8(query, "admin", cur_user);
    BSON_APPEND_UTF8(query, "type", "group");
    bson_t *update_values = bson_new();
    bson_add_time(update_values, false);
    if (name)
        BSON_APPEND_UTF8(update_values, "name", name);
    if (description)
        BSON_APPEND_UTF8(update_values, "description", description);
    if (avatar_id)
        BSON_APPEND_UTF8(update_values, "avatar_id", avatar_id);
    bson_t *update = bson_new();
    BSON_APPEND_DOCUMENT(update, "$set", update_values);
    bson_destroy(update_values);
    bson_error_t error;
    mongoc_collection_t *chats_collection = get_chat_collection(db);
    bool res = mongoc_collection_update_one(chats_collection, query, update, NULL, NULL, &error);
    bson_destroy(update);
    mongoc_collection_destroy(chats_collection);
    bson_destroy(query);

    bson_t *updated = get_chat(db, chat_id);

    if (!updated) {
        return false;
    }

    if (is_private(updated)) {
        bson_destroy(updated);
        return false;
    }

    string_t admin_id = prepare_string(bson_get_str(updated, "admin"));

    if (!admin_id) {
        bson_destroy(updated);
        return false;
    }
    
    if (strcmp(admin_id, cur_user) != 0) {
        mx_strdel(&admin_id);
        bson_destroy(updated);
        return false;
    }   
    mx_strdel(&admin_id);

    bson_t *users = get_participants(updated);
    bson_iter_t iter;
    if (res && bson_iter_init(&iter, users)) {
        bson_t *event = create_chat_changed_event(updated);
        while (bson_iter_next(&iter))
            user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
        bson_destroy(event);
    }

    bson_destroy(users);
    return updated;
}

bool group_chat_add_user(mongoc_database_t* db, const char *chat_id, const char *user_id, const char *cur_user) {    
    bson_t *query = get_id_doc(chat_id);
    bson_t *chat = get_chat(db, chat_id);
    
    if (!chat) {
        bson_destroy(query);
        return false;
    }
    
    string_t admin_id = prepare_string(bson_get_str(chat, "admin"));
    
    // Private chat doesn't have admin
    
    if (!admin_id) {
        bson_destroy(chat);
        bson_destroy(query);
        return false;
    }
    
    if (strcmp(admin_id, cur_user) != 0) {
        mx_strdel(&admin_id);
        bson_destroy(chat);
        bson_destroy(query);
        return false;
    }

    if (is_in_chat(chat, (char *)user_id)) {
        mx_strdel(&admin_id);
        bson_destroy(chat);
        bson_destroy(query);
        return false;
    }

    bson_t *update = BCON_NEW("$push", "{",
                              "participants", BCON_UTF8(user_id),
                              "}");

    bson_error_t error;
    mongoc_collection_t *chats_collection = get_chat_collection(db);
    bool res = mongoc_collection_update_one(chats_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(chats_collection);
    bson_destroy(chat);

    if (res) {
        user_add_chat(db, user_id, chat_id);
        chat = get_chat(db, chat_id);
        if (chat) {
            bson_t *event = create_chat_added_event(chat);
            user_add_event(db, user_id, event);
            bson_destroy(event);
            bson_t *users = get_participants(chat);
            bson_iter_t iter;
            if (bson_iter_init(&iter, users)) {
                bson_t *event = create_chat_changed_event(chat);
                while (bson_iter_next(&iter))
                    user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
                bson_destroy(event);
            }
            bson_destroy(users);
        }
    }
    bson_destroy(chat);
    bson_destroy(query);
    bson_destroy(update);

    return res;
}

bool group_chat_remove_user(mongoc_database_t* db, const char *chat_id, const char *user_id, const char *cur_id) {
    bson_t *query = get_id_doc(chat_id);
    bson_t *update = BCON_NEW("$pull", "{",
                              "participants", BCON_UTF8(user_id),
                              "}");
    bson_t *chat = get_chat(db, chat_id);

    if (!chat) {
        bson_destroy(update);
        bson_destroy(query);
        return false;
    }
    
    string_t admin_id = prepare_string(bson_get_str(chat, "admin"));
    
    // Private chat doesn't have admin
    if (admin_id == NULL) {
        bson_destroy(chat);
        bson_destroy(update);
        bson_destroy(query);
        return false;
    }

    if (strcmp(admin_id, cur_id) != 0) {
        mx_strdel(&admin_id);
        bson_destroy(chat);
        bson_destroy(update);
        bson_destroy(query);
        return false;
    }

    mx_strdel(&admin_id);

    bson_error_t error;
    mongoc_collection_t *chats_collection = get_chat_collection(db);
    bool res = mongoc_collection_update_one(chats_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(chats_collection);
    
    if (res) {
        user_remove_chat(db, user_id, chat_id);
        bson_t *event = create_chat_removed_event(chat_id);
        user_add_event(db, user_id, event);
        bson_destroy(event);
        
        bson_t *users = get_participants(chat);
        bson_iter_t iter;
        if (bson_iter_init(&iter, users)) {
            bson_t *event = create_chat_changed_event(chat);
            while (bson_iter_next(&iter))
                user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
            bson_destroy(event);
        }
        bson_destroy(users);
    }
    bson_destroy(chat);
    bson_destroy(query);
    bson_destroy(update);

    return res;
}

bson_t *get_chat(mongoc_database_t* db, const char *chat_id) {
    bson_t *query = get_id_doc(chat_id);
    
    mongoc_collection_t *chats_collection = get_chat_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(chats_collection, query, NULL, NULL);
    mongoc_collection_destroy(chats_collection);
    
    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    return doc;
}

bson_t *get_participants(bson_t *chat) {
    if (!chat)
        return NULL;
    const bson_value_t *participants_v = bson_get(chat, "participants");
    return bson_new_from_data(participants_v->value.v_doc.data, participants_v->value.v_doc.data_len);
}



bool delete_chat(mongoc_database_t* db, const char *chat_id, const char *member_id) {
    bson_t *doc = get_id_doc(chat_id);
    bson_t *chat = get_chat(db, chat_id);

    if (!chat) {
        bson_destroy(doc);
        return false;
    }

    // Only members can send this responce 
    if (!is_in_chat(chat, (char *)member_id)) {
        bson_destroy(chat);
        bson_destroy(doc);
        return false;
    }

    string_t admin_id = get_admin_id(chat);
    // if admin_id is NULL - chat is private, so both can delete it
    // if member id is equal to admin_id - delete the chat 
    if (admin_id != NULL) {
        if (strcmp(admin_id, member_id)) {
            mx_strdel(&admin_id);
            bson_destroy(chat);
            bson_destroy(doc);
            return false;
        }
    }

    const bson_value_t *key = bson_get(chat, "collection");

    if (key) {
        mongoc_collection_t *msg_collection = get_chat_message_collection(db, key->value.v_int64);
        if (msg_collection) {
            bson_error_t error;
            mongoc_collection_drop(msg_collection, &error);
            mongoc_collection_destroy(msg_collection);
        }
    }  

    bson_t *users = get_participants(chat);
    bson_error_t error;
    mongoc_collection_t *chats_collection = get_chat_collection(db);
    bool res = mongoc_collection_delete_one(chats_collection, doc, NULL, NULL, &error);
    mongoc_collection_destroy(chats_collection);
    bson_iter_t iter;
    if (res && bson_iter_init(&iter, users)) {
        bson_t *event = create_chat_deleted_event(chat_id);
        while (bson_iter_next(&iter)) {
            user_remove_chat(db, bson_iter_value(&iter)->value.v_utf8.str, chat_id);
            user_add_event(db, bson_iter_value(&iter)->value.v_utf8.str, event);
        }
        bson_destroy(event);
    }

    bson_destroy(doc);
    bson_destroy(users);
    bson_destroy(chat);

    return res;
}

string_t get_admin_id(bson_t *chat) {     
    if (!chat) {
        return NULL;
    }
    string_t admin_id = prepare_string(bson_get_str(chat, "admin"));
    return admin_id;
}

bool is_private(bson_t *chat) {
    if (!chat) {
        return false;
    }
    string_t type = prepare_string(bson_get_str(chat, "type"));
    return !strcmp(type, "private");
}

bool is_in_chat(bson_t *chat, string_t user_id) {
    bson_iter_t iter;
    bson_t *participants = get_participants(chat); 
    if (!participants) {
        return false;
    }
    if (bson_iter_init(&iter, participants)) {
        while (bson_iter_next(&iter)) {
            if (!strcmp(user_id, bson_iter_value(&iter)->value.v_utf8.str)) {
                bson_destroy(participants);
                return true;
            }         
        }
    }
    bson_destroy(participants);
    return false;
}
