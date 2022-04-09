#include "chats.h"

message_t *message_new_from_bson(bson_t *bson_message) {
    message_t *message = (message_t *)malloc(sizeof(message_t));
    message->chat_id = prepare_string(bson_get_str(bson_message, "chat_id"));
    message->type = prepare_string(bson_get_str(bson_message, "type"));
    message->text = prepare_string(bson_get_str(bson_message, "text"));
    message->message_id = prepare_string(bson_get_str(bson_message, "_id"));
    message->updated_at = bson_get_int64_t(bson_message, "updated_at") / 1000;
    message->is_edited = bson_get_bool(bson_message, "edited");
    message->from_user = prepare_string(bson_get_str(bson_message, "from_user"));
    return message;
}

message_t *init_new_message(const char *first_key, bson_t *update_data, int index) {
    string_t tmp = NULL;

    string_t str_index = mx_itoa(index);
    tmp = mx_strjoin(first_key, "type");
    string_t type_key = mx_replace_substr(tmp, "{index}", str_index);
    mx_strdel(&tmp);
    tmp = mx_strjoin(first_key, "_id");
    string_t id_key = mx_replace_substr(tmp, "{index}", str_index);
    mx_strdel(&tmp);
    tmp = mx_strjoin(first_key, "text");
    string_t text_key = mx_replace_substr(tmp, "{index}", str_index);
    mx_strdel(&tmp);
    tmp = mx_strjoin(first_key, "edited");
    string_t is_edited_key = mx_replace_substr(tmp, "{index}", str_index);
    mx_strdel(&tmp);
    tmp = mx_strjoin(first_key, "from_user");
    string_t from_user_key = mx_replace_substr(tmp, "{index}", str_index);
    mx_strdel(&tmp);
    tmp = mx_strjoin(first_key, "chat_id");
    string_t chat_id_key = mx_replace_substr(tmp, "{index}", str_index);
    mx_strdel(&tmp);
    tmp = mx_strjoin(first_key, "updated_at");
    string_t updated_at_key = mx_replace_substr(tmp, "{index}", str_index);
    mx_strdel(&tmp);

    message_t *message = (message_t *)malloc(sizeof(message_t));

    message->chat_id = prepare_string(bson_get_str(update_data, chat_id_key));
    message->type = prepare_string(bson_get_str(update_data, type_key));
    message->text = prepare_string(bson_get_str(update_data, text_key));
    message->message_id = prepare_string(bson_get_str(update_data, id_key));
    // #TODO add bson_get_bool
    message->updated_at = bson_get_int64_t(update_data, updated_at_key) / 1000;
    message->is_edited = false;
    message->from_user = prepare_string(bson_get_str(update_data, from_user_key));

    mx_strdel(&str_index);
    mx_strdel(&type_key);
    mx_strdel(&id_key);
    mx_strdel(&text_key);
    mx_strdel(&is_edited_key);
    mx_strdel(&from_user_key);
    mx_strdel(&chat_id_key);
    return message;
}

void add_message_to_chat(chat_entry *chat, message_t *message) {
    if (chat->messages_history == NULL) {
        // init message array;
        chat->count_messages++;
        chat->messages_history = (message_t **)malloc(sizeof(message_t *) * chat->count_messages);

        chat->messages_history[chat->count_messages - 1] = message;
        return;
    }
    // add message to array
    chat->count_messages++;
    chat->messages_history = (message_t **)realloc(chat->messages_history, sizeof(message_t *) * chat->count_messages);
    chat->messages_history[chat->count_messages - 1] = message;
}

void add_message_to_ui(chat_entry *chat, message_t *message, int pack_type) {
    gtk_actve_chat_entry_t *active = chat->active_entry;

    message_ui_t *message_ui = create_ui_message(message, true);

    Store *store = get_store();

    char *sender_name = NULL;
    for (int i = 0; i < store->all_users_count; i++)
        if (mx_streq(store->all_users[i]->id, message->from_user)) {
            sender_name = (char *)store->all_users[i]->name;
        }

    gtk_label_set_text(GTK_LABEL(message_ui->name_label), sender_name ? sender_name : message->from_user);

    message->message_ui = message_ui;

    message_add_style(message);

    if (pack_type == GTK_PACK_START) {
        gtk_box_pack_start(GTK_BOX(active->message_box), message_ui->main_box, 0, 0, 0);
    } else {
        gtk_box_pack_end(GTK_BOX(active->message_box), message_ui->main_box, 0, 0, 0);
    }
    g_signal_connect(message_ui->run_button, "clicked", G_CALLBACK(run_message_button_handler), message);
    gtk_widget_show_all(active->message_box);
}

message_ui_t *create_ui_message(message_t *message, bool is_sended) {
    if (message == NULL)
        return NULL;

    message_ui_t *message_ui = malloc(sizeof(message_ui_t));

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *name_label = gtk_label_new("");
    GtkWidget *body_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *run_button = gtk_button_new();
    GtkWidget *run_button_img = NULL;

    if (message_is_runable(message->type)) {
        gtk_widget_set_classname(run_button, "run_button");
        RunConfig *conf = (RunConfig *)dict_get(get_store()->run_configuration, get_lang_prefix(message->type));
        gtk_widget_set_classname(run_button, !conf || !conf->runner || !strlen(conf->runner) ? "not_configured" : "configured");
    }
    gtk_widget_set_valign(run_button, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(body_box), run_button, false, false, 0);

    GtkWidget *text_label = gtk_label_new(message->text);

    gtk_label_set_max_width_chars(GTK_LABEL(text_label), 50);
    gtk_label_set_line_wrap_mode(GTK_LABEL(text_label), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_line_wrap(GTK_LABEL(text_label), true);
    gtk_label_set_selectable(GTK_LABEL(text_label), TRUE);

    gtk_label_set_xalign(GTK_LABEL(text_label), 0.5f);
    gtk_label_set_yalign(GTK_LABEL(text_label), 0.5f);

    gtk_box_pack_start(GTK_BOX(body_box), text_label, false, false, 0);

    gtk_box_pack_start(GTK_BOX(main_box), name_label, true, true, 0);
    gtk_box_pack_start(GTK_BOX(main_box), body_box, true, true, 0);

    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(info_box, GTK_ALIGN_END);

    GtkWidget *sent_status_img = gtk_image_new();
    if (is_sended) {
        gtk_image_set_from_file(GTK_IMAGE(sent_status_img), SENDED_MSG_IMAGE);
    }

    gtk_box_pack_start(GTK_BOX(info_box), sent_status_img, false, false, 0);

    string_t message_time = get_message_time(message->updated_at);
    GtkWidget *time_label = gtk_label_new(message_time);
    gtk_box_pack_start(GTK_BOX(info_box), time_label, false, false, 0);

    gtk_box_pack_start(GTK_BOX(main_box), info_box, true, true, 0);

    message_ui->body_box = body_box;
    message_ui->main_box = main_box;
    message_ui->info_box = info_box;
    message_ui->run_button = run_button;
    message_ui->run_button_img = run_button_img;
    message_ui->sent_status_img = sent_status_img;
    message_ui->text_label = text_label;
    message_ui->time_label = time_label;
    message_ui->name_label = name_label;

    return message_ui;
}

void message_add_style(message_t *message) {
    Store *store = get_store();

    GtkStyleContext *main_context = gtk_widget_get_style_context(message->message_ui->main_box);
    GtkStyleContext *body_context = gtk_widget_get_style_context(message->message_ui->body_box);
    GtkStyleContext *text_context = gtk_widget_get_style_context(message->message_ui->text_label);

    if (message->message_id == NULL || mx_streq(message->message_id, UNSER_SENDED)) {
        gtk_style_context_add_class(body_context, "Unconfirmed");
        gtk_widget_set_halign(message->message_ui->main_box, GTK_ALIGN_END);
        gtk_widget_set_halign(message->message_ui->name_label, GTK_ALIGN_END);
    } else if (mx_streq(store->whoami, message->from_user)) {
        gtk_style_context_remove_class(body_context, "Unconfirmed");
        gtk_style_context_add_class(body_context, "MyMessage");
        gtk_widget_set_halign(message->message_ui->main_box, GTK_ALIGN_END);
        gtk_widget_set_halign(message->message_ui->name_label, GTK_ALIGN_END);
    } else {
        gtk_style_context_add_class(body_context, "EnemyMessage");
        gtk_widget_set_halign(message->message_ui->main_box, GTK_ALIGN_START);
        gtk_widget_set_halign(message->message_ui->info_box, GTK_ALIGN_START);
        gtk_widget_set_halign(message->message_ui->name_label, GTK_ALIGN_START);
    }
    gtk_style_context_add_class(main_context, "Message");
    gtk_style_context_add_class(body_context, "MessageBody");
    gtk_style_context_add_class(text_context, "MessageText");
    gtk_widget_set_valign(message->message_ui->main_box, GTK_ALIGN_START);
    gtk_widget_set_classname(message->message_ui->name_label, "MessageSender");
}

gboolean apply_message_event(gpointer message_ptr) {
    message_t *message = (message_t *)message_ptr;
    Store *store = get_store();
    int chat_index = find_chat_index_by_id(store, message->chat_id);
    if (chat_index == -1)
        return FALSE;
    chat_entry *chat = store->chats->chats_arr[chat_index];

    // set last message
    free_last_message(chat->last_message_str);
    chat->last_message = message;
    const char *last_message = trim_last_message(message->text);
    chat->last_message_str = last_message;
    gtk_label_set_text(GTK_LABEL(chat->gtk_last_message), last_message);

    message_t *exist_message = get_message_in_chat(chat, message);

    if (exist_message != NULL) {
        message_add_style(exist_message);
        exist_message->updated_at = message->updated_at;
        string_t message_time = get_message_time(exist_message->updated_at);
        gtk_label_set_text(GTK_LABEL(exist_message->message_ui->time_label), message_time);
        free(message_time);
    }
    if (chat->is_loaded && !exist_message) {
        add_message_to_chat(chat, message);
        add_message_to_ui(chat, message, GTK_PACK_START);
    }

    // move chat to up
    update_chats_pack_position(store);
    reorder_all_loaded_chats(store);

    return FALSE;
}

message_t *get_message_in_chat(chat_entry *chat, message_t *message) {
    if (!message || !chat)
        return NULL;

    for (int i = 0; i < chat->count_messages; i++) {
        if (mx_streq(chat->messages_history[i]->message_id, message->message_id))
            return chat->messages_history[i];
    }
    return NULL;
}
