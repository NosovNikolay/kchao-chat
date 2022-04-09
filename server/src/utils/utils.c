#include "utils.h"

string_t create_json_err(const char *code, const char *message) {
    string_t message_utf8 = bson_utf8_escape_for_json(message, strlen(message));
    string_t tmp = mx_replace_substr(ERROR_JSON_TEMPLATE, "<code>", code);
    string_t res = mx_replace_substr(tmp, "<message>", message_utf8);
    free(tmp);
    free(message_utf8);
    return res;
}

const bson_value_t *bson_get(bson_t *obj, const char *key) {
    bson_iter_t iter, res;
    if (bson_iter_init(&iter, obj) && bson_iter_find_descendant(&iter, key, &res))
        return bson_iter_value(&res);
    return NULL;
}

void bson_add_time(bson_t *doc, bool creation) {
    int64_t time = get_current_time();
    BSON_APPEND_INT64(doc, "updated_at", time);
    if (creation)
        BSON_APPEND_INT64(doc, "created_at", time);
}

void bson_add_id(bson_t *doc) {
    bson_oid_t oid;
    bson_oid_init(&oid, NULL);
    char buff[25];
    bson_oid_to_string(&oid, buff);
    BSON_APPEND_UTF8(doc, "_id", buff);
}

bson_t *get_id_doc(const char *id) {
    if (!id)
        return NULL;
    return BCON_NEW("_id", BCON_UTF8(id));
}

char *bson_get_str(bson_t *bson, const char *key) {
    bson_iter_t bson_iter, res_iter;
    bson_iter_init(&bson_iter, bson);
    if (bson_iter_find_descendant(&bson_iter, key, &res_iter) &&
        BSON_ITER_HOLDS_UTF8(&res_iter)) {
        uint32_t name_size;
        return (char *)bson_iter_utf8(&res_iter, &name_size);
    }
    return NULL;
}

bool bson_get_bool(bson_t *bson, const char *key) {
    bson_iter_t bson_iter, res_iter;
    bson_iter_init(&bson_iter, bson);
    if (bson_iter_find_descendant(&bson_iter, key, &res_iter) && BSON_ITER_HOLDS_BOOL(&res_iter))
        return (bool)bson_iter_bool(&res_iter);
    return NULL;
}

string_t prepare_string(const char *str) {
    if (!str)
        return NULL;
    string_t result = mx_strtrim(str);
    if (!result || !strlen(result))
        mx_strdel(&result);
    return result;
}

void free_strings(int size, char **str, ...) {
    va_list ap;
    char **i;
    va_start(ap, str);
    int j = 0;
    for (i = str; j < size; i = va_arg(ap, char **), j++) {
        if (i) {
            mx_strdel(i);
            i = NULL;
        }
    }
    va_end(ap);
}

int64_t get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
}

string_t get_default_name(string_t email) {
    if (!email)
        return NULL;
    size_t i = 0;
    while (i < strlen(email)) {
        if (email[i] == '@') {
            return mx_strndup(email, i);
        }
        i++;
    }
    return NULL;
}

void bson_respond_err(Response *res, const char *messege, const char *status) {
    set_header(res->headers, HEADER_CONTENT_TYPE, CONTENT_TYPE_application_json);
    string_t *tmp = mx_strdivide(messege, ' ');
    res->body = create_json_err(tmp[0], tmp[1]);
    res->body_len = strlen(res->body);
    res->status = (char *)status;
    mx_del_strarr(&tmp);
}

void bson_respond(Response *res, const char *status, bson_t *body) {
    set_header(res->headers, HEADER_CONTENT_TYPE, CONTENT_TYPE_application_json);
    res->body = bson_as_relaxed_extended_json(body, &res->body_len);
    res->status = (char *)status;
}

void str_respond(Response *res, const char *status, const char *body, const char *type) {
    set_header(res->headers, HEADER_CONTENT_TYPE, type ? type : CONTENT_TYPE_text_plain);
    res->body = mx_strdup(body);
    res->body_len = strlen(res->body);
    res->status = (char *)status;
}

string_t file_2_str(const char *fname) {
    string_t buffer = NULL;
    size_t length;
    FILE *f = fopen(fname, "r");
    if (!f)
        return buffer;
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (string_t)malloc(length + 1);
    memset(buffer, 0, length + 1);
    if (buffer) {
        fread(buffer, 1, length, f);
    }
    fclose(f);
    return buffer;
}

void print_bson(bson_t *obj) {
    size_t l;
    string_t json = bson_as_relaxed_extended_json(obj, &l);
    mx_printstr(json);
    mx_printchar('\n');
    free(json);
}
