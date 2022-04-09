#include "handlers.h"

static int64_t last_sended = 0;

static int count_search = 0;
static search_chats_t **search_chats = NULL;

static void clear_list(void) {
    if (count_search != 0) {
    for (int i = 0; i < count_search; i++) {
        if (search_chats[i]) {
            gtk_widget_destroy(search_chats[i]->ui->event_box);
        }
    }
    search_chats = NULL;
    count_search = 0;
    }
}


void chat_search_handler(GtkSearchEntry *entry, Store *store) {
    (void)store;
    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));

    participants_search_ctx_t *ctx = (participants_search_ctx_t *)malloc(sizeof(participants_search_ctx_t));
    ctx->sended_at = get_current_time();

    if (strlen(text)) {
        g_thread_unref(api_search_async(chat_search_cb, (void *)ctx, text));
    }
    else {
        clear_list();
    }
}

void chat_search_cb(RCResponse *res, void *ctx) {
    int64_t sended_at = ((participants_search_ctx_t *)ctx)->sended_at;
    free(ctx);
    if (last_sended > sended_at) {
        RC_response_destroy(res);
        return;
    }
    last_sended = sended_at;

    bson_error_t bson_error;
    bson_t *bson_search = bson_new_from_json(res->body, -1, &bson_error);
    if (!bson_search) {
        printf("Bson error %s\n", bson_error.message);
        return;
    }

    if (res->code == SUCCESS) {
        clear_list();
        //clear old search
        
        //render new
        create_chat_result_list(bson_search);
        for (int i = 0; i < count_search; i++) {
            if (search_chats[i]->name) {
                search_chats[i]->ui = create_result_list_ui(search_chats[i]->name);
                connect_chat_ui(search_chats[i]);
            }
            else {
                search_chats[i]->ui = create_result_list_ui("loading");
            }
            
        }
    }
}

void connect_chat_ui(search_chats_t *search) {
    Store *store = get_store();
    GtkBox *main_box = GTK_BOX(gtk_builder_get_object(store->builder, "BoxChatsResultList"));
    
    gtk_box_pack_start(GTK_BOX(search->ui->box), search->ui->avatar_drawing, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(search->ui->box), search->ui->name_label, 0, 0, 0);
    
    gtk_container_add(GTK_CONTAINER(search->ui->event_box), search->ui->box);

    gtk_box_pack_start(main_box, search->ui->event_box, 0, 0, 0);
    
    // connect button
    if (!search->type) {
        search->ui->event_box_handler = g_signal_connect(G_OBJECT(search->ui->event_box),
                                                    "button_press_event", G_CALLBACK(search_user_event), search);
    }
    else {
        search->ui->event_box_handler = g_signal_connect(G_OBJECT(search->ui->event_box),
                                                    "button_press_event", G_CALLBACK(search_group_event), search);
    }

    // render image
    if (search->type && !is_private(search->type)) {
        int chat_id = find_chat_index_by_id(store, search->id);
        if (chat_id != -1 && store->chats->chats_arr[chat_id]->gtk_chat_avatar_buf) {
            search->ui->avatar_handler = widget_redrow(45, 45, store->chats->chats_arr[chat_id]->gtk_chat_avatar_buf, 
                                                search->ui->avatar_drawing, search->ui->avatar_handler);
        }
        
    }
    //TODO render user image
    if (!search->type) {
        for (int i = 0; i < store->all_users_count; i++) {
            if (mx_streq(store->all_users[i]->id, search->id)) {
                if (store->all_users[i]->avabuf) {
                    search->ui->avatar_handler = widget_redrow(45, 45, store->all_users[i]->avabuf, 
                                                search->ui->avatar_drawing, search->ui->avatar_handler);
                }
                
            }
        }
    }



    gtk_widget_show_all(GTK_WIDGET(main_box));
}

void search_private_event(GtkWidget *widget, GdkEventButton *event, search_chats_t *part) {
    if (!part || !part->id) return;
    (void)widget;
    (void)event;

    Store *store = get_store();
    int chat_index = find_chat_index_by_id(store, part->id);
    // if -1 create new chat
    if (chat_index != -1) {
        clear_list();
        open_chat(NULL, NULL, store->chats->chats_arr[chat_index]->page);
    }
    else {
        mx_printstr("ERROR -1 in find group\n");
    }
}


void search_group_event(GtkWidget *widget, GdkEventButton *event, search_chats_t *part) {
    if (!part || !part->id) return;
    (void)widget;
    (void)event;

    Store *store = get_store();
    int chat_index = find_chat_index_by_id(store, part->id);
    // if -1 create new chat
    if (chat_index != -1) {
        clear_list();
        open_chat(NULL, NULL, store->chats->chats_arr[chat_index]->page);
    }
    else {
        mx_printstr("ERROR -1 in find group\n");
    }
}

void search_user_event(GtkWidget *widget, GdkEventButton *event, search_chats_t *part) {
    if (!part || !part->id) return;
    (void)widget;
    (void)event;

    Store *store = get_store();
    int chat_index = find_private_chat_by_second_member(store, part->id);
    // if -1 create new chat
    if (chat_index == -1) {
        create_private_chat(store, part->id);
    }
    else {
        clear_list();
        open_chat(NULL, NULL, store->chats->chats_arr[chat_index]->page);
    }
}

void create_private_chat(Store *store ,const char *id) {
    if (!id) return;
    
    bson_t *bson_chat = bson_new();
    bson_t *bson_array = bson_new();

    BSON_APPEND_UTF8(bson_chat, "type", "private");
    BSON_APPEND_UTF8(bson_chat, "admin_id", store->whoami);
    
    //fill array
    BSON_APPEND_UTF8(bson_array, "0", store->whoami);
    BSON_APPEND_UTF8(bson_array, "1", id);
    
    BSON_APPEND_ARRAY(bson_chat, "participants", bson_array);
    
    g_thread_unref(api_create_team_async(create_private_chat_cb, store, bson_chat));
    
    bson_destroy(bson_array);
}

void create_private_chat_cb(RCResponse *res, void *_) {
    Store *store = (Store *)_;

    clear_list();
    if (res->code == SUCCESS) {

        GtkNotebook *notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "MainPages");
        gtk_notebook_set_current_page(notebook, 0);

    } else {
        mx_printstr("resp_code != 200 in create private chat\n");
    }
    
    return;
}

void create_chat_result_list(bson_t *bson_search) {
    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, bson_search, "results") || !BSON_ITER_HOLDS_ARRAY(&iter))
        return;
    
    const bson_value_t *array = bson_iter_value(&iter);
    bson_t *results = bson_new_from_data(array->value.v_doc.data, array->value.v_doc.data_len);
    
    bson_iter_t array_iter;
    if (bson_iter_init(&array_iter, results)) {
        while (bson_iter_next(&array_iter)) {
            
            const bson_value_t *doc = bson_iter_value(&array_iter);
            bson_t *bson_data = bson_new_from_data(doc->value.v_doc.data, doc->value.v_doc.data_len);

            add_entry_to_search(bson_data);
            bson_destroy(bson_data);
        }
        
        bson_destroy(results);
    }
}

void add_entry_to_search(bson_t *bson_data) {
    if (count_search == 0) {
        count_search++;
        search_chats = malloc(sizeof(search_chats_t *) * count_search);
    }
    else {
        count_search++;
        search_chats = realloc(search_chats, sizeof(search_chats_t *) * count_search);
    }

    search_chats[count_search - 1] = malloc(sizeof(search_chats_t));
    search_chats[count_search - 1]->id = prepare_string(bson_get_str(bson_data, "_id"));
    search_chats[count_search - 1]->type = prepare_string(bson_get_str(bson_data, "type"));
    search_chats[count_search - 1]->name = prepare_string(bson_get_str(bson_data, "name"));
}
