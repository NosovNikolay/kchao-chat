#include "handlers.h"
#include "../../kb_handler.h"

void user_update_cb(RCResponse *res, void *_) {
    (void)_;
    if (res->code != 200)
        return;
    bson_t *bson_user = NULL;
    bson_error_t bson_error;
    bson_user = bson_new_from_json(res->body, -1, &bson_error);
    if (!bson_user)
        return;
    Store *store = get_store();
    destroy_user_info(store->user_info);
    load_user_info(store, bson_user);
    update_user_settings_info(store);
}

void user_update_avatar(RCResponse *res, void *_) {
    (void)_;
    Store *store = get_store();
    //redrow
    if (res->code == 200) {
        guchar *gudata = ((guchar *)((char *)(store->user_info->avatar_img->data)));
        GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
        GError *error = NULL;
        if (!gdk_pixbuf_loader_write(loader, gudata, store->user_info->avatar_img->len, &error)) { 
            printf("Error:\n%s\n", error->message); 
            return;
        }   
        GdkPixbuf *buf = gdk_pixbuf_loader_get_pixbuf(loader);
        GtkWidget *avatar = GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsUserAvatar"));
        store->user_info->avatar_signal_id = widget_redrow(70, 70 , buf, avatar, store->user_info->avatar_signal_id);
    }
    free(store->user_info->avatar_img->data);
    store->user_info->avatar_img->data = NULL;
    store->user_info->avatar_img->len = 0;
}

void logout_button_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    store->is_updatable = false;
    // stop updates
    GtkNotebook *chats_notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "ChatsNotebook");
    
    GtkNotebook *chats_info_notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "ChatInfoNotebook");

    //free chats
    for (int i = 0; i < store->chats->chats_count; i++) {
        free_chat(store->chats->chats_arr[i]);

        gtk_notebook_remove_page(chats_notebook, store->chats->chats_count - i - 1);
        gtk_notebook_remove_page(chats_info_notebook, store->chats->chats_count - i - 1);

        store->chats->chats_arr[i]->count_not_confirmed = 0;
    }

    store->chats->chats_count = 0;
    // free users
    
    for (int i = 0; i < store->all_users_count; i++) {
        free_user(store->all_users[i]);
    }
    
    store->chats = NULL;
    store->all_users_count = 0;
    store->all_users = NULL;
    store->whoami = NULL;

    free_save_file(PATH_TO_BINARY);

    // move to login
    go_2_auth(store);
}

void settings_user_input_changed(GtkWidget *widget, Store *store) {
    (void)widget;
    if (store->user_info->setting_new_avatar) {
        if (store->user_info->avatar_img->data) {
            g_thread_unref(api_update_user_avatar(user_update_avatar, NULL, store->user_info->avatar_img->data, store->user_info->avatar_img->len));        
        }
        store->user_info->setting_new_avatar = false;
    }
    
    string_t name = prepare_string((string_t)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserNameEntry"))));
    string_t bio = prepare_string((string_t)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserBioEntry"))));
    string_t nickname = prepare_string((string_t)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserNicknameEntry"))));
    
    bson_t *update = bson_new();

    if (!mx_streq(name, store->user_info->name))
        BSON_APPEND_UTF8(update, "name", name);
    if (!mx_streq(bio, store->user_info->bio))
        BSON_APPEND_UTF8(update, "bio", bio);
    if (!mx_streq(nickname, store->user_info->nickname))
        BSON_APPEND_UTF8(update, "nickname", nickname);

    if (name)
        mx_strdel(&name);
    if(bio) 
        mx_strdel(&bio);
    if (nickname)
        mx_strdel(&nickname);
        
    if (!bson_count_keys(update)) {
        bson_destroy(update);
        return;
    }

    g_thread_unref(api_update_user_async_new(user_update_cb, NULL, update));
    bson_destroy(update);
}

void set_user_avatar(GtkWidget *widget, GdkEventButton *event, void *size) {
    (void)event;
    Store *store = get_store();
    string_t filename = open_dialog(widget);
    if (!filename)
        return;
    int size_avatar = (int)size;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(filename, size_avatar, size_avatar, FALSE, NULL);
    store->user_info->avatar_set_signal_id = widget_redrow(size_avatar, size_avatar, pixbuf, widget, store->user_info->avatar_set_signal_id);
    store->user_info->setting_new_avatar = true;
    store->user_info->avatar_img = malloc(sizeof(file_t));
    store->user_info->avatar_img->len = 0;
    store->user_info->avatar_img->data = file_2_data(filename, &store->user_info->avatar_img->len);
    mx_strdel(&filename);
}

void apply_settings_user_handlers(void) {
    Store *store = get_store();
    g_signal_connect(gtk_builder_get_object(store->builder, "UserAvatar"), "button_press_event", G_CALLBACK(set_user_avatar), (void *)70);
    g_signal_connect(gtk_builder_get_object(store->builder, "ButtonSaveUser"), "clicked", G_CALLBACK(settings_user_input_changed), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "ButtonLogOut"), "clicked", G_CALLBACK(logout_button_handler), store);
}
