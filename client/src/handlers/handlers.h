#pragma once
#include <uchat.h>
#include "../api/api.h"
#include "../async/async.h"
#include "../chat/chats.h"
#include "settings/handlers.h"

typedef struct participants_search_ctx_s {
    int64_t sended_at;
} participants_search_ctx_t;

typedef struct search_chats_s {
    const char *type;
    const char *name;
    const char *id;
    search_partipiants_ui_t *ui;
}   search_chats_t;


void btn_window_close_handler(GtkWidget *widget, Store* store);
void btn_window_fullscreen_handler(GtkWidget *widget, Store* store);
void btn_window_minimize_handler(GtkWidget *widget, Store* store);
void setting_button_handler(GtkWidget *widget, Store *store);

void apply_root_handlers();

// search
search_partipiants_ui_t *create_result_list_ui(const char *name);
void chat_search_handler(GtkSearchEntry *entry, Store *store);
void chat_search_cb(RCResponse *res, void *ctx);
void connect_chat_ui(search_chats_t *search);
void create_chat_result_list(bson_t *bson_search);
void add_entry_to_search(bson_t *bson_data);

void search_private_event(GtkWidget *widget, GdkEventButton *event, search_chats_t *part);
void search_user_event(GtkWidget *widget, GdkEventButton *event, search_chats_t *part);
void search_group_event(GtkWidget *widget, GdkEventButton *event, search_chats_t *part);
void create_private_chat(Store *store ,const char *id);
void create_private_chat_cb(RCResponse *res, void *_);
