#pragma once

#include <curl/curl.h>
#include <libmx.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define SMTP_SSL_PROTOCOL "smtps"
#define MAIL_SERVER "smtp.gmail.com"
#define MAIL_PORT "465"

#define MAIL_TEMPLATE ("To: <{to}>\r\n"              \
                       "From: \"<k<h@0\"\r\n"        \
                       "Subject: {subject}\r\n"      \
                       "Content-Type: text/html\r\n" \
                       "\r\n"                        \
                       "{body}")

#define MAIL_SELF "self"

typedef struct email_ctx_s {
    string_t to;
    string_t subject;
    string_t message;
} email_ctx;

FILE *fmemopen(void *buf, size_t size, const char *mode);

int send_email(const char *to, const char *subject, const char *message);

pthread_t send_email_async(const char *to, const char *subject, const char *message);

int notify_server_started();

FILE *str_to_file(const char *str);
