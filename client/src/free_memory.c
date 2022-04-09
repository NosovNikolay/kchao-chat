#include "handlers/handlers.h"
#include "uchat.h"

void free_chat_messages(chat_entry *chat) {
    for (int i = 0; i < chat->count_messages; i++) {
        free_chat_message(chat->messages_history[i]);
    }
    chat->count_messages = 0;

    if (chat->active_entry)
        free_chat_ui_message(chat->active_entry);
}

void free_chat_message(message_t *message) {
    free((void *)message->chat_id);
    free((void *)message->from_user);
    free((void *)message->message_id);
    free((void *)message->text);
    free((void *)message->type);
    free((void *)message);
    message = NULL;
}

void free_chat_ui_message(gtk_actve_chat_entry_t *active) {
    free((void *)active->chat_info_sec_label_text);
}

void free_chat(chat_entry *chat) {
    if (GTK_IS_WIDGET(chat->gtk_event_box)) {
        g_signal_handler_disconnect(chat->gtk_event_box, chat->chat_window_signal);
        gtk_widget_destroy(chat->gtk_event_box);
    }
    if (GTK_IS_WIDGET(chat->gtk_event_box)) {
        g_signal_handler_disconnect(chat->gtk_event_box, chat->avatar_handler_id);
    }
    if (GTK_IS_WIDGET(chat->active_entry->adjustment)) {
        g_signal_handler_disconnect(chat->active_entry->adjustment, chat->active_entry->adjustment_signal);
    }
    if (GTK_IS_WIDGET(chat->active_entry->adjustment)) {
        g_signal_handler_disconnect(chat->active_entry->chat_image, chat->active_entry->handler_id);
    }

    free_chat_messages(chat);

    for (int i = 0; i < chat->count_participants; i++) {
        free((void *)chat->participants_id[i]);
    }

    free_chat_inst(chat);

    free((void *)chat);
    chat = NULL;
}

void free_chat_inst(chat_entry *chat) {
    if (chat->admin)
        free((void *)chat->admin);
    if (chat->last_message_str)
        free((void *)chat->last_message_str);
    if (chat->name)
        free((void *)chat->name);
    if (chat->description)
        free((void *)chat->description);
    if (chat->type)
        free((void *)chat->type);
    if (chat->_id)
        free((void *)chat->_id);
} 

void free_user(user_entry_t *user) {
    if (!user) return;
    if (user->avatar_id)
        free((void *)user->avatar_id);
    if (user->bio)
        free((void *)user->bio);
    if (user->email)
        free((void *)user->email);
    if (user->id)
        free((void *)user->id);
    if (user->nickname)
        free((void *)user->nickname);
    free((void *)user);
}