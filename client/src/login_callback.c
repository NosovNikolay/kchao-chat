#include "api/api.h"
#include "chat/chats.h"
#include "handlers/handlers.h"
#include "uchat.h"

void user_autologin(Store *store, users_data *data) {
    if (!data)
        return;
    api_authorize(data->users->token[data->current_user]);
    g_thread_unref(api_get_user_async(autologin_callback, (void *)store, data->users->user_id[data->current_user]));
}

void autologin_callback(RCResponse *response, void *data) {
    if (response->code != SUCCESS) {
        RC_response_destroy(response);
        return;
    }

    Store *store = (Store *)data;

    // set user_data
    bson_error_t bson_error;
    bson_t *bson_response = bson_new_from_json(response->body, -1, &bson_error);
    if (!bson_response) {
        printf("Bson error %s\n", bson_error.message);
        return;
    }

    store->whoami = prepare_string(bson_get_str(bson_response, "_id"));
    if (store->whoami == NULL)
        mx_printerr("whoami failed");

    // change page
    go_2_chat(store);

    // load chats
    login_init_chats(store, bson_response);
    // load user info
    load_user_info(store, bson_response);
    //
    set_settings_user_info(store);
    bson_destroy(bson_response);

    // load full chat info
    load_chats(store);

    GtkWidget *chat_avatar = GTK_WIDGET(gtk_builder_get_object(store->builder, "ChatAvatar"));
    gtk_widget_set_size_request(GTK_WIDGET(chat_avatar), 120, 120);
    gulong handler_id = g_signal_connect(G_OBJECT(chat_avatar), "draw", G_CALLBACK(draw_default_avatar), (int *)120);
    store->user_info->avatar_team_id = handler_id;

    store->is_updatable = true;
    g_thread_unref(g_thread_new("updates", run_updates, NULL));

    RC_response_destroy(response);
}

void login_init_chats(Store *store, bson_t *bson_user) {
    chat_preload(store, bson_user);
}
