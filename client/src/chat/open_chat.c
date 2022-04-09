#include "chats.h"

void open_chat(GtkWidget *widget, GdkEventButton *event, int *page) {
    (void)widget;
    (void)event;
    printf("page = %d\n", *page);
    Store *store = get_store();

    if (!store->chats->chats_arr[*page]->is_loaded) {
        load_chat_history_ctx *ctx = malloc(sizeof(load_chat_history_ctx));
        ctx->chat_id = store->chats->chats_arr[*page]->_id;
        ctx->cur = 0;
        GThread *chat_history_thread = PROMISE(load_chat_history, load_chat_history_cb, ctx, store->chats->chats_arr[*page]->_id);
        store->chats->chats_arr[*page]->is_loaded = true;
        g_thread_unref(chat_history_thread);
    }
    // change main pages notebook
    GtkNotebook *notebook = GTK_NOTEBOOK(gtk_builder_get_object(store->builder, "MainPages"));
    gtk_notebook_set_current_page(notebook, 2);

    // change chats notebook 
    GtkNotebook *chats_notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "ChatsNotebook");
    gtk_notebook_set_current_page(chats_notebook, *page);
    GtkNotebook *chats_info_notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "ChatInfoNotebook");
    gtk_notebook_set_current_page(chats_info_notebook, *page);
    gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(store->builder, "InputMessage")));
}

void *load_chat_history(void *data) {
    load_chat_history_ctx *ctx = (load_chat_history_ctx *)data;
    RCResponse *responce = api_get_messages(ctx->chat_id, ctx->cur, 100);
    free((void *)ctx);

    bson_t *bson_responce = NULL;
    bson_error_t bson_error;

    bson_responce = bson_new_from_json(responce->body, -1, &bson_error);
    RC_response_destroy(responce);
    if (!bson_responce) {
        printf("Bson error %s\n", bson_error.message);
        return NULL;
    }
    if (responce->code == SUCCESS) {
        return bson_responce;
    } else {
        mx_printerr("responce failure\n");
    }
    bson_destroy(bson_responce);

    return NULL;
}

void gtk_widget_reshow(GtkWidget *widget) {
    gtk_widget_hide(GTK_WIDGET(widget));
    gtk_widget_show_all(GTK_WIDGET(widget));
}

void load_chat_history_cb(bson_t *bson_messages, const char *chat_id) {
    if (bson_messages == NULL)
        return;

    Store *store = get_store();

    int chat_index = find_chat_index_by_id(store, chat_id);
    int count_messages = bson_get_int(bson_messages, "messages count");

    message_t *message = NULL;
    const char *last_message = NULL;
    int i = 0;
    for (i = count_messages; i; i--) {
        message = init_new_message(INIT_MESSAGE_CHAT_KEY, bson_messages, i - 1);
        if (message->chat_id == NULL)
            continue;

        add_message_to_chat(store->chats->chats_arr[chat_index], message);

        add_message_to_ui(store->chats->chats_arr[chat_index], message, GTK_PACK_START);


        // set last message
        free_last_message(store->chats->chats_arr[chat_index]->last_message_str);
        last_message = trim_last_message(message->text);
        store->chats->chats_arr[chat_index]->last_message_str = last_message;
        gtk_label_set_text(GTK_LABEL(store->chats->chats_arr[chat_index]->gtk_last_message), last_message);
    }
}
