#include "form_data.h"

string_t get_form_data_boundary(const char *content_type) {
    if (!content_type)
        return NULL;
    if (!strstr(content_type, CONTENT_TYPE_multipart_form_data))
        return NULL;
    char *name_start = mx_strstr(content_type, "boundary") + 10;
    return mx_strndup(name_start, mx_get_char_index(name_start, '"'));
}

form_data_entity_t *init_form_data_entity() {
    form_data_entity_t *fde = (form_data_entity_t *)malloc(sizeof(form_data_entity_t));
    fde->headers = NULL;
    fde->body = NULL;
    fde->body_len = 0;
    return fde;
}

FormData *parse_form_data(void *body, size_t body_len, const char *boundary) {
    uchar_t *body_end = (uchar_t *)body + body_len;

    FormData *form_data = create_dict();
    uchar_t *cur = body;
    size_t b_len = strlen(boundary);

    while ((cur = (uchar_t *)mx_memmem(cur, body_end - cur, boundary, b_len))) {
        cur += b_len + 2;

        uchar_t *end = (uchar_t *)mx_memmem(cur, body_end - cur, boundary, b_len);
        if (!end)
            break;
        end -= 4;

        form_data_entity_t *fde = init_form_data_entity();

        uchar_t *headers_end = (uchar_t *)mx_strstr((const char *)cur, "\r\n\r\n") + 4;
        if (!headers_end) {
            destroy_form_data_entity(fde);
            break;
        }

        int err = 0;
        fde->headers = parse_headers((const char *)cur, headers_end - cur, &err);
        if (err) {
            destroy_form_data_entity(fde);
            break;
        }

        fde->body_len = end - headers_end;
        fde->body = malloc(fde->body_len);
        memcpy(fde->body, headers_end, fde->body_len);

        char *disposition = get_header(fde->headers, "Content-Disposition");
        char *name_start = mx_strstr(disposition, "name") + 6;
        string_t name = mx_strndup(name_start, mx_get_char_index(name_start, '"'));
        destroy_form_data_entity(dict_get(form_data, name));
        dict_set(form_data, name, fde);
        free(name);
        cur = end;
    }

    return form_data;
}

void destroy_form_data_entity(form_data_entity_t *fde) {
    if (!fde)
        return;
    if (fde->headers)
        free(fde->headers);
    if (fde->body)
        free(fde->body);
    free(fde);
}

void destroy_form_data(FormData *fd) {
    dict_destroy(fd, (dictc_cleaner)destroy_form_data_entity);
}
