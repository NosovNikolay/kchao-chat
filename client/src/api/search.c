#include "api.h"

GThread *api_search_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *search) {
    string_t path = mx_replace_substr(SEARCH_TEMPLATE, "<data>", search);
    GThread *res = api_send_json_async("GET", path, NULL, cb, cb_data);
    free(path);
    return res;
}

GThread *api_search_users_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *search) {
    string_t path = mx_replace_substr(SEARCH_USERS_TEMPLATE, "<data>", search);
    GThread *res = api_send_json_async("GET", path, NULL, cb, cb_data);
    free(path);
    return res;
}

GThread *api_search_groups_async(void (*cb)(RCResponse *, void *), void *cb_data, const char *search) {
    string_t path = mx_replace_substr(SEARCH_GROUPS_TEMPLATE, "<data>", search);
    GThread *res = api_send_json_async("GET", path, NULL, cb, cb_data);
    free(path);
    return res;
}
