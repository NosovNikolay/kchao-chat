#include "oracle.h"

oracle_file_t *create_oracle_file() {
    oracle_file_t *file = (oracle_file_t *)malloc(sizeof(oracle_file_t));
    file->data = NULL;
    file->id = NULL;
    file->data_len = 0;
    return file;
}

void destroy_oracle_file(oracle_file_t *file) {
    if (!file)
        return;
    if (file->data)
        free(file->data);
    if (file->id)
        free(file->id);
    free(file);
}

string_t generate_name() {
    int64_t cur_time = get_current_time();
    string_t name = mx_strnew(30);
    sprintf(name, "%lx", (long)cur_time);
    return name;
}

oracle_file_t *post_file(void *data, size_t len) {
    oracle_file_t *file = create_oracle_file();
    file->id = generate_name();
    file->data = malloc(len);
    memcpy(file->data, data, len);
    file->data_len = len;

    RCHeaders *h = RC_create_headers();
    RC_set_header(h, "Host", ORACLE_HOST);

    string_t url = mx_strjoin(URL, file->id);

    RCResponse *res = RC_PUT(url, h, file->data, file->data_len);
    free(url);
    RC_destroy_headers(h);

    oracle_file_t *ret = file;

    if (!res || res->code != 200) {
        destroy_oracle_file(file);
        ret = NULL;
    }

    if (res)
        RC_response_destroy(res);

    return ret;
}

oracle_file_t *get_file(const char *id) {
    oracle_file_t *file = create_oracle_file();
    file->id = mx_strdup(id);

    RCHeaders *h = RC_create_headers();
    RC_set_header(h, "Host", ORACLE_HOST);

    string_t url = mx_strjoin(URL, file->id);

    RCResponse *res = RC_GET(url, h, NULL, 0);
    free(url);
    RC_destroy_headers(h);

    oracle_file_t *ret = file;

    if (!res || res->code != 200 || res->body_len == 0) {
        destroy_oracle_file(file);
        ret = NULL;
    } else {
        file->data = malloc(res->body_len);
        memcpy(file->data, res->body, res->body_len);
        file->data_len = res->body_len;
    }

    if (res)
        RC_response_destroy(res);

    return ret;
}

bool delete_file(const char *id) {
    RCHeaders *h = RC_create_headers();
    RC_set_header(h, "Host", ORACLE_HOST);
    RC_set_header(h, "Content-Length", "0");

    string_t url = mx_strjoin(URL, id);

    RCResponse *res = RC_PUT(url, h, NULL, 0);
    free(url);
    RC_destroy_headers(h);

    bool ret = res && res->code == 200;

    write(1, res->body, res->body_len);

    if (res)
        RC_response_destroy(res);

    return ret;
}
