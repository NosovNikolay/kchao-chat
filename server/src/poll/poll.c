#include "poll.h"

static Dict *_connections = NULL;

void init_poller() {
    _connections = create_dict();
}

static void destroy_poll_connection(poll_ctx *conn) {
    string_t key = conn->key;
    dict_remove(_connections, key, free);
    free(key);
}

static void free_poll_connection(void *ptr) {
    poll_ctx *conn = (poll_ctx *)ptr;
    free(conn->key);
    free(conn);
}

void destroy_poller() {
    dict_destroy(_connections, free_poll_connection);
}

poll_ctx *get_poll_connection(const char *key) {
    return (poll_ctx *)dict_get(_connections, key);
}

static void *update_poll_provider(void *ptr) {
    poll_ctx *conn = (poll_ctx *)ptr;
    if (!conn)
        return NULL;

    bson_t *events;
    int time_left = MAX_POLL_SEC;
    mongo_instance *minst = create_mongo_instance();
    while (time_left--) {
        if (events) 
            bson_destroy(events);
        events = get_user_events(minst->db, conn->key);
        if (bson_count_keys(events)){
            clear_user_events(minst->db, conn->key);
            break;
        }
        sleep(1);
    }
    destroy_mongo_instance(minst);
    
    conn->data = (void *)BCON_NEW("events", BCON_ARRAY(events));
    bson_destroy(events);

    conn->stopped = true;
    struct timespec t = {0, 100000};
    while (conn->connections)
        nanosleep(&t, &t);
    bson_destroy(conn->data);
    destroy_poll_connection(conn);
    return NULL;
}

static poll_ctx *create_update_poll_connection(const char *key) {
    poll_ctx *conn = (poll_ctx *)dict_get(_connections, key);
    if (conn)
        return conn;
    conn = (poll_ctx *)malloc(sizeof(poll_ctx));
    conn->connections = 0;
    conn->data = NULL;
    conn->key = strdup(key);
    conn->stopped = false;
    pthread_create(&conn->thread, NULL, update_poll_provider, (void *)conn);
    dict_set(_connections, key, conn);
    return conn;
}

bson_t *subscribe_updates_poll(const char *user_id) {
    poll_ctx *conn = create_update_poll_connection(user_id);
    conn->connections++;

    struct timespec t = {0, 100000000};
    while (!conn->stopped)
        nanosleep(&t, &t);

    bson_t *ret = bson_copy((bson_t *)conn->data);
    conn->connections--;
    return ret;
}
