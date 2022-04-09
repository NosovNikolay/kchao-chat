#include "middlewares.h"

bool is_json_req(Request *req) {
    char *ct = get_header(req->headers, HEADER_CONTENT_TYPE);
    return ct && strstr(ct, CONTENT_TYPE_application_json) ? true : false;
}
