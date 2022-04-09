#include "api/api.h"
#include "handlers/handlers.h"
#include "uchat.h"

void load_chat_name(chat_entry *chat) {
        gtk_label_set_text(GTK_LABEL(chat->gtk_chat_name), chat->name);
}

void load_chat_info_name(int chat_id, Store *store) {
    chat_entry *chat = store->chats->chats_arr[chat_id];
    if (!is_private(chat->type) && mx_streq(chat->admin, store->whoami)) {
        gtk_entry_set_text(GTK_ENTRY(chat->active_entry->chat_info_chat_name), chat->name);
    } else {
        gtk_label_set_text(GTK_LABEL(chat->active_entry->chat_info_chat_name), chat->name);
    }
}

void load_chat_info_bio(chat_entry *chat) {
    Store *store = get_store();
    if (!is_private(chat->type) && mx_streq(chat->admin, store->whoami)) {
        gtk_entry_set_text(GTK_ENTRY(chat->active_entry->chat_info_chat_bio), chat->description);
    } else {
        gtk_label_set_text(GTK_LABEL(chat->active_entry->chat_info_chat_bio), chat->description);
    }
}

void load_chat_sec_label(int chat_id, Store *store) {
    gtk_label_set_text(GTK_LABEL(store->chats->chats_arr[chat_id]->active_entry->chat_info_sec_label), store->chats->chats_arr[chat_id]->active_entry->chat_info_sec_label_text);
}
