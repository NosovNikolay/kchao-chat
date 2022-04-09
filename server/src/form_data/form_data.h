#pragma once

#include <dict.h>
#include <libmx.h>
#include <string.h>
#include "../server/headers.h"
#include "../server/HTTPTypes.h"
#include "../utils/utils.h"


typedef struct form_data_entity_s {
    Headers *headers;
    void *body;
    size_t body_len;
} form_data_entity_t;

typedef Dict FormData;

string_t get_form_data_boundary(const char *content_type);

form_data_entity_t* init_form_data_entity();

FormData *parse_form_data(void *body, size_t body_len, const char *boundary);

void destroy_form_data_entity(form_data_entity_t *fde);

void destroy_form_data(FormData *fd);
