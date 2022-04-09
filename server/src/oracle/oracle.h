#pragma once

#include <requests-c.h>
#include <uchat_server.h>

#include "../utils/utils.h"

#define KEY_NAME "ckchao_api"

#define ORACLE_HOST "objectstorage.eu-frankfurt-1.oraclecloud.com"
#define URL "https://" ORACLE_HOST "/p/Vr-CiqHJM4zVwjTwJagtTNUqDj_RLo7cUVsiyywkXrB7AvQbRa-Lv86F3iKEV6zT/n/frhtpmyrzqje/b/ckchao_files/o/"

typedef struct oracle_file_s {
    string_t id;
    void *data;
    size_t data_len;
} oracle_file_t;

oracle_file_t *create_oracle_file();

void destroy_oracle_file(oracle_file_t *file);

string_t generate_name();

oracle_file_t *post_file(void *data, size_t len);

oracle_file_t *get_file(const char *id);

bool delete_file(const char *id);
