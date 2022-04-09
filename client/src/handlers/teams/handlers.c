#include "handlers.h"
#include "../../api/api.h"

static int64_t last_sended = 0;

static int count_search = 0;
static search_partipiants_t **search_partipiants = NULL;

static int count_selected = 0;
static search_partipiants_t **selected_partipiants = NULL;

void create_team_button_handler(GtkWidget *widget, Store *store) {
    (void)widget;

    GtkNotebook *notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "MainPages");

    gint index = gtk_notebook_get_current_page(notebook);

    if (index != 1) {
        gtk_notebook_set_current_page(notebook, 1);
    } else {
        gtk_notebook_set_current_page(notebook, 0);
    }
}

void call_back_set_avatar(RCResponse *res, void *_) {
    (void)_;
    Store *store = get_store();
    if (store->user_info->team_avatar && store->user_info->team_avatar->data) {
        free(store->user_info->team_avatar->data);
        store->user_info->team_avatar->data = NULL;
        store->user_info->team_avatar->len = 0;
    }

    if (res->code == 200) {
        return;
    }
}

void free_selected_box(void){
    if (count_selected != 0) {
        for (int i = 0; i < count_selected; i++) {
            if (selected_partipiants[i]) {
                free_user(selected_partipiants[i]->user);
                g_signal_handler_disconnect(selected_partipiants[i]->ui->avatar_drawing,
                                selected_partipiants[i]->ui->avatar_handler);

                g_signal_handler_disconnect(selected_partipiants[i]->ui->event_box, 
                                selected_partipiants[i]->ui->event_box_handler);
                gtk_widget_destroy(selected_partipiants[i]->ui->box);
                free((void *)selected_partipiants[i]);
            }

        }
        free((void *)selected_partipiants);
        selected_partipiants = NULL;
        count_selected = 0;
    }
}

void chat_create_callback(RCResponse *res, void *_) {
    (void)_;
    Store *store = get_store();
    if (res->code == 200) {
        bson_error_t *error = NULL;
        bson_t *chat = bson_new_from_json(res->body, res->body_len, error);
        if (!chat) {
            fprintf(
                stderr, "ERROR: %d.%d: %s\n", error->domain, error->code, error->message);
            return;
        }

        string_t chat_id = prepare_string(bson_get_str(chat, "_id"));
        if (store->user_info->team_avatar) {
            g_thread_unref(api_set_team_avatar(call_back_set_avatar, NULL, store->user_info->team_avatar->data, store->user_info->team_avatar->len, chat_id));
        }
        bson_destroy(chat);
    
        GtkWidget *chat_avatar = GTK_WIDGET(gtk_builder_get_object(store->builder, "ChatAvatar"));
        GdkPixbuf *buff = gdk_pixbuf_new_from_file_at_scale("client/data/icons/avatars/avatar11.png", 120, 120, FALSE, NULL);
        store->user_info->avatar_team_id = widget_redrow(120, 120, buff, chat_avatar, store->user_info->avatar_team_id);
        
        gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "TeamName")), "");
        gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "TeamDescription")), "");

        free_selected_box();
        GtkNotebook *notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "MainPages");
        gtk_notebook_set_current_page(notebook, 0);

    } else {
        printf("%s\n", (char *)res->body);
    }
    
    return;
}

void set_team_avatar(GtkWidget *widget, GdkEventButton *event, void *size) {
    (void)event;
    Store *store = get_store();
    string_t filename = open_dialog(widget);
    if (!filename)
        return;
    int size_avatar = (int)size;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(filename, size_avatar, size_avatar, FALSE, NULL);
    store->user_info->avatar_team_id = widget_redrow(size_avatar, size_avatar, pixbuf, widget, store->user_info->avatar_team_id);
    printf("%s\n", filename);
    size_t len = 0;
    store->user_info->team_avatar->data = file_2_data(filename, &len);
    printf("%zu\n", len);
    store->user_info->team_avatar->len = len;
}

void participants_search_cb(RCResponse *res, void *ctx) {
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
        
    // clear BoxParticipantsResultList
    if (count_search != 0) {
        for (int i = 0; i < count_search; i++) {
            if (search_partipiants[i]) {
                free_user(search_partipiants[i]->user);
                gtk_widget_destroy(search_partipiants[i]->ui->box);
                free((void *)search_partipiants[i]);
            }

        }
        free((void *)search_partipiants);
        search_partipiants = NULL;
        count_search = 0;
    }

    // render new BoxParticipantsResultList
    create_result_list(bson_search);
    for (int i = 0; i < count_search; i++) {
        search_partipiants[i]->ui = create_result_list_ui(search_partipiants[i]->user->name);
        connect_ui(&search_partipiants[i]);
    }
    
    }

    RC_response_destroy(res);
}

void connect_ui(search_partipiants_t **part) {
    Store *store = get_store();
    GtkBox *main_box = GTK_BOX(gtk_builder_get_object(store->builder, "BoxParticipantsResultList"));
    
    gtk_box_pack_start(GTK_BOX((*part)->ui->box), (*part)->ui->avatar_drawing, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX((*part)->ui->box), (*part)->ui->name_label, 0, 0, 0);
    
    gtk_container_add(GTK_CONTAINER((*part)->ui->event_box), (*part)->ui->box);
    
    (*part)->ui->event_box_handler = g_signal_connect(G_OBJECT((*part)->ui->event_box), "button_press_event", G_CALLBACK(select_partipiant), part);

    gtk_box_pack_start(main_box, (*part)->ui->event_box, 0, 0, 0);
    
    for (int i = 0; i < store->all_users_count; i++) {
        if (mx_streq(store->all_users[i]->id, (*part)->user->id)) {
            if (store->all_users[i]->avabuf) {
                (*part)->ui->avatar_handler = widget_redrow(45, 45, store->all_users[i]->avabuf, 
                                            (*part)->ui->avatar_drawing, (*part)->ui->avatar_handler);
            }
        }
    }

    
    gtk_widget_show_all(GTK_WIDGET(main_box));
}


search_partipiants_ui_t *create_result_list_ui(const char *name) {
    search_partipiants_ui_t *ui = malloc(sizeof(search_partipiants_ui_t));

    ui->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    ui->event_box = gtk_event_box_new();
    gtk_widget_set_classname(ui->box, "search_result_item");

    ui->name_label = gtk_label_new(name);
    gtk_widget_set_classname(ui->name_label, "search_result_item_label");

    ui->avatar_drawing = gtk_drawing_area_new();

    gtk_widget_set_size_request(ui->avatar_drawing, 45, 45);
    ui->avatar_handler = g_signal_connect(G_OBJECT(ui->avatar_drawing), "draw", G_CALLBACK(draw_default_avatar), (int *)45);

    return ui;
}

void select_partipiant(GtkWidget *widget, GdkEventButton *event, search_partipiants_t **part) {
    g_signal_handler_disconnect((*part)->ui->event_box, (*part)->ui->event_box_handler);
    (*part)->ui->event_box_handler = -1;
    
    (void)widget;
    (void)event;
    Store *store = get_store();

    GtkBox *main_box = GTK_BOX(gtk_builder_get_object(store->builder, "BoxParticipantsResultList"));
    GtkBox *selected_box = GTK_BOX(gtk_builder_get_object(store->builder, "SelectedPartipiants"));

    search_partipiants_t *new_part = add_partipiant_to_selected(*part);
    
    *part = NULL;

    new_part->ui->event_box_handler = g_signal_connect(G_OBJECT(new_part->ui->event_box),
                                                    "button_press_event", G_CALLBACK(delete_partipiant), new_part);

    gtk_container_remove(GTK_CONTAINER(main_box), new_part->ui->event_box);
    gtk_box_pack_start(selected_box, new_part->ui->event_box, 0, 0, 0);


}

void delete_partipiant(GtkWidget *widget, GdkEventButton *event, search_partipiants_t *part) {
    (void)widget;
    (void)event;

    if (part != NULL) {
        free_user(part->user);
        gtk_widget_destroy(part->ui->box);
        free((void *)(part));
        part = NULL;
    }

}

void create_result_list(bson_t *bson_search) {
    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, bson_search, "results") || !BSON_ITER_HOLDS_ARRAY(&iter))
        return;
    
    const bson_value_t *array = bson_iter_value(&iter);
    bson_t *results = bson_new_from_data(array->value.v_doc.data, array->value.v_doc.data_len);
    
    bson_iter_t array_iter;
    if (bson_iter_init(&array_iter, results)) {
        while (bson_iter_next(&array_iter)) {
            
            const bson_value_t *doc = bson_iter_value(&array_iter);
            bson_t *bson_user = bson_new_from_data(doc->value.v_doc.data, doc->value.v_doc.data_len);

            add_user_to_search(user_new_from_bson(bson_user));
            bson_destroy(bson_user);
        }
        
        bson_destroy(results);
    }
    
}


search_partipiants_t *add_partipiant_to_selected(search_partipiants_t *part) {
    if (count_selected == 0) {
        count_selected++;
        selected_partipiants = malloc(sizeof(search_partipiants_t *) * count_selected);
    }
    else {
        count_selected++;
        selected_partipiants = realloc(selected_partipiants, sizeof(search_partipiants_t *) * count_selected);
    }

    selected_partipiants[count_selected - 1] = part;

    return selected_partipiants[count_selected - 1];
}

void add_user_to_search(user_entry_t *user) {
    if (count_search == 0) {
        count_search++;
        search_partipiants = malloc(sizeof(search_partipiants_t *) * count_search);
    }
    else {
        count_search++;
        search_partipiants = realloc(search_partipiants, sizeof(search_partipiants_t *) * count_search);
    }

    search_partipiants[count_search - 1] = malloc(sizeof(search_partipiants_t));
    search_partipiants[count_search - 1]->user = user;
}

user_entry_t *user_new_from_bson(bson_t *bson_user) {
    user_entry_t *user = (user_entry_t *)malloc(sizeof(user_entry_t));
    user->id = prepare_string(bson_get_str(bson_user, "_id"));
    user->name = prepare_string(bson_get_str(bson_user, "name"));
    user->email = prepare_string(bson_get_str(bson_user, "email"));
    user->nickname = prepare_string(bson_get_str(bson_user, "nickname"));
    user->bio = prepare_string(bson_get_str(bson_user, "bio"));
    user->avatar_id = prepare_string(bson_get_str(bson_user, "avatar_id"));
    user->ava_handler_id = -1;
    user->avabuf = NULL;
    user->pixbufloader = NULL;
    return user;
}

void participants_search_handler(GtkSearchEntry *entry, Store *store) {
    (void)store;
    participants_search_ctx_t *ctx = (participants_search_ctx_t *)malloc(sizeof(participants_search_ctx_t));
    ctx->sended_at = get_current_time();

    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));

    if (strlen(text))
        g_thread_unref(api_search_users_async(participants_search_cb, (void *)ctx, text));
    else {
        if (count_search != 0) {
            for (int i = 0; i < count_search; i++) {
                if (search_partipiants[i]) {
                    free_user(search_partipiants[i]->user);
                    gtk_widget_destroy(search_partipiants[i]->ui->box);
                    free((void *)search_partipiants[i]);
                }

            }
            free((void *)search_partipiants);
            search_partipiants = NULL;
            count_search = 0;
        }
    }
}

void add_participant_button_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(store->builder, "ParticipantsSearchResultsMenu")));
    gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(store->builder, "InputSearchParticipants")));
}

void create_team(GtkWidget *widget, Store *store) {
    (void)widget;

    string_t team_name = NULL;
    string_t description = NULL;
    team_name = prepare_string((string_t)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "TeamName"))));
    description = prepare_string((string_t)gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "TeamDescription"))));
    if (!team_name || !description) {
        // handle error in entry boxes
        gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "TeamName")), "DAVAI PO NOVOI");
        if (team_name)
            mx_strdel(&team_name);
        if (description)
            mx_strdel(&description);
        return;
    }

    bson_t *team = bson_new();
    bson_t *array = bson_new();
    // Fill array
    fill_user_array(array);

    BSON_APPEND_UTF8(team, "name", team_name);
    mx_strdel(&team_name);
    BSON_APPEND_UTF8(team, "description", description);
    mx_strdel(&description);
    BSON_APPEND_UTF8(team, "type", "group");
    BSON_APPEND_UTF8(team, "admin_id", store->whoami);
    BSON_APPEND_ARRAY(team, "participants", array);

    g_thread_unref(api_create_team_async(chat_create_callback, NULL, team));
    bson_destroy(array);
}

void fill_user_array(bson_t *array) {
    int j = 0;
    string_t str_j = NULL;
    for (int i = 0; i < count_selected; i++) {
        if (selected_partipiants[i]) {
            str_j = mx_itoa(j);
            if (selected_partipiants[i]->user->id) {
                BSON_APPEND_UTF8(array, str_j, selected_partipiants[i]->user->id);
            }

            mx_strdel(&str_j);
            j++;
        }
    }
}

void apply_teams_handlers() {
    Store *store = get_store();
    g_signal_connect(gtk_builder_get_object(store->builder, "CreateTeam"), "clicked", G_CALLBACK(create_team_button_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "ButtonAddParticipant"), "clicked", G_CALLBACK(add_participant_button_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "ChatAvatar"), "button_press_event", G_CALLBACK(set_team_avatar), (void *)120);
    g_signal_connect(gtk_builder_get_object(store->builder, "SendCreateTeamInfo"), "clicked", G_CALLBACK(create_team), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "InputSearchParticipants"), "search-changed", G_CALLBACK(participants_search_handler), store);
}
