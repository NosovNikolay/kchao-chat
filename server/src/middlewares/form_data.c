#include "middlewares.h"

int form_data_middleware_before(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)res;
    string_t boundary = get_form_data_boundary(get_header(req->headers, HEADER_CONTENT_TYPE));
    if (!boundary)
        return 0;

    ctx->form_data = parse_form_data(req->body, req->body_len, boundary);

    free(boundary);
    return 0;
}
