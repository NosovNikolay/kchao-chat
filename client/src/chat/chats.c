#include "chats.h"


void load_new_avatar_chat(GtkWidget *widget, GdkEventButton *event, chat_entry *chat) {
    (void)event;
    (void)widget;
    if (!chat->is_loaded) 
        return;
    string_t filename = open_dialog(chat->active_entry->change_image);
    if (!filename)
        return;
    GdkPixbuf *new_image = gdk_pixbuf_new_from_file_at_scale(filename, 180, 180, false, NULL);
    chat->active_entry->handler_id = widget_redrow(180, 180, new_image, chat->active_entry->chat_image, chat->active_entry->handler_id);
    chat->active_entry->changing_image = true;
    size_t len = 0;
    chat->active_entry->img->data = file_2_data(filename, &len);
    chat->active_entry->img->len = len;
    mx_strdel(&filename);
}

void check(RCResponse *res, void *_) {
    if (_)
        free(_);

    mx_printstr((char *)res->body);
}

void edit_chat_handler(GtkButton *save_buttton, chat_entry *chat) {
    (void)save_buttton;

    // TODO add valid check to prevent editing before chat loaded
    if (!chat->is_loaded) 
        return;
    
    if(chat->active_entry->changing_image) {
        chat->active_entry->changing_image = false;
        g_thread_unref(api_set_team_avatar(check, chat->active_entry->img->data, chat->active_entry->img->data, chat->active_entry->img->len, mx_strdup(chat->_id)));
    }
    
    mx_printstr("saved");
    string_t team_name = NULL;
    string_t description = NULL;

    team_name = prepare_string((string_t)gtk_entry_get_text(GTK_ENTRY(chat->active_entry->chat_info_chat_name)));
    description = prepare_string((string_t)gtk_entry_get_text(GTK_ENTRY(chat->active_entry->chat_info_chat_bio)));

    if (!team_name) {
        if (team_name)
            mx_strdel(&team_name);
        if (description)
            mx_strdel(&description);
        return;
    }

    if (mx_streq(team_name, chat->name) && mx_streq(description ? description : "", chat->description)) {
        if (team_name)
            mx_strdel(&team_name);
        if (description)
            mx_strdel(&description);
        return;
    }

    bson_t *team = bson_new();

    BSON_APPEND_UTF8(team, "name", team_name);
    mx_strdel(&team_name);
    BSON_APPEND_UTF8(team, "description", description ? description : "");
    mx_strdel(&description);
    g_thread_unref(api_update_team_async(check, NULL, team, chat->_id));
}

void chat_preload(Store *store, bson_t *user_data) {
    init_chats(store);
    // load chats
    int chats_count = bson_get_int(user_data, "chats_count");
    if (chats_count == BSON_NOT_FOUND) {
        mx_printerr("CRITICAL: count chats not found\n");
        return;
    }
    chat_entry *chat = NULL;
    for (int i = 0; i < chats_count; i++) {
        chat = load_user_chat(i, user_data);
        if (chat) {
            add_chat_to_store(store, chat);
            add_chat_to_ui(i, store);
        }
    }

    user_chats_add_style(store);
    // sort_chats_by_data(store);
}

void init_chats(Store *store) {
    store->chats = malloc(sizeof(all_chats_t));
    store->chats->chats_count = 0;
    store->chats->chats_arr = NULL;
}

// TODO IF CHAT == 0
chat_entry *load_user_chat(int chat_id, bson_t *user_data) {
    Store *store = get_store();
    string_t key = NULL;
    string_t str_iter = NULL;
    chat_entry *chat = NULL;
    const char *value = NULL;

    string_t id_key = "chats.{i}._id";
    string_t type_key = "chats.{i}.type";
    string_t name_key = "chats.{i}.name";
    string_t description_key = "chats.{i}.description";
    string_t avatar_key = "chats.{i}.avatar_id";
    string_t admin_key = "chats.{i}.admin";
    string_t edited_key = "chats.{i}.updated_at";

    str_iter = mx_itoa(chat_id);
    key = mx_replace_substr(id_key, "{i}", str_iter);

    if ((value = (const char *)prepare_string(bson_get_str(user_data, key))) == NULL)
        return NULL;
    chat = (chat_entry *)malloc(sizeof(chat_entry));
    chat->_id = value;
    mx_strdel(&key);

    key = mx_replace_substr(type_key, "{i}", str_iter);
    value = prepare_string(bson_get_str(user_data, key));
    chat->type = value;
    mx_strdel(&key);

    key = mx_replace_substr(name_key, "{i}", str_iter);
    value = prepare_string(bson_get_str(user_data, key));
    chat->name = value;
    mx_strdel(&key);

    key = mx_replace_substr(description_key, "{i}", str_iter);
    value = prepare_string(bson_get_str(user_data, key));
    chat->description = value;
    mx_strdel(&key);

    key = mx_replace_substr(avatar_key, "{i}", str_iter);
    value = prepare_string(bson_get_str(user_data, key));
    chat->avatar_id = value;
    mx_strdel(&key);

    key = mx_replace_substr(admin_key, "{i}", str_iter);
    value = prepare_string(bson_get_str(user_data, key));
    chat->admin = value;
    mx_strdel(&key);

    chat->count_participants = 0;
    chat->participants_id = chat_get_participants(chat, user_data, chat_id);

    int second_id = get_second_participant_id(store, chat);
    if (second_id != -1 && chat->avatar_id == NULL) {
        chat->avatar_id = DEFAULT_AVATAR_ID;
    }

    key = mx_replace_substr(edited_key, "{i}", str_iter);
    chat->updated_at = bson_get_int64_t(user_data, key) / 1000;
    mx_strdel(&key);

    mx_strdel(&str_iter);

    chat->count_messages = 0;
    chat->messages_history = NULL;
    chat->active_entry = NULL;
    chat->count_not_confirmed = 0;
    chat->pack_position = -1;
    chat->last_message = NULL;
    chat->gtk_chat_avatar_buf = NULL;
    return chat;
}

const char **chat_get_participants(chat_entry *chat, bson_t *user_data, int chat_id) {
    chat->participants_id = NULL;
    int i = 0;
    string_t str_i = NULL;
    const char *participant_id = NULL;
    const char **participants_id = NULL;

    string_t str_chat_id = mx_itoa(chat_id);
    string_t first_key = mx_replace_substr(PARTICIPANTS_TYPE_KEY, "{chat_index}", str_chat_id);
    mx_strdel(&str_chat_id);

    while (1) {
        str_i = mx_itoa(i);
        char *key = mx_replace_substr(first_key, "{index}", str_i);
        mx_strdel(&str_i);

        participant_id = prepare_string(bson_get_str(user_data, key));
        mx_strdel(&key);
        if (participant_id == NULL) {
            mx_strdel(&first_key);
            break;
        }
        if (chat->count_participants == 0) {
            chat->count_participants++;
            participants_id = malloc(sizeof(const char **) * chat->count_participants);
        } else {
            chat->count_participants++;
            participants_id = realloc(participants_id, sizeof(const char **) * chat->count_participants);
        }

        participants_id[i] = participant_id;

        i++;
    }
    return participants_id;
}

int add_chat_to_store(Store *store, chat_entry *chat) {
    if (store->chats->chats_count == 0) {
        store->chats->chats_count++;
        store->chats->chats_arr = malloc(sizeof(chat_entry *) * store->chats->chats_count);
        store->chats->chats_arr[store->chats->chats_count - 1] = chat;
        return 0;
    }

    store->chats->chats_count++;
    store->chats->chats_arr = (chat_entry **)realloc(store->chats->chats_arr, sizeof(chat_entry *) * store->chats->chats_count);
    store->chats->chats_arr[store->chats->chats_count - 1] = chat;

    return store->chats->chats_count - 1;
}

void add_chat_to_ui(int chat_id, Store *store) {
    // create gtk box
    // add gtkbox AllUserChats
    GtkWidget *main_chat_box = GTK_WIDGET(gtk_builder_get_object(store->builder, "AllUserChats"));
    GtkWidget *event_box = NULL;
    GtkWidget *chat_box = NULL;
    GtkWidget *chat_avatar = NULL;
    GtkWidget *chat_text_box = NULL;
    GtkWidget *chat_name = NULL;
    GtkWidget *last_message = NULL;
    string_t last_message_str = NULL;
    gulong handler_id;
    event_box = gtk_event_box_new();
    gtk_widget_show(event_box);

    chat_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_show(chat_box);

    chat_avatar = gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(chat_avatar), 70, 70);
    handler_id = g_signal_connect(G_OBJECT(chat_avatar), "draw", G_CALLBACK(draw_default_avatar), (int *)70);
    gtk_box_pack_start(GTK_BOX(chat_box), chat_avatar, 0, 0, 0);
    gtk_widget_show(chat_avatar);
    chat_text_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_end(GTK_BOX(chat_box), chat_text_box, 1, 1, 0);

    gtk_widget_show(chat_text_box);
    gtk_widget_set_size_request(chat_text_box, 180, -1);

    if (mx_streq(store->chats->chats_arr[chat_id]->type, "group")) {
        chat_name = gtk_label_new(store->chats->chats_arr[chat_id]->name);
    } else {
        chat_name = gtk_label_new("loading...");
    }
    gtk_label_set_xalign(GTK_LABEL(chat_name), 0.0f);
    gtk_label_set_ellipsize(GTK_LABEL(chat_name), PANGO_ELLIPSIZE_END);
    gtk_label_set_width_chars(GTK_LABEL(chat_name), 15);
    gtk_box_pack_start(GTK_BOX(chat_text_box), chat_name, 0, 0, 0);
    gtk_widget_show(chat_name);

    last_message_str = mx_strdup("no message yet");
    last_message = gtk_label_new(last_message_str);
    gtk_label_set_xalign(GTK_LABEL(last_message), 0.0f);
    gtk_box_pack_start(GTK_BOX(chat_text_box), last_message, 0, 0, 0);
    gtk_widget_show(last_message);
    gtk_container_add(GTK_CONTAINER(event_box), chat_box);
    gtk_container_add(GTK_CONTAINER(main_chat_box), event_box);
    store->chats->chats_arr[chat_id]->last_message_str = last_message_str;
    store->chats->chats_arr[chat_id]->avatar_handler_id = handler_id;
    store->chats->chats_arr[chat_id]->gtk_chat_avatar = chat_avatar;
    store->chats->chats_arr[chat_id]->gtk_chat_box = chat_box;
    store->chats->chats_arr[chat_id]->gtk_chat_name = chat_name;
    store->chats->chats_arr[chat_id]->gtk_chat_text_box = chat_text_box;
    store->chats->chats_arr[chat_id]->gtk_event_box = event_box;
    store->chats->chats_arr[chat_id]->gtk_last_message = last_message;
    store->chats->chats_arr[chat_id]->gtk_main_chat_box = main_chat_box;
    store->chats->chats_arr[chat_id]->page = malloc(sizeof(int));
    *store->chats->chats_arr[chat_id]->page = chat_id;

    // init chat window
    init_chat_window(store, chat_id);
    store->chats->chats_arr[chat_id]->chat_window_signal = g_signal_connect(G_OBJECT(event_box), "button_press_event", G_CALLBACK(open_chat), store->chats->chats_arr[chat_id]->page);

    // gtk_widget_realize(event_box);
}

void scroll_down_on_resize(GtkAdjustment *adjustment, double *upper_prev) {
    double upper = gtk_adjustment_get_upper(adjustment);
    if (upper == *upper_prev)
        return;
    *upper_prev = upper;
    double page_size = gtk_adjustment_get_page_size(adjustment);
    gtk_adjustment_set_value(adjustment, upper - page_size);
}

void init_chat_window(Store *store, int chat_index) {
    chat_entry *chat = store->chats->chats_arr[chat_index];
    chat->active_entry = malloc(sizeof(gtk_actve_chat_entry_t));
    chat->active_entry->count_messages = 0;
    chat->active_entry->messages = NULL;
    chat->active_entry->img = malloc(sizeof(file_t));
    // create chat_active zone
    GtkWidget *chat_active_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *chat_scroled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(chat_scroled, 1);

    gtk_widget_show(chat_scroled);

    GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_widget_set_classname(message_box, "MessageBox");

    gtk_container_add(GTK_CONTAINER(chat_scroled), message_box);
    gtk_widget_show(message_box);

    gtk_box_pack_end(GTK_BOX(chat_active_box), chat_scroled, true, true, 0);
    gtk_widget_show_all(chat_active_box);

    chat->active_entry->scroll_upper = 0.0;
    GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(chat_scroled));
    chat->active_entry->adjustment = adjustment;
    chat->active_entry->adjustment_signal = g_signal_connect(adjustment, "changed", (GCallback)scroll_down_on_resize, &chat->active_entry->scroll_upper);

    // create chat info
    GtkWidget *chat_info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *event_change_avatar = gtk_event_box_new();
    GtkWidget *chat_avatar = gtk_drawing_area_new();

    gtk_container_add(GTK_CONTAINER(event_change_avatar), chat_avatar);
    gtk_widget_set_size_request(GTK_WIDGET(chat_avatar), 180, 180);
    gulong handler_id = g_signal_connect(G_OBJECT(chat_avatar), "draw", G_CALLBACK(draw_default_avatar), (int *)180);

    gtk_widget_set_halign(chat_avatar, GTK_ALIGN_CENTER);
    GtkStyleContext *box_ctx = gtk_widget_get_style_context(chat_info_box);
    gtk_style_context_add_class(box_ctx, "ChatInfoBox");

    gtk_box_pack_start(GTK_BOX(chat_info_box), event_change_avatar, 0, true, 0);
    gtk_widget_show(chat_avatar);
    GtkWidget *chat_info_chat_name = NULL;
    GtkWidget *chat_info_chat_bio = NULL;
    GtkWidget *chat_info_sec_label = NULL;
    GtkWidget *save_button = NULL;

    chat_info_sec_label = gtk_label_new("loading...");
    gtk_label_set_line_wrap(GTK_LABEL(chat_info_sec_label), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(chat_info_sec_label), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_justify(GTK_LABEL(chat_info_sec_label), GTK_JUSTIFY_CENTER);

    if (!is_private(chat->type) && mx_streq(chat->admin, store->whoami)) {
        chat_info_chat_name = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(chat_info_chat_name), 15);
        gtk_entry_set_text(GTK_ENTRY(chat_info_chat_name), "loading...");

        chat_info_chat_bio = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(chat_info_chat_bio), 15);
        gtk_entry_set_text(GTK_ENTRY(chat_info_chat_bio), "loading...");

        save_button = gtk_button_new_with_label("Save");
        gtk_widget_set_valign(save_button, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(save_button, GTK_ALIGN_CENTER);
        g_signal_connect(G_OBJECT(save_button), "clicked", G_CALLBACK(edit_chat_handler), chat);
        g_signal_connect(G_OBJECT(event_change_avatar), "button_press_event", G_CALLBACK(load_new_avatar_chat), chat);
        gtk_box_pack_start(GTK_BOX(chat_info_box), chat_info_chat_name, 0, true, 0);
        gtk_box_pack_start(GTK_BOX(chat_info_box), chat_info_chat_bio, 0, true, 0);
        gtk_box_pack_start(GTK_BOX(chat_info_box), chat_info_sec_label, 0, true, 0);
        gtk_box_pack_start(GTK_BOX(chat_info_box), save_button, 0, true, 0);
    } else {
        chat_info_chat_name = gtk_label_new("loading...");
        gtk_label_set_line_wrap(GTK_LABEL(chat_info_chat_name), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(chat_info_chat_name), PANGO_WRAP_WORD_CHAR);
        gtk_label_set_justify(GTK_LABEL(chat_info_chat_name), GTK_JUSTIFY_CENTER);
        chat_info_chat_bio = gtk_label_new("loading...");
        gtk_label_set_line_wrap(GTK_LABEL(chat_info_chat_bio), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(chat_info_chat_bio), PANGO_WRAP_WORD_CHAR);
        gtk_label_set_justify(GTK_LABEL(chat_info_chat_bio), GTK_JUSTIFY_CENTER);

        gtk_box_pack_start(GTK_BOX(chat_info_box), chat_info_chat_name, 0, true, 0);
        gtk_box_pack_start(GTK_BOX(chat_info_box), chat_info_chat_bio, 0, true, 0);
        gtk_box_pack_start(GTK_BOX(chat_info_box), chat_info_sec_label, 0, true, 0);
    }
    
    // group members

    //!!! count member ui always eq to count members
    if (!is_private(chat->type)) {
        GtkWidget *members_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        chat->active_entry->members_box = members_box;

        chat->active_entry->members_label = gtk_label_new("Members");
        gtk_box_pack_start(GTK_BOX(chat->active_entry->members_box), chat->active_entry->members_label, 0, true, 0);

        chat->active_entry->members_ui = init_members_ui(chat);
        gtk_box_pack_start(GTK_BOX(chat_info_box), chat->active_entry->members_box, 0, true, 0);

        members_add_style(chat);
    }

    gtk_widget_show_all(chat_info_box);

    GtkNotebook *chats_notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "ChatsNotebook");
    GtkWidget *tab_label = gtk_notebook_get_nth_page(chats_notebook, chat_index);
    gtk_notebook_append_page(chats_notebook, chat_active_box, tab_label);

    GtkNotebook *chat_info_notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "ChatInfoNotebook");
    tab_label = gtk_notebook_get_nth_page(chat_info_notebook, chat_index);
    gtk_notebook_append_page(chat_info_notebook, chat_info_box, tab_label);

    chat->active_entry->chat_scrolled = chat_scroled;
    chat->active_entry->chat_active_box = chat_active_box;
    chat->active_entry->chat_image = chat_avatar;
    chat->active_entry->change_image = event_change_avatar;
    chat->active_entry->chat_info_box = chat_info_box;
    chat->active_entry->chat_users = chat_info_chat_name;
    chat->active_entry->message_box = message_box;
    chat->active_entry->handler_id = handler_id;
    chat->active_entry->chat_info_sec_label = chat_info_sec_label;
    chat->active_entry->chat_info_chat_name = chat_info_chat_name;
    chat->active_entry->chat_info_chat_bio = chat_info_chat_bio;
    chat->active_entry->save_button = save_button;
}

members_ui_t **init_members_ui(chat_entry *chat) {
    members_ui_t **members_ui = malloc(sizeof(members_ui_t *) * chat->count_participants);
    members_ui_t *member_ui = NULL;

    for (int i = 0; i < chat->count_participants; i++) {
        member_ui = malloc(sizeof(members_ui_t));
        member_ui->member_avatar = gtk_drawing_area_new();
        member_ui->member_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        member_ui->name_label = gtk_label_new("loading");
        member_ui->member_id = chat->participants_id[i];
        members_ui[i] = member_ui;

        gtk_widget_set_size_request(GTK_WIDGET(member_ui->member_avatar), 50, 50);
        member_ui->avatar_handler = g_signal_connect(G_OBJECT(member_ui->member_avatar), "draw", G_CALLBACK(draw_default_avatar), (int *)50);
        gtk_widget_show(member_ui->member_avatar);

        gtk_box_pack_start(GTK_BOX(member_ui->member_box), member_ui->member_avatar, false, false, 0);
        gtk_box_pack_start(GTK_BOX(member_ui->member_box), member_ui->name_label, false, false, 0);

        gtk_box_pack_start(GTK_BOX(chat->active_entry->members_box), member_ui->member_box, false, false, 0);
    }

    return members_ui;
}

chat_entry *chat_new_from_bson(bson_t *bson_chat) {
    chat_entry *chat = (chat_entry *)malloc(sizeof(chat_entry));
    chat->type = prepare_string(bson_get_str(bson_chat, "type"));
    chat->name = prepare_string(bson_get_str(bson_chat, "name"));
    chat->avatar_id = prepare_string(bson_get_str(bson_chat, "avatar_id"));
    chat->admin = prepare_string(bson_get_str(bson_chat, "admin"));
    chat->_id = prepare_string(bson_get_str(bson_chat, "_id"));
    chat->description = prepare_string(bson_get_str(bson_chat, "description"));
    chat->updated_at = bson_get_int64_t(bson_chat, "updated_at") / 1000;
    chat->is_loaded = false;
    chat->messages_history = NULL;
    chat->count_messages = 0;
    chat->count_not_confirmed = 0;
    chat->not_confirmed_messages = NULL;
    chat->last_message = NULL;
    bson_t *participiants = get_participants(bson_chat);
    if (participiants) {
        chat->count_participants = 0;
        chat->participants_id = bson_get_str_array(participiants, &chat->count_participants);
    }
    return chat;
}

const char **bson_get_str_array(bson_t *bson_array, int *size) {
    bson_iter_t array_iter;
    const char **arr = NULL;
    uint32_t name_size;

    if (bson_iter_init(&array_iter, bson_array)) {
        while (bson_iter_next(&array_iter)) {
            if (BSON_ITER_HOLDS_UTF8(&array_iter)) {
                if (*size == 0) {
                    arr = malloc(sizeof(const char *));

                    arr[*size] = prepare_string((char *)bson_iter_utf8(&array_iter, &name_size));
                    *size = *size + 1;
                } else {
                    *size = *size + 1;
                    arr = realloc(arr, sizeof(const char *) * *size);
                    arr[*size - 1] = prepare_string((char *)bson_iter_utf8(&array_iter, &name_size));
                }
            }
        }
    }
    return arr;
}

gboolean apply_chat_event(gpointer chat_ptr) {
    chat_entry *chat = (chat_entry *)chat_ptr;
    Store *store = get_store();
    int chat_id = add_chat_to_store(store, chat);
    add_chat_to_ui(chat_id, store);
    user_chats_add_style(store);
    load_chats(store);
    update_chats_pack_position(store);
    reorder_all_loaded_chats(store);
    return FALSE;
}

gboolean apply_chat_changed_event(gpointer chat_ptr) {
    chat_entry *existed_chat = (chat_entry *)chat_ptr;
    Store *store = get_store();

    int chat_index = find_chat_index_by_id(store, existed_chat->_id);
    if (chat_index != -1) {

        load_chat_name(existed_chat);

        if (!is_private(existed_chat->type) && existed_chat->avatar_id) {
            g_thread_unref(api_get_file_async(load_chat_images_cb, (void *)(long)chat_index, existed_chat->avatar_id));
        }

        load_chat_info_bio(existed_chat);

        update_chats_pack_position(store);
        reorder_all_loaded_chats(store);
    }
    return FALSE;
}

void compare_chat_changes(chat_entry *dst, chat_entry *src, int chat_index) {
    if (!is_private(dst->type)) {
        if (!mx_streq(dst->name, src->name)) {
            dst->name = src->name;
            load_chat_name(dst);
        }
        if (src->avatar_id) {
            if (!mx_streq(dst->avatar_id, src->avatar_id)) {
                dst->avatar_id = src->avatar_id;
                g_thread_unref(api_get_file_async(load_chat_images_cb, (void *)(long)chat_index, dst->avatar_id));
            }
        }
    }
}

void update_chats_pack_position(Store *store) {
    for (int i = 0; i < store->chats->chats_count; i++) {
        if (store->chats->chats_arr[i]->pack_position != -1) {
            if (store->chats->chats_arr[i]->last_message) {
                store->chats->chats_arr[i]->pack_position = get_age(store->chats->chats_arr[i]->last_message->updated_at);
            } else {
                store->chats->chats_arr[i]->pack_position = get_age(store->chats->chats_arr[i]->updated_at);
            }
        }
    }
}

void reorder_all_loaded_chats(Store *store) {
    int count_loaded = 0;
    for (int i = 0; i < store->chats->chats_count; i++) {
        if (store->chats->chats_arr[i]->pack_position != -1) {
            count_loaded++;
        }
    }

    int *loaded_arr = malloc(sizeof(int) * count_loaded);
    int *index_arr = malloc(sizeof(int) * count_loaded);

    int index = 0;
    for (int i = 0; i < store->chats->chats_count; i++) {
        if (store->chats->chats_arr[i]->pack_position != -1) {
            loaded_arr[index] = store->chats->chats_arr[i]->pack_position;
            index_arr[index] = i;
            index++;
        }
    }

    int swap;
    for (int i = 0; i < count_loaded - 1; i++) {
        for (int j = 0; j < count_loaded - i - 1; j++) {
            if (loaded_arr[j] > loaded_arr[j + 1]) {
                swap = loaded_arr[j];
                loaded_arr[j] = loaded_arr[j + 1];
                loaded_arr[j + 1] = swap;

                swap = index_arr[j];
                index_arr[j] = index_arr[j + 1];
                index_arr[j + 1] = swap;
            }
        }
    }

    int position = 4;
    for (int i = 0; i < count_loaded; i++) {
        gtk_box_reorder_child(GTK_BOX(store->chats->chats_arr[index_arr[i]]->gtk_main_chat_box),
                              store->chats->chats_arr[index_arr[i]]->gtk_event_box, position);
        position++;
    }

    free(loaded_arr);
    free(index_arr);
}
