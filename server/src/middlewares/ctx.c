#include "middlewares.h"

middleware_ctx_t *init_ctx(void) {
    middleware_ctx_t *ctx = malloc(sizeof(middleware_ctx_t));
    ctx->bson = NULL;
    ctx->user = NULL;
    ctx->minst = NULL;
    ctx->form_data = NULL;
    return ctx;
}

void destroy_ctx(middleware_ctx_t *ctx) {
    if (!ctx)
        return;
    if (ctx->bson)
        bson_destroy(ctx->bson);
    if (ctx->user)
        bson_destroy(ctx->user);
    if (ctx->minst)
        destroy_mongo_instance(ctx->minst);
    if (ctx->form_data)
        destroy_form_data(ctx->form_data);
    free(ctx);
}
