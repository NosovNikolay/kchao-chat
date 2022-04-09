#include "auth.h"

static int handler_auth_login(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;
    string_t email = prepare_string(bson_get_str(ctx->bson, "email"));
    string_t password = prepare_string(bson_get_str(ctx->bson, "password"));

    if (!password || !email) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        free_strings(2, &email, &password);
        return 1;
    }

    string_t hashed_pswrd = hash_password(password);
    bool authorized = authorize_user(ctx->minst->db, email, hashed_pswrd);
    free(hashed_pswrd);

    if (!authorized) {
        bson_t *uuser = get_unconfirmed_user_by_email(ctx->minst->db, email);
        if (!uuser) {
            bson_respond_err(res, ERR_INVALID_CREDENTIALS, RESPONSE_401);
            free_strings(2, &email, &password);
            return 1;
        }
        bson_respond_err(res, ERR_EMAIL_NOT_CONFIRMED, RESPONSE_401);
        bson_destroy(uuser);
        free_strings(2, &email, &password);
        return 1;
    }

    bson_t *user = get_full_user_by_email(ctx->minst->db, email);

    free_strings(2, &email, &password);

    string_t user_id = prepare_string(bson_get_str(user, "_id"));
    string_t token_data = mx_replace_substr(USER_TOKEN_DATA, "<user_id>", user_id);
    string_t auth_token = sha512_generate_token(token_data);

    bson_t *res_bson = BCON_NEW(
        "auth_token", BCON_UTF8(auth_token),
        "user", BCON_DOCUMENT(user));

    bson_respond(res, RESPONSE_200, res_bson);

    free_strings(3, &user_id, &token_data, &auth_token);
    bson_destroy(res_bson);
    bson_destroy(user);
    return 0;
}

static int handler_auth_register(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;
    string_t name = prepare_string(bson_get_str(ctx->bson, "name"));
    string_t email = prepare_string(bson_get_str(ctx->bson, "email"));
    string_t password = prepare_string(bson_get_str(ctx->bson, "password"));
    if (!name && email)
        name = get_default_name(email);

    if (!name || !password || !email) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        free_strings(3, &name, &email, &password);
        return 1;
    }

    bson_t *user = get_user_by_email(ctx->minst->db, email);

    bson_t *uuser = get_unconfirmed_user_by_email(ctx->minst->db, email);

    if (user != NULL || uuser != NULL) {
        bson_respond_err(res, ERR_USER_REGISTERED, RESPONSE_409);
        if (user)
            bson_destroy(user);
        if (uuser)
            bson_destroy(uuser);
        free_strings(3, &name, &email, &password);
        return 1;
    }

    string_t hashed_pswrd = hash_password(password);
    user = create_unconfirmed_user(ctx->minst->db, name, email, hashed_pswrd);

    if (!user) {
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);
        free_strings(4, &name, &email, &password, &hashed_pswrd);
        return 1;
    }
    str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"User added successfully\"}", CONTENT_TYPE_application_json);
    free_strings(4, &name, &email, &password, &hashed_pswrd);
    bson_destroy(user);
    return 0;
}

static int handler_auth_confirm_email(Request *req, Response *res, middleware_ctx_t *ctx) {
    char *token = (char *)dict_get(req->query, "confirmToken");
    string_t page;

    if (!token) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }
    string_t json = sha512_decode_token(token);

    if (!json) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }

    bson_error_t error;
    bson_t *token_info = bson_new_from_json((const uint8_t *)json, strlen(json), &error);
    mx_strdel(&json);

    if (!token_info) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }

    string_t id = prepare_string(bson_get_str(token_info, "user_id"));
    bson_destroy(token_info);
    if (!id) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }

    bson_t *user = get_unconfirmed_user_and_delete(ctx->minst->db, id);
    mx_strdel(&id);
    if (!user) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }

    string_t name = prepare_string(bson_get_str(user, "name"));
    string_t email = prepare_string(bson_get_str(user, "email"));
    string_t pwd_hash = prepare_string(bson_get_str(user, "pwd_hash"));
    bson_t *new_user = create_user(ctx->minst->db, name, email, pwd_hash);
    free_strings(3, &name, &email, &pwd_hash);
    if (!new_user) {
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);
        return 1;
    }
    bson_destroy(new_user);

    page = file_2_str("server/data/pages/email_confirmed_success.html");
    str_respond(res, RESPONSE_200, page, CONTENT_TYPE_text_html);
    mx_strdel(&page);
    return 0;
}

static int handler_auth_send_confirmation_email(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t email = prepare_string(bson_get_str(ctx->bson, "email"));
    if (!email) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        return 1;
    }

    bson_t *user = get_unconfirmed_user_by_email(ctx->minst->db, email);

    if (!user) {
        bson_respond_err(res, ERR_USER_NOT_FOUND, RESPONSE_400);
        mx_strdel(&email);
        return 1;
    }
    string_t id = prepare_string(bson_get_str(user, "_id"));
    bson_destroy(user);
    string_t json_data = mx_replace_substr(EMAIL_CONFIRMATION_JSON_TEMPLATE, "<user_id>", id);
    string_t token = sha512_generate_token(json_data);
    string_t confirm_link = mx_strjoin(req->ctx->server->host, CONFIRM_LINK);
    string_t link = mx_strjoin(confirm_link, token);

    free_strings(4, &json_data, &token, &id, &confirm_link);

    string_t message = file_2_str("server/data/emails/verify_email.html");
    string_t rep_email = mx_replace_substr(message, "{email}", email);
    string_t rep_link = mx_replace_substr(rep_email, "{link}", link);

    extend_unconfirmed_user_by_email(ctx->minst->db, email);
    send_email_async(email, "Confirm your email", rep_link);

    free_strings(4, &message, &rep_email, &rep_link, &email);
    str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"Confirmation message sent.\"}", CONTENT_TYPE_application_json);
    return 0;
}

static int handler_auth_send_reset_password(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;

    string_t email = prepare_string(bson_get_str(ctx->bson, "email"));

    if (!email) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        return 1;
    }

    bson_t *user = get_user_by_email(ctx->minst->db, email);

    if (!user) {
        bson_respond_err(res, ERR_USER_NOT_FOUND, RESPONSE_400);
        return 1;
    }

    string_t user_id = prepare_string(bson_get_str(user, "_id"));

    bson_destroy(user);

    if (!user_id) {
        bson_respond_err(res, ERR_UNEXPECTED, RESPONSE_400);
        return 1;
    }

    if (!user_allow_pswrd_reset(ctx->minst->db, user_id)) {
        mx_strdel(&user_id);
        bson_respond_err(res, ERR_USER_NOT_FOUND, RESPONSE_400);
        return 1;
    }

    string_t rep_id = mx_replace_substr(EMAIL_CONFIRMATION_JSON_TEMPLATE, "<user_id>", user_id);
    int64_t time = get_current_time();
    string_t date_time = mx_strnew(sizeof(int64_t));
    snprintf(date_time, 14, "%" PRId64, time);
    string_t rep_date = mx_replace_substr(rep_id, "<sent_at>", date_time);
    string_t token = sha512_generate_token(rep_date);
    string_t confirm_link_reset = mx_strjoin(req->ctx->server->host, CONFIRM_LINK_RESET);
    string_t rep_link = mx_replace_substr(confirm_link_reset, "<token>", token);
    string_t rep_email = mx_strjoin(rep_link, email);

    free_strings(6, &rep_link, &user_id, &date_time, &rep_id, &rep_date, &confirm_link_reset);

    if (!token) {
        bson_respond_err(res, ERR_UNEXPECTED, RESPONSE_400);
        free_strings(2, &email, &rep_email);
        return 1;
    }
    mx_strdel(&token);
    string_t message = file_2_str("server/data/emails/reset_password.html");
    string_t rep_message = mx_replace_substr(message, "{link}", rep_email);
    mx_strdel(&rep_email);

    if (!rep_message || !message) {
        bson_respond_err(res, ERR_UNEXPECTED, RESPONSE_400);
        free_strings(2, &message, &rep_message);
        return 1;
    }

    send_email_async(email, "Change your password", rep_message);
    free_strings(3, &message, &rep_message, &email);
    str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"Password change email sent.\"}", CONTENT_TYPE_application_json);
    return 0;
}

static int handler_auth_reset_password_page(Request *req, Response *res, middleware_ctx_t *ctx) {
    string_t page;

    string_t token = (string_t)dict_get(req->query, "resetPasswordToken");
    if (!token) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }
    string_t json = prepare_string(sha512_decode_token(token));
    if (!json) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }

    bson_error_t error;
    bson_t *token_info = bson_new_from_json((const uint8_t *)json, strlen(json), &error);
    mx_strdel(&json);

    if (!token_info) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }

    string_t id = prepare_string(bson_get_str(token_info, "user_id"));
    string_t date = prepare_string(bson_get_str(token_info, "sent_at"));

    char *eptr = NULL;
    int64_t sent_at = strtol(date, &eptr, 10);
    int64_t cur_time = get_current_time();
    bson_destroy(token_info);

    if (cur_time - sent_at >= HOUR_IN_MS) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        free_strings(2, &id, &date);
        return 1;
    }

    bson_t *user = get_user(ctx->minst->db, id);
    free_strings(2, &id, &date);

    if (!user) {
        page = file_2_str("server/data/pages/link_was_expired.html");
        str_respond(res, RESPONSE_400, page, CONTENT_TYPE_text_html);
        mx_strdel(&page);
        return 1;
    }
    bson_destroy(user);

    page = file_2_str("server/data/pages/reset_password.html");
    str_respond(res, RESPONSE_200, page, CONTENT_TYPE_text_html);
    mx_strdel(&page);
    return 0;
}

static int handler_auth_reset_password(Request *req, Response *res, middleware_ctx_t *ctx) {
    (void)req;

    char *new_password = prepare_string(bson_get_str(ctx->bson, "password"));

    if (!new_password) {
        bson_respond_err(res, RESPONSE_Bad_Request, RESPONSE_400);
        return 1;
    }

    string_t token = (string_t)dict_get(req->query, "resetPasswordToken");

    if (!token) {
        mx_strdel(&new_password);
        bson_respond_err(res, ERR_TOKEN_NOT_VALID, RESPONSE_400);
        return 1;
    }

    string_t json = prepare_string(sha512_decode_token(token));

    if (!json) {
        mx_strdel(&new_password);
        bson_respond_err(res, RESPONSE_400, ERR_TOKEN_NOT_VALID);
        return 1;
    }

    bson_error_t error;
    bson_t *token_info = bson_new_from_json((const uint8_t *)json, strlen(json), &error);
    mx_strdel(&json);

    if (!token_info) {
        mx_strdel(&new_password);
        bson_respond_err(res, ERR_TOKEN_NOT_VALID, RESPONSE_400);
        return 1;
    }
    char *id = prepare_string(bson_get_str(token_info, "user_id"));
    bson_destroy(token_info);

    if (!id) {
        mx_strdel(&new_password);
        bson_respond_err(res, ERR_TOKEN_NOT_VALID, RESPONSE_400);
        return 1;
    }
    char *hashed_pswrd = hash_password(new_password);
    mx_strdel(&new_password);

    bson_t *doc = get_user(ctx->minst->db, id);
    string_t email = prepare_string(bson_get_str(doc, "email"));
    if (!doc) {
        bson_respond_err(res, ERR_TOKEN_NOT_VALID, RESPONSE_400);
        return 1;
    }

    const bson_value_t *value = bson_get(doc, "reseting_password");

    if (!value->value.v_bool) {
        bson_destroy(doc);
        bson_respond_err(res, "403 Link was pizdec", RESPONSE_403);
        return 1;
    }
    
    bson_destroy(doc);

    bool is_update = user_update_password(ctx->minst->db, id, hashed_pswrd);
    free_strings(2, &id, &hashed_pswrd);

    if (!is_update) {
        bson_respond_err(res, RESPONSE_Internal_Server_Error, RESPONSE_500);
        return 1;
    }
    string_t data = file_2_str("server/data/emails/password_changed.html");
    send_email_async(email, "Your password was changed", data);
    free_strings(2, &email, &data);
    str_respond(res, RESPONSE_200, "{\"code\":200,\"message\":\"Password changed successfully\"}", CONTENT_TYPE_application_json);
    return 0;
}

void attach_handlers_auth(void) {
    add_handler(REQUEST_POST, AUTH_PREFIX "/login", API_CB(handler_auth_login));
    add_handler(REQUEST_POST, AUTH_PREFIX "/register", API_CB(handler_auth_register));
    add_handler(REQUEST_GET, AUTH_PREFIX "/confirmEmail", API_CB(handler_auth_confirm_email));
    add_handler(REQUEST_POST, AUTH_PREFIX "/sendConfirmationEmail", API_CB(handler_auth_send_confirmation_email));

    add_handler(REQUEST_POST, AUTH_PREFIX "/sendResetPassword", API_CB(handler_auth_send_reset_password));
    add_handler(REQUEST_POST, AUTH_PREFIX "/resetPassword", API_CB(handler_auth_reset_password));
    add_handler(REQUEST_GET, AUTH_PREFIX "/resetPassword", API_CB(handler_auth_reset_password_page));
}
