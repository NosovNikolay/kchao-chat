#include "api/api.h"
#include "handlers/handlers.h"
#include "uchat.h"

void load_chats(Store *store) {

    // load partipiants
    GThread *load_users_thread = PROMISE(load_participants, load_participants_cb, store, store);
    g_thread_unref(load_users_thread);

    // load group images
    for (int i = 0; i < store->chats->chats_count; i++) {
        if (!is_private(store->chats->chats_arr[i]->type) && store->chats->chats_arr[i]->avatar_id) {
            load_chat_image(i, store);
        }
    }

    // load last message
    load_last_messages(store);
}

void load_last_messages(Store *store) {
    for (int i = 0; i < store->chats->chats_count; i++) {
        api_get_messages_async(load_last_messages_cb, (void *)store->chats->chats_arr[i]->_id, store->chats->chats_arr[i]->_id, 0, 1);
    }
}

void load_last_messages_cb(RCResponse *response, void *data) {
    const char *chat_str_id = (const char *)data;

    Store *store = get_store();
    if (response->code == SUCCESS) {
        bson_error_t bson_error;
        bson_t *bson_messages = bson_new_from_json(response->body, -1, &bson_error);
        if (!bson_messages) {
            printf("Bson error %s\n", bson_error.message);
            return;
        }

        message_t *message = init_new_message(INIT_MESSAGE_CHAT_KEY, bson_messages, 0);

        int chat_id = find_chat_index_by_id(store, chat_str_id);

        if (!message->chat_id) {
            free(message);
            message = NULL;
        }

        int result = add_message_to_store_last_message(message, store);

        if (chat_id != -1) {
            if (result != -1) {
                update_last_message(store, message);
            }

            // set chat position
            long position;

            if (message) {
                position = get_age(message->updated_at);
            } else {
                position = get_age(store->chats->chats_arr[chat_id]->updated_at);
            }
            store->chats->chats_arr[chat_id]->pack_position = (int)position;
            reorder_all_loaded_chats(store);
        }
        bson_destroy(bson_messages);
    } else {
        mx_printstr("RESPONCE FAILED\n");
    }
}

void update_last_message(Store *store, message_t *message) {
    int chat_id = find_chat_index(store, message);
    if (chat_id == -1) {
        free_message(message);
        return;
    }

    const char *last_message_str = trim_last_message(store->chats->chats_arr[chat_id]->last_message->text);
    free_last_message(store->chats->chats_arr[chat_id]->last_message_str);
    gtk_label_set_text(GTK_LABEL(store->chats->chats_arr[chat_id]->gtk_last_message), last_message_str);
    store->chats->chats_arr[chat_id]->last_message_str = last_message_str;
}

int add_message_to_store_last_message(message_t *message, Store *store) {
    if (!message)
        return -1;
    int chat_id = find_chat_index(store, message);
    if (chat_id == -1) {
        free_message(message);
        return -1;
    }
    store->chats->chats_arr[chat_id]->last_message = message;
    return 0;
}

void *load_participants(Store *store) {
    load_users_t *users = malloc(sizeof(load_users_t));
    users->all_users = NULL;
    users->all_users_count = 0;
    for (int i = 0; i < store->chats->chats_count; i++) {
        for (int j = 0; j < store->chats->chats_arr[i]->count_participants; j++) {
            if (!is_user_loaded(users, store->chats->chats_arr[i]->participants_id[j])) {
                RCResponse *response = api_get_user_by_id(store->chats->chats_arr[i]->participants_id[j]);
                if (response->code == SUCCESS) {
                    add_user_to_all_users(users, response);
                }
                RC_response_destroy(response);
            }
        }
    }

    return users;
}

void add_user_to_all_users(load_users_t *users, RCResponse *response) {
    if (response->code == SUCCESS) {
        bson_error_t bson_error;
        bson_t *bson_user_info = bson_new_from_json(response->body, -1, &bson_error);
        if (!bson_user_info) {
            printf("Bson error %s\n", bson_error.message);
            return;
        }

        if (users->all_users_count == 0) {
            users->all_users = malloc(sizeof(user_entry_t *));
            users->all_users_count++;
        } else {
            users->all_users_count++;
            users->all_users = realloc(users->all_users, sizeof(user_entry_t *) * users->all_users_count);
        }

        users->all_users[users->all_users_count - 1] = fill_participant(bson_user_info);

        bson_destroy(bson_user_info);
    }
}

user_entry_t *fill_participant(bson_t *bson_user_info) {
    user_entry_t *user_entry = malloc(sizeof(user_entry_t));
    user_entry->avatar_id = prepare_string(bson_get_str(bson_user_info, "avatar_id"));
    user_entry->bio = prepare_string(bson_get_str(bson_user_info, "bio"));
    user_entry->email = prepare_string(bson_get_str(bson_user_info, "email"));
    user_entry->id = prepare_string(bson_get_str(bson_user_info, "_id"));
    user_entry->name = prepare_string(bson_get_str(bson_user_info, "name"));
    user_entry->nickname = prepare_string(bson_get_str(bson_user_info, "nickname"));

    user_entry->avabuf = NULL;
    return user_entry;
}

void load_participants_cb(load_users_t *users, void *data) {
    Store *store = (Store *)data;
    for (int i = 0; i < users->all_users_count; i++) {
        if (store->all_users_count == 0) {
            store->all_users_count++;
            store->all_users = malloc(sizeof(user_entry_t));
        } else {
            store->all_users_count++;
            store->all_users = realloc(store->all_users, sizeof(user_entry_t) * store->all_users_count);
        }
        store->all_users[store->all_users_count - 1] = users->all_users[i];
    }

    for (int i = 0; i < store->chats->chats_count; i++) {
        refresh_chat_data(i, store);
        refresh_chat_ui(i, store);
    }

    // send image update thread
    for (int i = 0; i < store->all_users_count; i++) {
        if (store->all_users[i]->avatar_id) {
            GThread *load_user_image = api_get_file_async(load_user_image_cb, store->all_users[i], store->all_users[i]->avatar_id);
            g_thread_unref(load_user_image);
        }
    }
}

void load_user_image_cb(RCResponse *response, void *data) {
    user_entry_t *user = (user_entry_t *)data;
    Store *store = get_store();
    if (response->code == SUCCESS) {
        guchar *gudata = ((guchar *)((char *)(response->body)));
        GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
        GError *error = NULL;

        if (!gdk_pixbuf_loader_write(loader, gudata, response->body_len, &error)) {
            printf("Error:\n%s\n", error->message);
        }

        user->pixbufloader = loader;
        user->avabuf = gdk_pixbuf_loader_get_pixbuf(loader);

        refresh_private_chats_image(store, user);
        refresh_group_member_image(store, user);
        refresh_setting_image(store, user);
    } else {
        mx_printstr("load image resp failed\n");
    }

    RC_response_destroy(response);
}

void refresh_group_member_image(Store *store, user_entry_t *user) {
    chat_entry *chat = NULL;
    for (int i = 0; i < store->chats->chats_count; i++) {
        chat = store->chats->chats_arr[i];
        if (!is_private(chat->type)) {
            for (int j = 0; j < chat->count_participants; j++) {
                if (mx_streq(chat->active_entry->members_ui[j]->member_id, user->id)) {
                    chat->active_entry->members_ui[j]->avatar_handler = widget_redrow(50, 50, user->avabuf,
                                                                                      chat->active_entry->members_ui[j]->member_avatar, chat->active_entry->members_ui[j]->avatar_handler);
                }
            }
        }
    }
}

void refresh_setting_image(Store *store, user_entry_t *user) {
    if (mx_streq(store->whoami, user->id)) {
        GtkWidget *setting_user_avatar = GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsUserAvatar"));
        store->user_info->avatar_signal_id = widget_redrow(70, 70, user->avabuf, setting_user_avatar, store->user_info->avatar_signal_id);

        GtkWidget *user_avatar = GTK_WIDGET(gtk_builder_get_object(store->builder, "UserAvatar"));
        store->user_info->avatar_set_signal_id = widget_redrow(70, 70, user->avabuf, user_avatar, store->user_info->avatar_set_signal_id);
    }
}

void refresh_private_chats_image(Store *store, user_entry_t *user) {
    all_chats_t *chats = store->chats;
    chat_entry *chat = NULL;
    for (int i = 0; i < chats->chats_count; i++) {
        chat = chats->chats_arr[i];
        if (is_private(chat->type) && !mx_streq(store->whoami, user->id) && is_user_in_chat(chat, user->id)) {
            chat->avatar_handler_id = widget_redrow(70, 70, user->avabuf, chat->gtk_chat_avatar, chat->avatar_handler_id);
            chat->active_entry->handler_id = widget_redrow(180, 180, user->avabuf, chat->active_entry->chat_image,
                                                           chat->active_entry->handler_id);
        }
    }
}

bool is_user_loaded(load_users_t *users, const char *user_id) {
    if (!users->all_users)
        return false;

    for (int i = 0; i < users->all_users_count; i++) {
        if (mx_streq(users->all_users[i]->id, user_id))
            return true;
    }

    return false;
}

void refresh_chat_ui(int chat_id, Store *store) {
    chat_entry *chat = store->chats->chats_arr[chat_id];
    load_chat_name(chat);
    load_chat_info_name(chat_id, store);
    load_chat_info_bio(chat);

    load_chat_sec_label(chat_id, store);
}

void refresh_chat_data(int chat_id, Store *store) {
    chat_entry *chat = store->chats->chats_arr[chat_id];

    /*FOR PRIVATE CHAT*/
    if (is_private(chat->type)) {

        int sec = get_second_participant_id(store, chat);
        int user_index = get_user_index(store, chat->participants_id[sec]);
        // refresh image
        if (user_index == -1) {
            chat->avatar_id = NULL;
        } else {
            if (store->all_users[user_index]->avatar_id)
                chat->avatar_id = mx_strdup(store->all_users[user_index]->avatar_id);
        }
        // refresh name
        if (user_index == -1) {
            chat->name = mx_strdup("User deleted");
            chat->description = mx_strdup("");
        } else {
            if (store->all_users[user_index]->name)
                chat->name = mx_strdup(store->all_users[user_index]->name);
            if (store->all_users[user_index]->bio)
                chat->description = mx_strdup(store->all_users[user_index]->bio);
        }

        // refresh info sec label
        if (user_index == -1) {
            chat->active_entry->chat_info_sec_label_text = mx_strdup("User deleted");
        } else {
            chat->active_entry->chat_info_sec_label_text = mx_strdup(store->all_users[user_index]->email);
        }
    } else {
        /*FOR GROUP*/

        // refresh info sec label
        string_t count_partipiants = mx_itoa(chat->count_participants);
        chat->active_entry->chat_info_sec_label_text = mx_strjoin(count_partipiants, " members");
        mx_strdel(&count_partipiants);

        // refresh chat members label
        for (int i = 0; i < chat->count_participants; i++) {
            int user_i = get_user_index(store, chat->active_entry->members_ui[i]->member_id);
            if (user_i != -1) {
                gtk_label_set_text(GTK_LABEL(chat->active_entry->members_ui[i]->name_label), store->all_users[user_i]->name);
            } else {
                gtk_label_set_text(GTK_LABEL(chat->active_entry->members_ui[i]->name_label), "User deleted");
            }
        }
    }
}
