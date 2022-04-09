#include "email.h"

int send_email(const char *to, const char *subject, const char *message) {
    CURLcode res = -1;
    CURL *curl = curl_easy_init();
    if (!curl)
        return res;
    const char *password = getenv("MAIL_PASS");
    if (!password) {
        password = "znouqugmnoswefti";
        // curl_easy_cleanup(curl);
        // return res;
    }
    const char *app_email = getenv("APP_EMAIL");
    if (!app_email)
        app_email = "ckchao.chat@gmail.com";

    if (mx_streq(to, MAIL_SELF))
        to = app_email;

    string_t text_ = mx_replace_substr(MAIL_TEMPLATE, "{to}", to);
    string_t text__ = mx_replace_substr(text_, "{subject}", subject);
    string_t text = mx_replace_substr(text__, "{body}", message);
    FILE *file = str_to_file(text);

    curl_easy_setopt(curl, CURLOPT_URL, SMTP_SSL_PROTOCOL "://" MAIL_SERVER ":" MAIL_PORT);

    curl_easy_setopt(curl, CURLOPT_USERNAME, app_email);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, app_email);

    struct curl_slist *recipients = NULL;
    recipients = curl_slist_append(recipients, to);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_easy_setopt(curl, CURLOPT_READDATA, file);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, fread);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    free(text_);
    free(text__);
    free(text);
    fclose(file);

    return (int)res;
}

static void *send_email_ctx(email_ctx *ctx) {
    send_email(ctx->to, ctx->subject, ctx->message);
    if (ctx->to)
        free(ctx->to);
    if (ctx->subject)
        free(ctx->subject);
    if (ctx->message)
        free(ctx->message);
    free(ctx);
    return NULL;
}

int notify_server_started() {
    return send_email(MAIL_SELF, "Server started.", "Server started!");
}

pthread_t send_email_async(const char *to, const char *subject, const char *message) {
    email_ctx *ctx = (email_ctx *)malloc(sizeof(email_ctx));
    ctx->to = to ? mx_strdup(to) : NULL;
    ctx->subject = subject ? mx_strdup(subject) : NULL;
    ctx->message = message ? mx_strdup(message) : NULL;
    pthread_t p;
    pthread_create(&p, NULL, (void *(*)(void *))send_email_ctx, (void *)ctx);
    return p;
}
