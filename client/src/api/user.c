#include "api.h"

RCResponse *api_update_user(const char *field, const char *value) {
    bson_t *data = BCON_NEW(field, BCON_UTF8(value));
    RCResponse *res = api_send_json("PATCH", USER_PATH, data);
    bson_destroy(data);
    return res;
}

GThread *api_update_user_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *field, const char *value) {
    bson_t *data = BCON_NEW(field, BCON_UTF8(value));
    GThread *res = api_send_json_async("PATCH", USER_PATH, data, cb, cb_data);
    bson_destroy(data);
    return res;
}

GThread *api_update_user_async_new(void (*cb)(RCResponse *, void *), void *cb_data, bson_t *data) {
    GThread *res = api_send_json_async("PATCH", USER_PATH, data, cb, cb_data);
    return res;
}

GThread *api_update_user_avatar(void (*cb)(RCResponse *, void *), void *cb_data, void *data, size_t len) {
    GThread *res = api_send_data_async("POST", USER_PATH AVATAR_PATH, data, len, cb, cb_data);
    return res;
}
