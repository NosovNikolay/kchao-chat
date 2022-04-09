#include "db.h"

bson_t *create_new_message_event(bson_t *message) {
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(EVENT_TYPE_NEW_MESSAGE),
        "message", BCON_DOCUMENT(message));
    bson_add_id(doc);
    bson_add_time(doc, true);
    return doc;
}

bson_t *create_message_updated_event(bson_t *message) {
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(EVENT_TYPE_MESSAGE_UPDATED),
        "message", BCON_DOCUMENT(message));
    bson_add_id(doc);
    bson_add_time(doc, true);
    return doc;
}

bson_t *create_message_deleted_event(const char *chat_id, const char *msg_id) {
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(EVENT_TYPE_MESSAGE_DELETED),
        "message", BCON_UTF8(msg_id),
        "chat", BCON_UTF8(chat_id));
    bson_add_id(doc);
    bson_add_time(doc, true);
    return doc;
}

bson_t *create_chat_added_event(bson_t *chat) {
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(EVENT_TYPE_GROUP_ADDED),
        "chat", BCON_DOCUMENT(chat));
    bson_add_id(doc);
    bson_add_time(doc, true);
    return doc;
}

bson_t *create_chat_removed_event(const char *chat_id) {
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(EVENT_TYPE_GROUP_REMOVED),
        "chat", BCON_UTF8(chat_id));
    bson_add_id(doc);
    bson_add_time(doc, true);
    return doc;
}

bson_t *create_chat_deleted_event(const char *chat_id) {
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(EVENT_TYPE_CHAT_DELETED),
        "chat", BCON_UTF8(chat_id));
    bson_add_id(doc);
    bson_add_time(doc, true);
    return doc;
}

bson_t *create_chat_changed_event(bson_t *chat) {
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(EVENT_TYPE_GROUP_CHANGED),
        "chat", BCON_DOCUMENT(chat));
    bson_add_id(doc);
    bson_add_time(doc, true);
    return doc;
}

bson_t *create_user_changed_event(bson_t *user) {
    bson_t *doc = BCON_NEW(
        "type", BCON_UTF8(EVENT_TYPE_USER_CHANGED),
        "user", BCON_DOCUMENT(user));
    bson_add_id(doc);
    bson_add_time(doc, true);
    return doc;
}
