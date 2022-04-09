#pragma once

#include "../utils/utils.h"
#include <uchat_server.h>

#define DB_NAME "ckchaodb"
#define DB_URI "mongodb+srv://chikibambony:4tQvpCKHcE9YXgx@ckchao.yhma5.mongodb.net/" DB_NAME "?readPreference=primary&ssl=true"

#define CHAT_TYPE_PRIVATE "private"
#define CHAT_TYPE_GROUP "group"

#define EVENT_TYPE_NEW_MESSAGE "new_message"
#define EVENT_TYPE_MESSAGE_DELETED "message_deleted"
#define EVENT_TYPE_MESSAGE_UPDATED "message_updated"
#define EVENT_TYPE_GROUP_ADDED "group_added"
#define EVENT_TYPE_GROUP_REMOVED "group_removed"
#define EVENT_TYPE_GROUP_CHANGED "chat_changed"
#define EVENT_TYPE_CHAT_DELETED "chat_deleted"
#define EVENT_TYPE_USER_CHANGED "user_changed"

typedef struct mongo_instance_s {
    mongoc_client_t *client;
    mongoc_database_t *db;
    mongoc_client_pool_t *pool;
} mongo_instance;

typedef bson_t *(*search_func)(mongoc_database_t *db, const char *searcher_id, const char *key, const char *data);

void init_db();

void destroy_db();

int db_ping();

mongo_instance *create_mongo_instance();

mongo_instance *get_separate_mnist();

void destroy_separate_mnist(mongo_instance *mnist);

void destroy_mongo_instance(mongo_instance *minst);

/******* USERS *******/

mongoc_collection_t *get_user_collection(mongoc_database_t *db);

bson_t *create_user(mongoc_database_t *db, const char *name, const char *email, const char *pwd_hash);

bson_t *update_user(mongoc_database_t *db, const char *user_id, const char *name,
                    const char *pwd_hash, const char *nickname,
                    const char *bio, const char *avatar_id, bool reseting_password);

bool user_confirm_email(mongoc_database_t *db, const char *user_id);

bool user_update_password(mongoc_database_t *db, const char *user_id, const char *new_password);

bool delete_user(mongoc_database_t *db, const char *user_id);

bson_t *get_user(mongoc_database_t *db, const char *user_id);

bson_t *get_user_min(mongoc_database_t *db, const char *user_id);

bool authorize_user(mongoc_database_t *db, const char *email, const char *pwd_hash);

bson_t *get_full_user(mongoc_database_t *db, const char *user_id);

bson_t *get_full_user_by_email(mongoc_database_t *db, const char *email);

bson_t *get_user_by_email(mongoc_database_t *db, const char *email);

bool user_add_chat(mongoc_database_t *db, const char *user_id, const char *chat_id);

bool user_remove_chat(mongoc_database_t *db, const char *user_id, const char *chat_id);

bool user_add_event(mongoc_database_t *db, const char *user_id, bson_t *event);

bson_t *get_user_events(mongoc_database_t *db, const char *user_id);

bool clear_user_events(mongoc_database_t *db, const char *user_id);

bson_t *collect_events(mongoc_database_t *db, const char *user_id, bool (*is_stop)(void *), void *is_stop_arg);

bson_t *search_users(mongoc_database_t *db, const char *searcher_id, const char *key, const char *data);

bson_t *search_chats(mongoc_database_t *db, const char *searcher_id, const char *key, const char *data);

bool user_allow_pswrd_reset(mongoc_database_t *db, const char *user_id);

// UNCONFIRMED USERS

mongoc_collection_t *get_unconfirmed_user_collection(mongoc_database_t *db);

bson_t *create_unconfirmed_user(mongoc_database_t *db, const char *name, const char *email, const char *pwd_hash);

bool delete_unconfirmed_user(mongoc_database_t *db, const char *user_id);

bool delete_unconfirmed_user_by_email(mongoc_database_t *db, const char *email);

bson_t *get_unconfirmed_user(mongoc_database_t *db, const char *user_id);

bson_t *get_unconfirmed_user_by_email(mongoc_database_t *db, const char *email);

bson_t *get_unconfirmed_user_and_delete(mongoc_database_t *db, const char *user_id);

bson_t *get_unconfirmed_user_by_email_and_delete(mongoc_database_t *db, const char *email);

bool extend_unconfirmed_user(mongoc_database_t *db, const char *user_id);

bool extend_unconfirmed_user_by_email(mongoc_database_t *db, const char *email);

bson_t *get_real_users(mongoc_database_t *db, bson_t *participants_array, int *count);

/******* END USERS *******/

/******* CHATS *******/

mongoc_collection_t *get_chat_collection(mongoc_database_t *db);

bson_t *create_private_chat(mongoc_database_t *db, bson_t *participants_array);

bson_t *create_group_chat(mongoc_database_t* db, const char *name, const char *admin_id, const char *description, bson_t *participants_array);

void notify_chat_participants(mongoc_database_t *db, bson_t *chat, bson_t *event);

bson_t * update_group_chat(mongoc_database_t *db, const char *chat_id,
                       const char *name, const char *description,
                       const char *avatar_id, const char *cur_user);

bool group_chat_add_user(mongoc_database_t *db, const char *chat_id, const char *user_id, const char *cur_user);

bool group_chat_remove_user(mongoc_database_t *db, const char *chat_id, const char *user_id, const char *cur_id);

bson_t *get_chat(mongoc_database_t *db, const char *chat_id);

bool delete_chat(mongoc_database_t *db, const char *chat_id, const char *member_id);

bson_t *get_participants(bson_t *chat);

string_t get_admin_id(bson_t *chat);

bool is_private(bson_t *chat);

bool is_in_chat(bson_t *chat, string_t user_id);

/******* END CHATS *******/

/******* MESSAGES *******/

mongoc_collection_t *get_chat_message_collection(mongoc_database_t *db, long int key_num);

bson_t *get_message(mongoc_database_t *db, const char *msg_id, const char *chat_id);

bson_t *get_message_from_collection(mongoc_collection_t *col, const char *msg_id);

bson_t *send_message(mongoc_database_t *db, const char *from_user_id, const char *chat_id, const char *type, const char *text);

bson_t *edit_message(mongoc_database_t *db, const char *msg_id, const char *chat_id, const char *text, const char *cur_id);

bool delete_message(mongoc_database_t *db, const char *msg_id, const char *chat_id, const char *cur_id);

string_t get_msg_owner(bson_t *msg);

bson_t *get_messages(mongoc_database_t* db, int64_t collection_number, int cur_pos, int quantity);

/******* END MESSAGES *******/

/******* EVENTS *******/

bson_t *create_new_message_event(bson_t *message);

bson_t *create_chat_added_event(bson_t *chat);

bson_t *create_chat_removed_event(const char *chat_id);

bson_t *create_chat_changed_event(bson_t *chat);

bson_t *create_chat_deleted_event(const char *chat_id);

bson_t *create_user_changed_event(bson_t *user);

bson_t *create_message_updated_event(bson_t *message);

bson_t *create_message_deleted_event(const char *chat_id, const char *msg_id);

/******* END EVENTS *******/
