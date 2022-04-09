#include "db.h"

static mongoc_client_pool_t *_pool = NULL;
static mongoc_uri_t *_uri = NULL;

pthread_mutex_t pool_mutex;

void init_db() {
    if (_pool)
        return;
    mongoc_init();
    if (!_uri) {
        bson_error_t error;
        _uri = mongoc_uri_new_with_error(DB_URI, &error);
        if (!_uri) {
            fprintf(stderr,
                    "failed to parse URI: %s\n"
                    "error message:       %s\n",
                    DB_URI,
                    error.message);
        }
    }
    _pool = mongoc_client_pool_new(_uri);
}

int db_ping() {
    mongo_instance *minst = create_mongo_instance();
    bson_t *command, reply;
    bson_error_t error;
    char *str;
    command = BCON_NEW("ping", BCON_INT32(1));
    bool retval = mongoc_database_command_simple(minst->db, command, NULL, &reply, &error);

    if (!retval) {
        fprintf(stderr, "DB ping: %s\n", error.message);
        return EXIT_FAILURE;
    }

    str = bson_as_json(&reply, NULL);

    mx_printstr("DB ping: ");
    mx_printstr(str);
    mx_printchar('\n');

    bson_destroy(command);
    bson_destroy(&reply);
    bson_free(str);
    destroy_mongo_instance(minst);
    return EXIT_SUCCESS;
}

void destroy_db() {
    if (_pool)
        mongoc_client_pool_destroy(_pool);
    if (_uri)
        mongoc_uri_destroy(_uri);
    mongoc_cleanup();
}

mongo_instance *get_separate_mnist() {
    mongo_instance *minst = (mongo_instance *)malloc(sizeof(mongo_instance));
    minst->pool = NULL;
    pthread_mutex_lock(&pool_mutex);
    minst->client = mongoc_client_new_from_uri(_uri);
    pthread_mutex_unlock(&pool_mutex);
    minst->db = mongoc_client_get_database(minst->client, DB_NAME);
    return minst;
}

void destroy_separate_mnist(mongo_instance *minst) {
    if (!minst)
        return;
    if (minst->client)
        mongoc_client_destroy(minst->client);
    if (minst->db)
        mongoc_database_destroy(minst->db);
    free(minst);
}

mongo_instance *create_mongo_instance() {
    mongo_instance *minst = (mongo_instance *)malloc(sizeof(mongo_instance));
    pthread_mutex_lock(&pool_mutex);
    minst->pool = _pool;
    pthread_mutex_unlock(&pool_mutex);
    minst->client = mongoc_client_pool_pop(minst->pool);
    minst->db = mongoc_client_get_database(minst->client, DB_NAME);
    return minst;
}

void destroy_mongo_instance(mongo_instance *minst) {
    if (!minst)
        return;
    if (minst->db)
        mongoc_database_destroy(minst->db);
    pthread_mutex_lock(&pool_mutex);
    if (minst->client && minst->pool == _pool && _pool) {
        mongoc_client_pool_push(minst->pool, minst->client);
    } else if (minst->client)
        mongoc_client_destroy(minst->client);
    pthread_mutex_unlock(&pool_mutex);
    free(minst);
}
