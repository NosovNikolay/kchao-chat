#include "handlers.h"

void login_button_handler(GtkWidget *widget, Store *store) {
    (void)widget;

    GtkEntry *email_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryEmail");
    const char *email = gtk_entry_get_text(GTK_ENTRY(email_entry));
    GtkEntry *pass_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryPassword");
    const char *password = gtk_entry_get_text(GTK_ENTRY(pass_entry));

    GtkLabel *label = NULL;
    GtkStyleContext *context = NULL;

    // Hide them all
    gtk_remove_labels_text_login(store);
    gtk_remove_styles_login(store);

    // check correctly of data
    if (!is_email_valid(email)) {
        context = gtk_widget_get_style_context(GTK_WIDGET(email_entry));
        gtk_style_context_add_class(context, "InputFailed");

        label = (GtkLabel *)gtk_builder_get_object(store->builder, "LoginEmailLabel");
        gtk_label_set_text(GTK_LABEL(label), "invalid email");
        gtk_widget_show(GTK_WIDGET(label));
        return;
    }

    // if all is correct send data to server
    GThread *thread = api_login_async(login_callback, store, email, password);
    g_thread_unref(thread);
}

void login_callback(RCResponse *response, void *data) {
    Store *store = (Store *)data;
    bson_t *bson_responce = NULL;
    bson_error_t bson_error;

    bson_responce = bson_new_from_json(response->body, -1, &bson_error);
    RC_response_destroy(response);
    if (!bson_responce) {
        printf("Bson error %s\n", bson_error.message);
        return;
    }

    int code = bson_get_int(bson_responce, "code");

    if (response->code == SUCCESS) {

        const char *token = prepare_string(bson_get_str(bson_responce, "auth_token"));
        if (token == NULL) {
            mx_printerr("token = NULL");
            return;
        }
        const char *user_id = bson_get_inside_str(bson_responce, "user", "_id");
        if (user_id == NULL) {
            mx_printerr("user_id = NULL");
            return;
        }
        api_authorize(token);

        GThread *autologin_thread = api_get_user_async(autologin_callback, (void *)store, user_id);
        g_thread_unref(autologin_thread);

        // add user to datafile


        if (!is_user_in_data(data, user_id)) {
            users_data *user_data = add_user_to_data(NULL, user_id, token);
            save_users(user_data, PATH_TO_BINARY);
            free_user_data(user_data);
        }

        // delete entry text
        GtkEntry *email_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryEmail");
        GtkEntry *pass_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryPassword");
        gtk_entry_set_text(email_entry, "");
        gtk_entry_set_text(pass_entry, "");
        return;
    }

    GtkStyleContext *context = NULL;
    GtkLabel *label = NULL;

    GtkEntry *email_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryEmail");
    GtkEntry *pass_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryPassword");
    GtkEntry *message_label = (GtkEntry *)gtk_builder_get_object(store->builder, "UserMessageLabel");

    if (code == EMAIL_NOT_CONFIRMED) {
        context = gtk_widget_get_style_context(GTK_WIDGET(email_entry));
        gtk_style_context_add_class(context, "InputFailed");
        label = (GtkLabel *)gtk_builder_get_object(store->builder, "LoginEmailLabel");
        gtk_label_set_text(GTK_LABEL(label), "Email is not confirmed");
        gtk_widget_show(GTK_WIDGET(label));
    } else if (code == UNAME_OR_PSW_WRONG || code == USER_NOT_FOUND) {
        context = gtk_widget_get_style_context(GTK_WIDGET(email_entry));
        gtk_style_context_add_class(context, "InputFailed");
        context = gtk_widget_get_style_context(GTK_WIDGET(pass_entry));
        gtk_style_context_add_class(context, "InputFailed");
        context = gtk_widget_get_style_context(GTK_WIDGET(message_label));
        gtk_style_context_add_class(context, "UserErrorLabel");
        gtk_style_context_remove_class(context, "UserSuccessLabel");
        label = (GtkLabel *)gtk_builder_get_object(store->builder, "UserMessageLabel");
        gtk_label_set_text(GTK_LABEL(label), "Wrong password/email");
        gtk_widget_show(GTK_WIDGET(label));
    }

    bson_destroy(bson_responce);
}

void register_button_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    GtkLabel *label;
    GtkStyleContext *context;

    // hide all mesage labels
    gtk_remove_labels_text_register(store);
    gtk_remove_styles_register(store);

    GtkEntry *pass_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterPassword");
    GtkEntry *pass_repeat_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterRepeatPassword");
    GtkEntry *email_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterEmail");
    GtkEntry *usermane_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterUsername");

    const char *username = gtk_entry_get_text(GTK_ENTRY(usermane_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(pass_entry));
    const char *repeat_password = gtk_entry_get_text(GTK_ENTRY(pass_repeat_entry));
    const char *email = gtk_entry_get_text(GTK_ENTRY(email_entry));

    if (!is_email_valid(email)) {
        context = gtk_widget_get_style_context(GTK_WIDGET(email_entry));
        gtk_style_context_add_class(context, "InputFailed");
        label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterEmailLabel");
        gtk_label_set_text(GTK_LABEL(label), "invalid email");
        gtk_widget_show(GTK_WIDGET(label));
        return;
    }

    // check is password valid
    char *error_message = NULL;
    if (!is_len_password_valid(password)) {
        error_message = "password is too short";
    } else if (!is_password_valid(password)) {
        error_message = "A-Za-z_!\"#$&'()*+,-./:";
    }

    if (error_message) {
        context = gtk_widget_get_style_context(GTK_WIDGET(pass_entry));
        gtk_style_context_add_class(context, "InputFailed");
        label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterPasswordLabel");
        gtk_label_set_text(GTK_LABEL(label), error_message);
        gtk_widget_show(GTK_WIDGET(label));
        return;
    }

    if (!mx_streq(password, repeat_password)) {
        context = gtk_widget_get_style_context(GTK_WIDGET(pass_repeat_entry));
        gtk_style_context_add_class(context, "InputFailed");
        label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterRepeatPasswordLabel");
        gtk_label_set_text(GTK_LABEL(label), "passwords not equal");
        gtk_widget_show(GTK_WIDGET(label));
        return;
    }

    // Send message
    t_store_email *store_email = malloc(sizeof(store_email));
    store_email->store = store;
    store_email->email = email;

    GThread *register_thread = api_register_async(register_callback, (void *)store_email, username, email, password);
    g_thread_unref(register_thread);
}

void register_callback(RCResponse *response, void *data) {
    t_store_email *store_email = (t_store_email *)data;
    const char *email = store_email->email;
    Store *store = (Store *)store_email->store;

    GtkLabel *label;
    GtkStyleContext *context;

    printf("%s\n", response->body);
    printf("%ld\n", response->code);

    bson_t *bson_responce = NULL;
    bson_error_t bson_error;
    bson_responce = bson_new_from_json(response->body, -1, &bson_error);
    RC_response_destroy(response);
    if (!bson_responce) {
        printf("%s\n", bson_error.message);
        return;
    }

    int code = bson_get_int(bson_responce, "code");

    if (code == BSON_NOT_FOUND) {
        mx_printerr("BSON_NOT_FOUND\n");
        return;
    }

    string_t message;
    if (code == SUCCESS) {
        // send email confirmation
        GThread *email_thread = api_send_email_confirmation_async(send_email_callback, (void *)store, email);
        g_thread_unref(email_thread);

        label = (GtkLabel *)gtk_builder_get_object(store->builder, "UserMessageLabel");
        context = gtk_widget_get_style_context(GTK_WIDGET(label));
        gtk_style_context_add_class(context, "UserSuccessLabel");
        gtk_style_context_remove_class(context, "UserErrorLabel");
        message = create_success_message(email);
        gtk_label_set_text(GTK_LABEL(label), message);
        gtk_widget_show(GTK_WIDGET(label));
        mx_strdel(&message);
        back_button_handler(NULL, store);
    } else if (code == NO_CONNECTION) {
        label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterButtonLabel");
        message = mx_strdup("no server connection");
        gtk_label_set_text(GTK_LABEL(label), message);
        gtk_widget_show(GTK_WIDGET(label));
        mx_strdel(&message);
    } else if (code == USER_EXIST) {
        GtkEntry *email_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterEmail");
        context = gtk_widget_get_style_context(GTK_WIDGET(email_entry));
        gtk_style_context_add_class(context, "InputFailed");
        label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterEmailLabel");
        message = mx_strdup("user aledary exit");
        gtk_label_set_text(GTK_LABEL(label), message);
        gtk_widget_show(GTK_WIDGET(label));
        mx_strdel(&message);
    }

    bson_destroy(bson_responce);
    free(store_email);
}

void send_email_callback(RCResponse *response, void *data) {
    RC_response_destroy(response);
    (void)data;
    return;
}

void reset_password_button_handler(GtkWidget *widget, Store *store) {
    GtkLabel *label = (GtkLabel *)gtk_builder_get_object(store->builder, "EmailResetLabel");
    gtk_widget_hide(GTK_WIDGET(label));

    GtkEntry *email_reset = (GtkEntry *)gtk_builder_get_object(store->builder, "ResetEmail");
    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(email_reset));
    gtk_style_context_remove_class(context, "InputFailed");

    const char *email = gtk_entry_get_text(GTK_ENTRY(email_reset));

    if (!is_email_valid(email)) {
        context = gtk_widget_get_style_context(GTK_WIDGET(email_reset));
        gtk_style_context_add_class(context, "InputFailed");
        gtk_label_set_text(GTK_LABEL(label), "invalid email");
        gtk_widget_show(GTK_WIDGET(label));
        return;
    }

    t_store_email *store_email = malloc(sizeof(store_email));
    store_email->store = store;
    store_email->email = email;

    GThread *reset_thread = api_send_email_reset_async(send_reset_email_callback, (void *)store_email, email);
    g_thread_unref(reset_thread);

    back_button_handler(widget, store);
}

void send_reset_email_callback(RCResponse *response, void *data) {
    t_store_email *store_email = (t_store_email *)data;
    const char *email = store_email->email;
    Store *store = (Store *)store_email->store;

    if (response->code == 200) {
        GtkLabel *message_label = (GtkLabel *)gtk_builder_get_object(store->builder, "UserMessageLabel");
        string_t message = create_reset_message(email);

        gtk_label_set_text(GTK_LABEL(message_label), message);
        GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(message_label));
        gtk_style_context_add_class(context, "UserSuccessLabel");
        gtk_widget_show(GTK_WIDGET(message_label));
        mx_strdel(&message);
    }

    RC_response_destroy(response);
    free(store_email);
    return;
}

void register_link_handler(GtkWidget *widget, Store *store) {
    widget = NULL;
    gtk_remove_text_login(store);
    gtk_remove_styles_login(store);

    GtkStack *stack = (GtkStack *)gtk_builder_get_object(store->builder, "LoginTabs");
    gtk_stack_set_transition_duration(stack, 400);
    gtk_stack_set_visible_child_full(stack, "UserRegister", GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
}

void back_button_handler(GtkWidget *widget, Store *store) {
    widget = NULL;

    gtk_remove_styles_register(store);
    gtk_remove_text_register(store);

    GtkStack *stack = (GtkStack *)gtk_builder_get_object(store->builder, "LoginTabs");
    gtk_stack_set_transition_duration(stack, 400);
    gtk_stack_set_visible_child_full(stack, "UserAuth", GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
}

void reset_password_link_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    gtk_remove_text_login(store);
    gtk_remove_styles_login(store);

    GtkStack *stack = (GtkStack *)gtk_builder_get_object(store->builder, "LoginTabs");

    gtk_stack_set_transition_duration(stack, 400);
    gtk_stack_set_visible_child_full(stack, "ResetPassword", GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
}

void apply_auth_handlers() {
    Store *store = get_store();

    g_signal_connect(gtk_builder_get_object(store->builder, "BackButton"), "clicked", G_CALLBACK(back_button_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "BackButtonReset"), "clicked", G_CALLBACK(back_button_handler), store);

    g_signal_connect(gtk_builder_get_object(store->builder, "ContinueButton"), "clicked", G_CALLBACK(login_button_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "RegisterButton"), "clicked", G_CALLBACK(register_link_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "ContinueRegisterButton"), "clicked", G_CALLBACK(register_button_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "ResetPasswordLink"), "clicked", G_CALLBACK(reset_password_link_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "ResetPasswordButton"), "clicked", G_CALLBACK(reset_password_button_handler), store);
}
