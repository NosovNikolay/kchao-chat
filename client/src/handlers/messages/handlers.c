#include "handlers.h"

static void message_thread_callback(RCResponse *response, void *data) {
    message_t *message = (message_t *)data;

    bson_error_t bson_error;
    bson_t *bson_message = bson_new_from_json(response->body, -1, &bson_error);
    RC_response_destroy(response);

    if (!bson_message) {
        printf("Bson error %s\n", bson_error.message);
        return;
    }

    if (response->code == SUCCESS) {
        message_t *resp_message = message_new_from_bson(bson_message);
        message->message_id = mx_strdup(resp_message->message_id);
        free_message(resp_message);
    }
    bson_destroy(bson_message);
}

void send_message_handler(GtkWidget *widget, Store *store) {
    (void)widget;

    GtkTextView *message_entry = GTK_TEXT_VIEW(gtk_builder_get_object(store->builder, "InputMessage"));

    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(message_entry);

    gtk_text_buffer_get_bounds(buffer, &start, &end);
    string_t message_text = prepare_string(gtk_text_buffer_get_text(buffer, &start, &end, FALSE));
    if (!message_text)
        return;

    gtk_text_buffer_delete(buffer, &start, &end);

    GtkNotebook *chats_notebook = GTK_NOTEBOOK(gtk_builder_get_object(store->builder, "ChatsNotebook"));
    int page = gtk_notebook_get_current_page(chats_notebook);
    const char *chat_id = store->chats->chats_arr[page]->_id;
    message_t *message = malloc(sizeof(message_t));

    message->chat_id = mx_strdup(store->chats->chats_arr[page]->_id);
    message->from_user = mx_strdup(store->whoami);
    message->is_edited = false;
    message->text = message_text;

    GtkWidget *combo_box = GTK_WIDGET(gtk_builder_get_object(store->builder, "SelectLanguage"));
    int current = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));

    message->type = get_message_type(current);

    message->message_id = NULL;

    add_message_to_chat(store->chats->chats_arr[page], message);
    add_message_to_ui(store->chats->chats_arr[page], message, GTK_PACK_START);

    GThread *message_thread = api_send_message_async(message_thread_callback, message, message_text, message->type, chat_id);
    g_thread_unref(message_thread);
}

void run_message_button_handler(GtkWidget *widget, message_t *message) {
    (void)widget;
    Store *store = get_store();
    char *lang = get_lang_prefix(message->type);
    RunConfig *conf = (RunConfig *)dict_get(store->run_configuration, lang);
    if (!conf || !conf->runner || !strlen(conf->runner)) {
        g_message("Language %s not configured", message->type);
        settings_config_select_runner_input(lang);
        return;
    }
    g_signal_connect(G_OBJECT(gtk_widget_get_parent(exec_message(conf->runner, conf->flags, message->text, lang))), "key_press_event", G_CALLBACK(kb_terminal_window_handler), message);
}

void apply_messages_handlers() {
    Store *store = get_store();

    g_signal_connect(gtk_builder_get_object(store->builder, "SendMessage"), "clicked", G_CALLBACK(send_message_handler), store);
}
