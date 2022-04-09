#pragma once

#define _XOPEN_SOURCE 700
#define _DARWIN_C_SOURCE

#include <libmx.h>
#include <pthread.h>
#include <mongoc/mongoc.h>
#include <stdarg.h>

#include "errors.h"

#define ERROR_JSON_TEMPLATE "{\"code\":<code>,\"message\":\"<message>\"}"
#define EMAIL_CONFIRMATION_JSON_TEMPLATE "{\"user_id\":\"<user_id>\", \"sent_at\":\"<sent_at>\"}"
