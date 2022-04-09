#include "middlewares.h"

int db_middleware_before(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)res;
    if (strcmp(req->location, "/") == 0)
        return 0;
    ctx->minst = create_mongo_instance();
    return 0;
}
