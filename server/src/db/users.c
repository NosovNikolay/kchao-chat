#include "db.h"

mongoc_collection_t *get_user_collection(mongoc_database_t *db) {
    return mongoc_database_get_collection(db, "users");
}

bson_t *create_user(mongoc_database_t *db, const char *name, const char *email, const char *pwd_hash) {
    bson_t *doc = BCON_NEW(
        "name", BCON_UTF8(name),
        "email", BCON_UTF8(email),
        "pwd_hash", BCON_UTF8(pwd_hash),
        "nickname", BCON_UTF8(""),
        "bio", BCON_UTF8(""),
        "avatar_id", BCON_UTF8(""),
        "reseting_password", BCON_BOOL(false),
        "chats", "[", "]",
        "events", "[", "]");
    bson_add_id(doc);
    bson_add_time(doc, true);

    bson_error_t error;
    mongoc_collection_t *users_collection = get_user_collection(db);
    bool res = mongoc_collection_insert_one(users_collection, doc, NULL, NULL, &error);
    mongoc_collection_destroy(users_collection);

    if (res)
        return doc;
    bson_destroy(doc);
    return NULL;
}

bool user_add_event(mongoc_database_t *db, const char *user_id, bson_t *event) {
    bson_t *query = get_id_doc(user_id);

    bson_t *update = BCON_NEW("$push", "{",
                              "events", BCON_DOCUMENT(event),
                              "}");

    bson_error_t error;
    mongoc_collection_t *users_collection = get_user_collection(db);
    bool res = mongoc_collection_update_one(users_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(users_collection);

    bson_destroy(query);
    bson_destroy(update);

    return res;
}

bool user_add_chat(mongoc_database_t *db, const char *user_id, const char *chat_id) {
    bson_t *query = get_id_doc(user_id);

    bson_t *update = BCON_NEW("$push", "{",
                              "chats", BCON_UTF8(chat_id),
                              "}");

    bson_error_t error;
    mongoc_collection_t *users_collection = get_user_collection(db);
    bool res = mongoc_collection_update_one(users_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(users_collection);

    bson_destroy(query);
    bson_destroy(update);

    return res;
}

bool user_remove_chat(mongoc_database_t *db, const char *user_id, const char *chat_id) {
    bson_t *query = get_id_doc(user_id);

    bson_t *update = BCON_NEW("$pull", "{",
                              "chats", BCON_UTF8(chat_id),
                              "}");

    bson_error_t error;
    mongoc_collection_t *users_collection = get_user_collection(db);
    bool res = mongoc_collection_update_one(users_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(users_collection);

    bson_destroy(query);
    bson_destroy(update);

    return res;
}

bson_t *update_user(mongoc_database_t *db, const char *user_id, const char *name,
                    const char *pwd_hash, const char *nickname,
                    const char *bio, const char *avatar_id, bool reseting_password) {
    bson_t *query = get_id_doc(user_id);

    bson_t *update_values = bson_new();
    bson_add_time(update_values, false);
    if (name)
        BSON_APPEND_UTF8(update_values, "name", name);
    if (pwd_hash)
        BSON_APPEND_UTF8(update_values, "pwd_hash", pwd_hash);
    if (nickname)
        BSON_APPEND_UTF8(update_values, "nickname", nickname);
    if (bio)
        BSON_APPEND_UTF8(update_values, "bio", bio);
    if (avatar_id)
        BSON_APPEND_UTF8(update_values, "avatar_id", avatar_id);
    BSON_APPEND_BOOL(update_values, "reseting_password", reseting_password);
    bson_t *update = bson_new();
    BSON_APPEND_DOCUMENT(update, "$set", update_values);

    bson_error_t error;
    mongoc_collection_t *users_collection = get_user_collection(db);
    bool res = mongoc_collection_update_one(users_collection, query, update, NULL, NULL, &error);
    bson_destroy(query);
    bson_destroy(update_values);
    bson_destroy(update);
    if (!res) {
        mongoc_collection_destroy(users_collection);
        return NULL;
    }

    bson_t *pipeline = BCON_NEW(
        "pipeline", "[",
        "{", "$match", "{", "_id", BCON_UTF8(user_id), "}", "}",
        "{", "$project", "{", "pwd_hash", BCON_INT32(0), "}", "}",
        "{", "$lookup", "{",
        "from", BCON_UTF8("chats"),
        "localField", BCON_UTF8("chats"),
        "foreignField", BCON_UTF8("_id"),
        "as", BCON_UTF8("chats"),
        "}", "}",
        "{", "$addFields", "{", "chats", "{", "$filter", "{",
        "input", BCON_UTF8("$chats"),
        "as", BCON_UTF8("chat"),
        "cond", "{",
        "type", BCON_UTF8(CHAT_TYPE_PRIVATE),
        "}",
        "}", "}", "}", "}",
        "]");

    mongoc_cursor_t *cursor = mongoc_collection_aggregate(users_collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
    mongoc_collection_destroy(users_collection);
    bson_destroy(pipeline);

    const bson_t *tmp = NULL;
    if (!mongoc_cursor_next(cursor, &tmp))
        return (bson_t *)tmp;

    bson_t *user = BCON_NEW(
        "_id", BCON_UTF8(bson_get_str((bson_t *)tmp, "_id")),
        "name", BCON_UTF8(bson_get_str((bson_t *)tmp, "name")),
        "email", BCON_UTF8(bson_get_str((bson_t *)tmp, "email")),
        "nickname", BCON_UTF8(bson_get_str((bson_t *)tmp, "nickname")),
        "bio", BCON_UTF8(bson_get_str((bson_t *)tmp, "bio")),
        "avatar_id", BCON_UTF8(bson_get_str((bson_t *)tmp, "avatar_id")),
        "updated_at", BCON_INT64(bson_get((bson_t *)tmp, "updated_at")->value.v_int64),
        "created_at", BCON_INT64(bson_get((bson_t *)tmp, "created_at")->value.v_int64));

    const bson_value_t *chats_v = bson_get((bson_t *)tmp, "chats");
    bson_t *chats = bson_new_from_data(chats_v->value.v_doc.data, chats_v->value.v_doc.data_len);
    bson_iter_t iter;
    if (bson_iter_init(&iter, chats)) {
        bson_t *event = create_user_changed_event(user);
        while (bson_iter_next(&iter)) {
            const bson_value_t *chat_v = bson_iter_value(&iter);
            bson_t *chat = bson_new_from_data(chat_v->value.v_doc.data, chat_v->value.v_doc.data_len);
            notify_chat_participants(db, chat, event);
            bson_destroy(chat);
        }
        bson_destroy(event);
    }
    bson_destroy(chats);

    return user;
}

// bool user_confirm_email(mongoc_database_t *db, const char *user_id) {
//     bson_t *user = 
//     return update_user(db, user_id, NULL, NULL, NULL, NULL, NULL, false);
// }

bool user_update_password(mongoc_database_t *db, const char *user_id, const char *new_password) {
    bson_t *user = update_user(db, user_id, NULL, new_password, NULL, NULL, NULL, false);
    if (user) {
        bson_destroy(user);
        return true;
    }
    return false;
}

bool delete_user(mongoc_database_t *db, const char *user_id) {
    bson_t *doc = get_id_doc(user_id);
    if (!doc)
        return false;

    bson_error_t error;
    mongoc_collection_t *users_collection = get_user_collection(db);
    bool res = mongoc_collection_delete_one(users_collection, doc, NULL, NULL, &error);
    mongoc_collection_destroy(users_collection);

    bson_destroy(doc);
    return res;
}

bson_t *get_user(mongoc_database_t *db, const char *user_id) {
    bson_t *query = get_id_doc(user_id);
    if (!query)
        return NULL;

    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(users_collection, query, NULL, NULL);
    mongoc_collection_destroy(users_collection);

    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *get_user_min(mongoc_database_t *db, const char *user_id) {
    bson_t *query = get_id_doc(user_id);
    if (!query)
        return NULL;

    bson_t *projection = BCON_NEW(
        "projection", "{",
        "reseting_password", BCON_INT32(0),
        "chats", BCON_INT32(0),
        "events", BCON_INT32(0),
        "pwd_hash", BCON_INT32(0),
        "}");

    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(users_collection, query, projection, NULL);
    mongoc_collection_destroy(users_collection);
    bson_destroy(projection);
    bson_destroy(query);

    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    mongoc_cursor_destroy(cursor);

    return doc;
}

bool authorize_user(mongoc_database_t *db, const char *email, const char *pwd_hash) {
    bson_t *query = BCON_NEW("email", BCON_UTF8(email), "pwd_hash", BCON_UTF8(pwd_hash));

    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(users_collection, query, NULL, NULL);
    mongoc_collection_destroy(users_collection);

    const bson_t *tmp;
    bool res = false;
    if (mongoc_cursor_next(cursor, &tmp))
        res = true;

    bson_destroy((bson_t *)tmp);
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return res;
}

bson_t *get_full_user(mongoc_database_t *db, const char *user_id) {
    bson_t *pipeline = BCON_NEW(
        "pipeline", "[",
        "{", "$match", "{", "_id", BCON_UTF8(user_id), "}", "}",
        "{", "$project", "{", "pwd_hash", BCON_INT32(0), "}", "}",
        "{", "$lookup", "{",
        "from", BCON_UTF8("chats"),
        "localField", BCON_UTF8("chats"),
        "foreignField", BCON_UTF8("_id"),
        "as", BCON_UTF8("chats"),
        "}", "}",
        "]");

    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_aggregate(users_collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
    mongoc_collection_destroy(users_collection);

    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);
    
    const bson_value_t *chats = bson_get(doc, "chats");
    bson_t *chats_arrray = bson_new_from_data(chats->value.v_doc.data, chats->value.v_doc.data_len);
    bson_iter_t iter;
    int chats_count = 0;
    if (bson_iter_init(&iter, chats_arrray)) {
        while (bson_iter_next(&iter)) { 
            chats_count++;
        }
    }
    bson_destroy(chats_arrray);
    BSON_APPEND_INT32(doc, "chats_count", chats_count);
    bson_destroy(pipeline);
    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *get_full_user_by_email(mongoc_database_t *db, const char *email) {
    bson_t *pipeline = BCON_NEW(
        "pipeline", "[",
        "{", "$match", "{", "email", BCON_UTF8(email), "}", "}",
        "{", "$project", "{", "pwd_hash", BCON_INT32(0), "}", "}",
        "{", "$lookup", "{",
        "from", BCON_UTF8("chats"),
        "localField", BCON_UTF8("chats"),
        "foreignField", BCON_UTF8("_id"),
        "as", BCON_UTF8("chats"),
        "}", "}",
        "]");

    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_aggregate(users_collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
    mongoc_collection_destroy(users_collection);

    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    bson_destroy(pipeline);
    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *get_user_by_email(mongoc_database_t *db, const char *email) {
    if (!email)
        return NULL;
    bson_t *query = BCON_NEW("email", BCON_UTF8(email));

    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(users_collection, query, NULL, NULL);
    mongoc_collection_destroy(users_collection);

    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *get_user_events(mongoc_database_t *db, const char *user_id) {
    bson_t *user = get_user(db, user_id);
    const bson_value_t *events_v = bson_get(user, "events");
    bson_t *events = bson_new_from_data(events_v->value.v_doc.data, events_v->value.v_doc.data_len);
    bson_destroy(user);
    return events;
}

bool clear_user_events(mongoc_database_t *db, const char *user_id) {
    bson_t *query = get_id_doc(user_id);
    bson_t *update = BCON_NEW("$set", "{", "events", "[", "]", "}");

    bson_error_t error;
    mongoc_collection_t *users_collection = get_user_collection(db);
    bool res = mongoc_collection_update_one(users_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(users_collection);

    bson_destroy(query);
    bson_destroy(update);
    return res;
}

bson_t *collect_events(mongoc_database_t *db, const char *user_id, bool (*is_stop)(void *), void *is_stop_arg) {
    bson_t *user, *events;
    while ((user = get_user(db, user_id))) {
        const bson_value_t *events_v = bson_get(user, "events");
        events = bson_new_from_data(events_v->value.v_doc.data, events_v->value.v_doc.data_len);
        if (bson_count_keys(events))
            break;
        bson_destroy(user);
        bson_destroy(events);
        user = NULL;
        if (is_stop(is_stop_arg))
            break;
        sleep(1);
    }
    if (!user)
        return NULL;

    bson_t *events_obj = BCON_NEW("events", BCON_ARRAY(events));
    bson_destroy(events);

    bson_t *query = get_id_doc(user_id);
    bson_t *update = BCON_NEW("$set", "{", "events", "[", "]", "}");

    bson_error_t error;
    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_collection_update_one(users_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(users_collection);

    bson_destroy(query);
    bson_destroy(update);

    return events_obj;
}

bool already_added(bson_t *real_users, const char *id) {
    bson_iter_t iter;
    if (bson_iter_init(&iter, real_users) && id) {
        while (bson_iter_next(&iter)) {
            if (strcmp(bson_iter_value(&iter)->value.v_utf8.str, id) == 0)
                return true;
        }
    }
    return false;
}

bson_t *get_real_users(mongoc_database_t *db, bson_t *participants_array, int *count) {
    bson_iter_t iter;
    bson_t *real_users = bson_new();
    if (bson_iter_init(&iter, participants_array)) {
        while (bson_iter_next(&iter)) {
            if (!already_added(real_users, bson_iter_value(&iter)->value.v_utf8.str)) {
                bson_t *user = get_user(db, bson_iter_value(&iter)->value.v_utf8.str);
                if (user) {
                    string_t cur_pos = mx_itoa(*count);
                    BSON_APPEND_UTF8(real_users, cur_pos, bson_iter_value(&iter)->value.v_utf8.str);
                    mx_strdel(&cur_pos);
                    bson_destroy(user);
                    *count += 1;
                }
            }
        }
    }
    return real_users;
}

bson_t *search_chats(mongoc_database_t *db, const char *searcher_id, const char *key, const char *data) {
    bson_t *filter = NULL;
    string_t regex_pattern = mx_replace_substr("{data}", "{data}", data);
    string_t r_key = mx_strjoin("$$r.", key);
    if (key) {
        filter = BCON_NEW(
            "$regexMatch", "{", 
                "input", BCON_UTF8(r_key),
                "regex", BCON_REGEX(regex_pattern, "i"),
            "}");
    } else {
        filter = BCON_NEW(
            "$or", "[", 
            "{", "$regexMatch", "{", 
                "input", BCON_UTF8("$$r.type"),
                "regex", BCON_REGEX(regex_pattern, "i"),
            "}", "}",
            "{", "$regexMatch", "{", 
                "input", BCON_UTF8("$$r.name"),
                "regex", BCON_REGEX(regex_pattern, "i"),
            "}", "}",
            "]");
    }
    bson_t *pipeline = BCON_NEW(
        "pipeline", "[",
        "{", "$match", "{", "_id", BCON_UTF8(searcher_id), "}", "}",
        "{", "$lookup", "{",
            "from", BCON_UTF8("chats"),
            "localField", BCON_UTF8("chats"),
            "foreignField", BCON_UTF8("_id"),
            "as", BCON_UTF8("results"),
        "}", "}",
        "{", "$project", "{", "results", "{", 
            "$filter", "{", 
                "input", "$results",
                "as", "r",
                "cond", BCON_DOCUMENT(filter),
            "}",
        "}", "_id", BCON_INT32(0), "}", "}",
        "{", "$project",  "{", "results", "{", "$slice", "[", "$results", BCON_INT32(10), "]", "}", "}", "}",
        "]");

    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_aggregate(users_collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
    mongoc_collection_destroy(users_collection);

    free(regex_pattern);
    bson_destroy(filter);
    bson_destroy(pipeline);
    free(r_key);

    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *search_users(mongoc_database_t *db, const char *searcher_id, const char *key, const char *data) {
    bson_t *filter = NULL;
    string_t regex_pattern = mx_replace_substr("{data}", "{data}", data);
    if (key) {
        filter = BCON_NEW(
            "$and", "[",
            "{", key, BCON_REGEX(regex_pattern, "i"), "}",
            "{", "_id", "{", "$ne", BCON_UTF8(searcher_id), "}", "}",
            "]");
    } else {
        filter = BCON_NEW(
            "$and", "[",
                "{", "$or", "[",
                    "{", "nickname", BCON_REGEX(regex_pattern, "i"), "}",
                    "{", "name", BCON_REGEX(regex_pattern, "i"), "}",
                    "{", "email", BCON_REGEX(regex_pattern, "i"), "}",
                "]", "}",
                "{", "_id", "{", "$ne", BCON_UTF8(searcher_id), "}", "}",
            "]");
    }

    bson_t *options = BCON_NEW(
        "projection", "{",
            "reseting_password", BCON_INT32(0),
            "chats", BCON_INT32(0),
            "events", BCON_INT32(0),
            "pwd_hash", BCON_INT32(0),
        "}",
        "limit", BCON_INT64(10));

    mongoc_collection_t *users_collection = get_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(users_collection, filter, options, NULL);
    mongoc_collection_destroy(users_collection);
    free(regex_pattern);
    bson_destroy(filter);
    bson_destroy(options);

    bson_t arr;
    const bson_t *tmp;
    bson_t *ret = bson_new();
    BSON_APPEND_ARRAY_BEGIN(ret, "results", &arr);
    size_t i = 0;
    while (mongoc_cursor_next(cursor, &tmp)) {
        char buf[30];
        const char *key;
        int keylen = bson_uint32_to_string(i++, &key, buf, sizeof buf);
        bson_append_document(&arr, key, keylen, tmp);
    }
    bson_append_array_end(ret, &arr);

    mongoc_cursor_destroy(cursor);
    return ret;
}


bool user_allow_pswrd_reset(mongoc_database_t *db, const char *user_id) {
    bson_t *user = update_user(db, user_id, NULL, NULL, NULL, NULL, NULL, true);
    if (user) {
        bson_destroy(user);
        return true;
    }
    return false;
}
