#pragma once

#include <uchat_server.h>
#include "../db/db.h"

typedef struct poll_ctx_s {
    pthread_t thread;
    int connections;
    void *data;
    string_t key;
    bool stopped;
}   poll_ctx;


#define MAX_POLL_SEC 20

void init_poller();

void destroy_poller();

poll_ctx *get_poll_connection(const char *key);

bson_t *subscribe_updates_poll(const char *user_id);
