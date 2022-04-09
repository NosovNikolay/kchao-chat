#pragma once

#include <uchat_server.h>
#include "../server/server.h"
#include "../crypto/crypto.h"
#include "../db/db.h"
#include "../form_data/form_data.h"
#include "../utils/utils.h"

typedef struct middleware_ctx_s {
    bson_t *bson;
    bson_t *user;
    FormData *form_data;
    mongo_instance *minst;
}   middleware_ctx_t;

middleware_ctx_t *init_ctx(void);

void destroy_ctx(middleware_ctx_t *ctx);

int json_middleware_before(Request *req, Response *res, middleware_ctx_t *ctx);

int expect_json_middleware_before(Request *req, Response *res);

int auth_middleware_before(Request *req, Response *res, middleware_ctx_t *ctx);

int db_middleware_before(Request *req, Response *res, middleware_ctx_t *ctx);

int form_data_middleware_before(Request *req, Response *res, middleware_ctx_t *ctx);

bool is_json_req(Request *req);

void apply_middlewares(void);
