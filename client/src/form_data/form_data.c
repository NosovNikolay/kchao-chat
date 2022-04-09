#include "form_data.h"

string_t generate_boundary() {
    string_t boundary = mx_strnew(60);
    memset(boundary, '-', 60);
    char buff[30];
    memset(buff, 0, 30);
    struct timeval te;
    gettimeofday(&te, NULL);
    long long us = te.tv_sec * 1000000LL + te.tv_usec;
    sprintf(buff, "%lld", us);
    mx_memcpy(boundary + (60 - strlen(buff)), buff, strlen(buff));
    return boundary;
}

string_t create_boundary(const char *boundary) {
    static char *template = "--<boundary>\r\n";
    return mx_replace_substr(template, "<boundary>", boundary);
}
string_t create_end_boundary(const char *boundary) {
    static char *template = "--<boundary>--\r\n";
    return mx_replace_substr(template, "<boundary>", boundary);
}
string_t create_form_data_header(const char *boundary) {
    static char *template = "multipart/form-data;boundary=\"<boundary>\"";
    return mx_replace_substr(template, "<boundary>", boundary);
}
string_t create_form_data_entity_disposition_header(const char *name) {
    static char *template = "form-data; name=\"<name>\"";
    return mx_replace_substr(template, "<name>", name);
}

FormData *create_form_data(const char *boundary) {
    FormData *fd = (FormData *)malloc(sizeof(FormData));
    fd->boundary = mx_strdup(boundary);
    fd->data = create_dict();
    return fd;
}
void destroy_form_data(FormData *fd) {
    if (!fd)
        return;
    if (fd->boundary)
        free(fd->boundary);
    if (fd->data)
        dict_destroy(fd->data, (dictc_cleaner)destroy_form_data_entity);
    free(fd);
}

form_data_entity_t *create_form_data_entity(const char *name, void *body, size_t body_len) {
    form_data_entity_t *fde = (form_data_entity_t *)malloc(sizeof(form_data_entity_t));
    fde->name = mx_strdup(name);
    fde->body_len = body_len;
    fde->body = malloc(body_len);
    memcpy(fde->body, body, body_len);
    fde->headers = create_dict();
    dict_set(fde->headers, "Content-Disposition", create_form_data_entity_disposition_header(name));
    return fde;
}
void destroy_form_data_entity(form_data_entity_t *fde) {
    if (!fde)
        return;
    if (fde->name)
        free(fde->name);
    if (fde->body)
        free(fde->body);
    if (fde->headers)
        dict_destroy(fde->headers, (dictc_cleaner)free);
    free(fde);
}

void form_data_entity_set_header(form_data_entity_t *fde, const char *key, const char *value) {
    dict_remove(fde->headers, key, (dictc_cleaner)free);
    dict_set(fde->headers, key, mx_strdup(value));
}

char *form_data_entity_get_header(form_data_entity_t *fde, const char *key) {
    return (char *)dict_get(fde->headers, key);
}

void form_data_entity_remove_header(form_data_entity_t *fde, const char *key) {
    dict_remove(fde->headers, key, (dictc_cleaner)free);
}

void form_data_set_entity(FormData *fd, form_data_entity_t *fde) {
    dict_remove(fd->data, fde->name, (dictc_cleaner)destroy_form_data_entity);
    dict_set(fd->data, fde->name, fde);
}

void *compile_form_data(FormData *fd, size_t *data_len) {
    if (!fd)
        return NULL;

    void *data = NULL;
    size_t _data_len = 0;

    string_t boundary = create_boundary(fd->boundary);
    size_t b_len = strlen(boundary);
    for (ssize_t i = 0; i < fd->data->len; i++) {
        mx_memjoin_mutation(&data, &_data_len, boundary, b_len);
        form_data_entity_t *fde = fd->data->values[i];
        for (ssize_t j = 0; j < fde->headers->len; j++) {
            mx_memjoin_mutation(&data, &_data_len, fde->headers->keys[j], strlen(fde->headers->keys[j]));
            mx_memjoin_mutation(&data, &_data_len, ": ", 2);
            mx_memjoin_mutation(&data, &_data_len, fde->headers->values[j], strlen(fde->headers->values[j]));
            mx_memjoin_mutation(&data, &_data_len, "\r\n", 2);
        }
        mx_memjoin_mutation(&data, &_data_len, "\r\n", 2);
        mx_memjoin_mutation(&data, &_data_len, fde->body, fde->body_len);
        mx_memjoin_mutation(&data, &_data_len, "\r\n", 2);
    }
    free(boundary);
    boundary = create_end_boundary(fd->boundary);
    mx_memjoin_mutation(&data, &_data_len, boundary, strlen(boundary));
    free(boundary);

    *data_len = _data_len;
    return data;
}
