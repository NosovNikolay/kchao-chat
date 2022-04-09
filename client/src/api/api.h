#pragma once

#include <uchat.h>

#include "../async/async.h"

#define API_URL "https://ckchao.herokuapp.com"

#define AUTH_PATH "/auth"

#define USER_PATH "/user"

#define AVATAR_PATH "/avatar"

#define USER_ID_PATH "/user/id?user_id="

#define CHAT_PATH "/chat"

#define CHAT_MEMBER_PATH CHAT_PATH "/member"

#define FILE_PATH "/file?file_id="

#define UPDATE_PATH "/updates"

#define GET_CHAT_PATH "/message?chat_id="

#define SEND_MSG_PATH "/message/sendText"

#define SEARCH_PATH "/search"
#define SEARCH_TEMPLATE SEARCH_PATH "?data=<data>"
#define SEARCH_USERS_TEMPLATE SEARCH_PATH "?data=u:<data>"
#define SEARCH_GROUPS_TEMPLATE SEARCH_PATH "?data=c:<data>"

#define ID_QUERY "?id="

typedef struct async_json_ctx_s {
    char *method;
    char *path;
    bson_t *obj;
} async_json_ctx;

typedef struct async_data_ctx_s {
    char *method;
    char *path;
    void *data;
    size_t len;
} async_data_ctx;

void init_api(const char *port, const char *auth_token);

void destroy_api();

char *get_hostname();

void api_authorize(const char *auth_token);

string_t create_url(const char *path);

RCHeaders *get_default_headers();

RCResponse *api_send_json(const char *method, const char *path, bson_t *obj);

GThread *api_send_json_async(const char *method, const char *path, bson_t *obj, void (*cb)(RCResponse *, void *), void *cb_data);

string_t create_url(const char *path);

RCResponse *api_login(const char *email, const char *password);
GThread *api_login_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *email, const char *password);

RCResponse *api_register(const char *name, const char *email, const char *password);
GThread *api_register_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *name, const char *email, const char *password);

RCResponse *api_send_email_confirmation(const char *email);
GThread *api_send_email_confirmation_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *email);

GThread *api_send_email_reset_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *email);

GThread *api_get_user_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *id);

RCResponse *api_get_updates();
GThread *api_get_updates_async(void (*cb)(RCResponse *, void *), void *cb_data);

RCResponse *api_get_messages(const char *chat_id, int cur, int quantity);

GThread *api_get_messages_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *chat_id, int cur, int quantity);

GThread *api_send_message_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *text, const char *type, const char *chat_id);

RCResponse *api_get_user_by_id(const char *id);

GThread *api_get_file_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *file_id);

GThread *api_get_user_by_id_asynk(void (*cb)(RCResponse *, void *), void *cb_data, const char *id);

RCResponse *api_update_user(const char *field, const char *value);
GThread *api_update_user_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *field, const char *value);

GThread *api_update_user_async_new(void (*cb)(RCResponse *, void *), void *cb_data, bson_t *data);

RCResponse *api_send_data(const char *method, const char *path, void *data, size_t len);
GThread *api_send_data_async(const char *method, const char *path, void *data, size_t len, void (*cb)(RCResponse *, void *), void *cb_data);

RCResponse *api_send_data(const char *method, const char *path, void *data, size_t len);

GThread *api_update_user_avatar(void (*cb)(RCResponse *, void *), void *cb_data, void *data, size_t len);

GThread *api_create_team_async(void (*cb)(RCResponse *, void *), void *cb_data, bson_t *data);
GThread *api_set_team_avatar(void (*cb)(RCResponse *, void *), void *cb_data, void *data, size_t len, string_t chat_id);
GThread *api_update_team_async(void (*cb)(RCResponse *, void *), void *cb_data, bson_t *data, const char *chat_id);

GThread *api_search_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *search);
GThread *api_search_users_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *search);
GThread *api_search_groups_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *search);
