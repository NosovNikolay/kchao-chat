#pragma once

#include <uchat.h>
#include "../../api/api.h"

typedef struct store_email_s {
    Store *store;
    const char *email; 
} t_store_email;

void login_callback(RCResponse *response, void *data);
void register_callback(RCResponse *response, void *data);
void send_email_callback(RCResponse *response, void *data);
void send_reset_email_callback(RCResponse *response, void *data);
void send_message_handler(GtkWidget *widget, Store *store);

void login_button_handler(GtkWidget *widget, Store *store);
void register_link_handler(GtkWidget *widget, Store *store);
void back_button_handler(GtkWidget *widget, Store *store);
void register_button_handler(GtkWidget *widget, Store *store);
void reset_password_link_handler(GtkWidget *widget, Store *store);
void reset_password_button_handler(GtkWidget *widget, Store *store);

void apply_auth_handlers();
