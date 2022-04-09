#include "chats.h"

bool message_is_runable(const char *type) {
    if(mx_streq(type, "c")) return true;
    if(mx_streq(type, "c++")) return true;
    if(mx_streq(type, "python")) return true;
    if(mx_streq(type, "node")) return true;
    if(mx_streq(type, "shell")) return true;
    return false;
}


string_t get_message_time(const time_t clock) {
    string_t full_time = mx_strnew(26);
    string_t message_time = mx_strnew(5);
    ctime_r(&(clock), full_time);
    mx_strncpy(message_time, (full_time + 11), 5);
    mx_strdel(&full_time);
    return message_time;
}

int get_event_type(const char *event_type ) {
    if (event_type == NULL) return E_NO_NEW;
    if (mx_streq(event_type, NEW_MESSAGE)) return E_NEW_MESSAGE;
    return -1;
}

int get_unconfirmed_index(Store *store, int chat_index, message_t *message) {
    for (int i = 0; i < store->chats->chats_arr[chat_index]->count_not_confirmed; i++) {
        if (mx_streq(store->chats->chats_arr[chat_index]->not_confirmed_messages[i]->type, message->type) &&
            mx_streq(store->chats->chats_arr[chat_index]->not_confirmed_messages[i]->text, message->text)) return i;
    }
    return -1;
}

int find_chat_index(Store *store, message_t *message) {
    if (!message->chat_id) return -1;
    for (int i = 0; i < store->chats->chats_count; i++) {
        if (mx_streq(store->chats->chats_arr[i]->_id, message->chat_id)) return i;
    }
    return -1;
}

int find_member_index_by_id(Store *store, const char *id) {
    for (int i = 0; i < store->all_users_count; i++) {
        if (mx_streq(store->all_users[i]->id, id)) return i;
    }
    return -1;
}

int find_private_chat_by_second_member(Store *store, const char *id) {
    for (int i = 0; i < store->chats->chats_count; i++) {
        if (is_private(store->chats->chats_arr[i]->type)) {
            for (int j = 0; j < store->chats->chats_arr[i]->count_participants; j++) {
                if (mx_streq(store->chats->chats_arr[i]->participants_id[j], id)) return i;
            }
        }
    }
    return -1;
}

int find_chat_index_by_id(Store *store, const char *id) {
    for (int i = 0; i < store->chats->chats_count; i++) {
        if (mx_streq(store->chats->chats_arr[i]->_id, id)) return i;
    }
    return -1;
}

int get_message_index_in_last_50(Store *store, int chat_id, message_t *message) {
    int border_left = store->chats->chats_arr[chat_id]->count_messages;
    int borger_right = border_left - 50;
    if (borger_right < 0) borger_right = 0;
    for (int i = border_left - 1; i > borger_right; i--) {
        if (mx_streq(store->chats->chats_arr[chat_id]->messages_history[i]->text, message->text)) return i;
    }
    mx_printstr("return -1\n");
    printf("последнее сообщение в чате = %s\n", store->chats->chats_arr[chat_id]->messages_history[border_left - 1]->text);
    printf("месседж = %s\n", message->text);
    return -1;
}

bool is_message_in_chat(Store *store, message_t *message) {
    if (find_chat_index(store, message) == -1) {
        return false;
    }
    return true;
}

void free_message(message_t *message) {
    free((void *)message->text);
    free((void *)message->type);
    free((void *)message->message_id);
    free((void *)message->chat_id);
    free((void *)message->from_user);
    free((void *)message);
    message = NULL;
}

int get_second_participant_id(Store *store,chat_entry *chat) {
    if (!mx_streq(chat->type, "private")) return -1;

    if (chat->count_participants == 1) return 0;
    if (!mx_streq(chat->participants_id[0], store->whoami)) return 0;
    return 1;
}

void free_last_message(const char* message) {
    if (message) free((void *)message);
}


const char *trim_last_message(const char *message) {
    if (mx_strlen(message) > 15) {
        char* last_message = mx_strnew(18);
        mx_strncpy(last_message, message, 15);
        mx_strcat(last_message, "...");
        return (const char*)last_message;
    }
    return mx_strdup(message);
}

bool is_private(const char *type) {
    if (mx_streq(type, "private")) return true;
    return false;
}

int get_user_index(Store *store ,const char *id) {
    for (int i = 0; i < store->all_users_count; i++) {
        if (mx_streq(id, store->all_users[i]->id)) return i;
    }
    return -1;
}
