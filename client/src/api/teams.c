#include "api.h"

GThread *api_create_team_async(void (*cb)(RCResponse *, void *), void *cb_data, bson_t *data) {
    GThread *res = api_send_json_async("POST", CHAT_PATH, data, cb, cb_data);
    return res;
}

GThread *api_set_team_avatar(void (*cb)(RCResponse *, void *), void *cb_data, void *data, size_t len, string_t chat_id) {
    string_t url = mx_strjoin(CHAT_PATH AVATAR_PATH ID_QUERY, chat_id);
    GThread *res = api_send_data_async("POST", url, data, len, cb, cb_data);
    mx_strdel(&url);
    mx_strdel(&chat_id);
    return res;
}

GThread *api_update_team_async(void (*cb)(RCResponse *, void *), void *cb_data, bson_t *data, const char *chat_id) {
    char path[100] = CHAT_PATH ID_QUERY;
    strcat(path, chat_id);
    GThread *res = api_send_json_async("PATCH", path, data, cb, cb_data);
    return res;
}
