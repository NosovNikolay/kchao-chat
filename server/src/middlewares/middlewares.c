#include "middlewares.h"

void apply_middlewares(void) {
    add_before_middleware(API_CB(db_middleware_before));
    add_before_middleware(API_CB(auth_middleware_before));
    add_before_middleware(API_CB(expect_json_middleware_before));
    add_before_middleware(API_CB(json_middleware_before));
    add_before_middleware(API_CB(form_data_middleware_before));
}
