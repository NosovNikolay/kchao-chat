#include "api.h"

RCResponse *api_login(const char *email, const char *password) {
    bson_t *credentials = BCON_NEW(
        "email", BCON_UTF8(email),
        "password", BCON_UTF8(password));
    RCResponse *res = api_send_json("POST", AUTH_PATH "/login", credentials);
    bson_destroy(credentials);
    return res;
}

GThread *api_login_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *email, const char *password) {
    bson_t *credentials = BCON_NEW(
        "email", BCON_UTF8(email),
        "password", BCON_UTF8(password));
    GThread *res = api_send_json_async("POST", AUTH_PATH "/login", credentials, cb, cb_data);
    bson_destroy(credentials);
    return res;
}

RCResponse *api_register(const char *name, const char *email, const char *password) {
    bson_t *user = BCON_NEW(
        "name", BCON_UTF8(name),
        "email", BCON_UTF8(email),
        "password", BCON_UTF8(password));
    RCResponse *res = api_send_json("POST", AUTH_PATH "/register", user);
    bson_destroy(user);
    return res;
}

GThread *api_register_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *name, const char *email, const char *password) {
    bson_t *user = BCON_NEW(
        "name", BCON_UTF8(name),
        "email", BCON_UTF8(email),
        "password", BCON_UTF8(password));
    GThread *res = api_send_json_async("POST", AUTH_PATH "/register", user, cb, cb_data);
    bson_destroy(user);
    return res;
}

RCResponse *api_send_email_confirmation(const char *email) {
    bson_t *email_bson = BCON_NEW("email", BCON_UTF8(email));
    RCResponse *res = api_send_json("POST", AUTH_PATH "/sendConfirmationEmail", email_bson);
    bson_destroy(email_bson);
    return res;
}

GThread *api_send_email_confirmation_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *email) {
    bson_t *email_bson = BCON_NEW("email", BCON_UTF8(email));
    GThread *res = api_send_json_async("POST", AUTH_PATH "/sendConfirmationEmail", email_bson, cb, cb_data);
    bson_destroy(email_bson);
    return res;
}

GThread *api_send_email_reset_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *email) {
    bson_t *email_bson = BCON_NEW("email", BCON_UTF8(email));
    GThread *res = api_send_json_async("POST", AUTH_PATH "/sendResetPassword", email_bson, cb, cb_data);
    bson_destroy(email_bson);
    return res;
}

GThread *api_get_user_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *id) {
    bson_t *user_id = BCON_NEW("id", BCON_UTF8(id));
    GThread *res = api_send_json_async("GET", USER_PATH, user_id, cb, cb_data);
    bson_destroy(user_id);
    return res;
}

RCResponse *api_get_updates() {
    return api_send_json("GET", UPDATE_PATH, NULL);
}

GThread *api_get_updates_async(void (*cb)(RCResponse *, void *), void *cb_data) {
    GThread *res = api_send_json_async("GET", UPDATE_PATH, NULL, cb, cb_data);
    return res;
}

RCResponse *api_get_messages(const char *chat_id, int cur, int quantity) {
    char path[512];
    sprintf(path, "/message?chat_id=%s&cur=%d&quantity=%d", chat_id, cur, quantity);
    RCResponse *res = api_send_json("GET", path, NULL);
    return res;
}


GThread *api_get_messages_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *chat_id, int cur, int quantity) {
    char path[512];
    sprintf(path, "/message?chat_id=%s&cur=%d&quantity=%d", chat_id, cur, quantity);
    GThread *res = api_send_json_async("GET", path, NULL, cb, cb_data);
    return res;
}

GThread *api_send_message_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *text, const char *type, const char *chat_id) {
    bson_t *message_bson = BCON_NEW(
        "text", BCON_UTF8(text),
        "type", BCON_UTF8(type),
        "chat_id", BCON_UTF8(chat_id));
    GThread *res = api_send_json_async("POST", SEND_MSG_PATH, message_bson, cb, cb_data);
    bson_destroy(message_bson);
    return res;
}

GThread *api_get_file_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *file_id) {
    const char *path = mx_strjoin(FILE_PATH, file_id);
    GThread *res = api_send_json_async("GET", path, NULL, cb, cb_data);
    return res;
    free((void *)path);
}

GThread *api_get_user_by_id_asynk(void (*cb)(RCResponse *, void *), void *cb_data, const char *id) {
    char *path = mx_strjoin(USER_ID_PATH, id);
    GThread *res = api_send_json_async("GET", path, NULL, cb, cb_data);
    mx_strdel(&path);
    return res;
}

RCResponse *api_get_user_by_id(const char *id) {
    char *path = mx_strjoin(USER_ID_PATH, id);
    RCResponse *res = api_send_json("GET", path, NULL);
    mx_strdel(&path);
    return res;
}
