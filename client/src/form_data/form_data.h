#pragma once

#include <dict.h>
#include <libmx.h>
#include <sys/time.h>

typedef struct form_data_entity_s {
    string_t name;
    Dict *headers;
    void *body;
    size_t body_len;
} form_data_entity_t;

typedef struct form_data_s {
    string_t boundary;
    Dict *data;
} FormData;

string_t generate_boundary();
string_t create_boundary(const char *boundary);
string_t create_end_boundary(const char *boundary);
string_t create_form_data_header(const char *boundary);
string_t create_form_data_entity_disposition_header(const char *name);

FormData *create_form_data(const char *boundary);
void destroy_form_data(FormData *fd);

form_data_entity_t *create_form_data_entity(const char *name, void *body, size_t body_len);
void destroy_form_data_entity(form_data_entity_t *fde);

void form_data_entity_set_header(form_data_entity_t *fde, const char *key, const char *value);

char *form_data_entity_get_header(form_data_entity_t *fde, const char *key);

void form_data_entity_remove_header(form_data_entity_t *fde, const char *key);

void form_data_set_entity(FormData *fd, form_data_entity_t *fde);

void *compile_form_data(FormData *fd, size_t *data_len);
