#include "db.h"

mongoc_collection_t *get_unconfirmed_user_collection(mongoc_database_t *db) {
    return mongoc_database_get_collection(db, "unconfirmed_users");
}

bson_t *create_unconfirmed_user(mongoc_database_t* db, const char *name, const char *email, const char *pwd_hash) {
    int64_t t = get_current_time();
    bson_t *doc = BCON_NEW(
        "name", BCON_UTF8(name),
        "email", BCON_UTF8(email),
        "pwd_hash", BCON_UTF8(pwd_hash),
        "created_at", BCON_DATE_TIME(t),
        "updated_at", BCON_DATE_TIME(t));
    bson_add_id(doc);

    bson_error_t error;
    mongoc_collection_t *uusers_collection = get_unconfirmed_user_collection(db);
    bool res = mongoc_collection_insert_one(uusers_collection, doc, NULL, NULL, &error);
    mongoc_collection_destroy(uusers_collection);

    if (res)
        return doc;
    bson_destroy(doc);

    return NULL;
}

bool delete_unconfirmed_user(mongoc_database_t* db, const char *user_id) {
    bson_t *doc = get_id_doc(user_id);

    bson_error_t error;
    mongoc_collection_t *uusers_collection = get_unconfirmed_user_collection(db);
    bool res = mongoc_collection_delete_one(uusers_collection, doc, NULL, NULL, &error);
    mongoc_collection_destroy(uusers_collection);

    bson_destroy(doc);
    return res;
}

bool delete_unconfirmed_user_by_email(mongoc_database_t* db, const char *email) {
    bson_t *doc = BCON_NEW("email", BCON_UTF8(email));

    bson_error_t error;
    mongoc_collection_t *uusers_collection = get_unconfirmed_user_collection(db);
    bool res = mongoc_collection_delete_one(uusers_collection, doc, NULL, NULL, &error);
    mongoc_collection_destroy(uusers_collection);

    bson_destroy(doc);
    return res;
}

bson_t *get_unconfirmed_user(mongoc_database_t* db, const char *user_id) {
    bson_t *query = get_id_doc(user_id);

    mongoc_collection_t *uusers_collection = get_unconfirmed_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(uusers_collection, query, NULL, NULL);
    mongoc_collection_destroy(uusers_collection);

    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *get_unconfirmed_user_by_email(mongoc_database_t* db, const char *email) {
    bson_t *query = BCON_NEW("email", BCON_UTF8(email));

    mongoc_collection_t *uusers_collection = get_unconfirmed_user_collection(db);
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(uusers_collection, query, NULL, NULL);
    mongoc_collection_destroy(uusers_collection);

    const bson_t *tmp;
    bson_t *doc = NULL;
    if (mongoc_cursor_next(cursor, &tmp))
        doc = bson_copy(tmp);

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return doc;
}

bson_t *get_unconfirmed_user_and_delete(mongoc_database_t* db, const char *user_id) {
    bson_t *user = get_unconfirmed_user(db, user_id);
    delete_unconfirmed_user(db, user_id);
    return user;
}

bson_t *get_unconfirmed_user_by_email_and_delete(mongoc_database_t* db, const char *email) {
    bson_t *user = get_unconfirmed_user_by_email(db, email);
    delete_unconfirmed_user_by_email(db, email);
    return user;
}

bool extend_unconfirmed_user(mongoc_database_t* db, const char *user_id) {
    bson_t *query = get_id_doc(user_id);

    int64_t t = get_current_time();
    bson_t *update = BCON_NEW(
        "$set", "{",
        "updated_at", BCON_DATE_TIME(t),
        "}");

    bson_error_t error;
    mongoc_collection_t *uusers_collection = get_unconfirmed_user_collection(db);
    bool res = mongoc_collection_update_one(uusers_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(uusers_collection);

    bson_destroy(query);
    bson_destroy(update);
    return res;
}

bool extend_unconfirmed_user_by_email(mongoc_database_t* db, const char *email) {
    bson_t *query = BCON_NEW("email", BCON_UTF8(email));

    int64_t t = get_current_time();
    bson_t *update = BCON_NEW(
        "$set", "{",
        "updated_at", BCON_DATE_TIME(t),
        "}");

    bson_error_t error;
    mongoc_collection_t *uusers_collection = get_unconfirmed_user_collection(db);
    bool res = mongoc_collection_update_one(uusers_collection, query, update, NULL, NULL, &error);
    mongoc_collection_destroy(uusers_collection);

    bson_destroy(query);
    bson_destroy(update);
    return res;
}
