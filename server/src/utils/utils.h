#pragma once

#include "../server/server.h"
#include <uchat_server.h>

const bson_value_t *bson_get(bson_t *doc, const char *key);

void bson_add_time(bson_t *doc, bool creation);

void bson_add_id(bson_t *doc);

bson_t *get_id_doc(const char *id);

string_t create_json_err(const char *code, const char *message);

char *bson_get_str(bson_t *bson, const char *key);

bool bson_get_bool(bson_t *bson, const char *key);

string_t prepare_string(const char *str);

void free_strings(int size, char **str, ...);

int64_t get_current_time();

string_t get_default_name(string_t email);

void bson_respond_err(Response *res, const char *messege, const char *status);

void bson_respond(Response *res, const char* status, bson_t *body);

void str_respond(Response *res, const char *status, const char *body, const char* type);

string_t file_2_str(const char *fname);

void print_bson(bson_t *obj);
