#pragma once

#include <cairo.h>

#include "../api/api.h"
#include "../handlers/handlers.h"
#include "../handlers/messages/handlers.h"
#include "uchat.h"

#define CHATS_KEY "chats.{index}.{key}"
#define EVENTS_TYPE_KEY "events.{index}.type"
#define PARTICIPANTS_TYPE_KEY "chats.{chat_index}.participants.{index}"

#define DEFAULT_IMAGE "data/icons/logo_reg.png"
#define SENDED_MSG_IMAGE "client/data/icons/sended.png"
#define MESSAGE_RUN_IMAGE "client/data/icons/run.png"

#define NEW_MESSAGE "new_message"

#define E_NO_NEW 0
#define E_NEW_MESSAGE 1

#define INIT_MESSAGE_EVENT_KEY "events.{index}.message." 
#define INIT_MESSAGE_CHAT_KEY "messages.{index}." 

#define DEFAULT_AVATAR_ID "0000000000"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define GTK_PACK_START 1
#define GTK_PACK_END 2 

typedef struct load_chat_history_s {
    const char *chat_id;
    int cur;
} load_chat_history_ctx;

typedef struct widget_id{
    GtkWidget *widget;
    gulong id;
    int size;
} widget_id_t;


char *ctime_r(const time_t * clock, char *buf);

void chat_preload(Store *store, bson_t *user_data);

void init_chats(Store *store);

chat_entry *load_user_chat(int chat_id, bson_t *user_data);

int add_chat_to_store(Store *store, chat_entry *chat);

void add_chat_to_ui(int chat_id, Store *store);

void open_chat(GtkWidget *widget, GdkEventButton *event, int *page);

void apply_events(bson_t *updates_return);

gpointer run_updates(gpointer _);

int get_event_type(const char *event_type );

int find_member_index_by_id(Store *store, const char *id);

int find_chat_index(Store *store, message_t *message);

int find_private_chat_by_second_member(Store *store, const char *id);

message_t *init_new_message(const char *first_key, bson_t *update_data, int index);

message_t *message_new_from_bson(bson_t *bson_message);

void add_message_to_chat(chat_entry *chat, message_t *message);

void add_message_to_ui(chat_entry *chat, message_t *message, int pack_type);

message_t *get_message_in_chat(chat_entry *chat, message_t *message);

void init_chat_window(Store *store, int chat_index);

void message_add_style(message_t *message);

void get_messages_callback(RCResponse *responce, void *data);

bool is_message_in_chat(Store *store, message_t *message);

int find_chat_index_by_id(Store *store, const char *id);

int get_message_index_in_last_50(Store *store, int chat_id, message_t *message);

void add_message_to_not_confirmed(Store *store, int chat_id,message_t *message);

void add_message_to_not_confirmed_ui(Store *store, int chat_index, message_t *message);

void free_message(message_t *message);

int get_unconfirmed_index(Store *store, int chat_index, message_t *message);

void pop_message_from_not_confirmed_arr(Store *store, int chat_index, int message_index);

void remove_message_from_not_confirmed(Store *store, int chat_index, message_t *message);

void pop_message_from_not_confirmed_ui(Store *store, int chat_index, int message_index);

const char **chat_get_participants(chat_entry *chat, bson_t *user_data, int chat_id);

void chat_get_participants_cb(RCResponse *response, void *data);

void load_chat_image(int chat_id, Store *store);

void load_chat_images_cb(RCResponse *response, void *data);

int get_second_participant_id(Store *store,chat_entry *chat);

void draw_default_avatar(GtkWidget *widget, cairo_t *cr, int size);

void draw_user_avatar(GtkWidget *widget, cairo_t *cr, void *data);

void user_chats_add_style(Store *store);

const char *trim_last_message(const char *message);

void free_last_message(const char* message);

message_ui_t *create_ui_message(message_t *message, bool is_sended);

string_t get_message_time(const time_t clock);

bool message_is_runable(const char *type);

void *load_chat_history(void *data);

void load_chat_history_cb(bson_t *bson_messages, const char *chat_id);

void load_default_user(Store *store);

bool is_private(const char *type);

gboolean apply_message_event(gpointer message_ptr);

void prepare_scroll_down(GtkWidget *scrolled_window);

bool is_user_in_chat(chat_entry *chat, const char *id);

const char **bson_get_str_array(bson_t *bson_array, int *size);

chat_entry *chat_new_from_bson(bson_t *bson_chat);

gboolean apply_chat_event(gpointer chat_ptr);

gboolean apply_chat_changed_event(gpointer chat_ptr);

void compare_chat_changes(chat_entry *dst,  chat_entry *src, int chat_index);

void update_chats_pack_position(Store *store);

members_ui_t **init_members_ui(chat_entry *chat);

void members_add_style(chat_entry *chat);
