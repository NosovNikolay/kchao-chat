#include "api/api.h"
#include "handlers/handlers.h"
#include "uchat.h"

void load_all_active_users(Store *store, users_data *data) {
    for (int i = 0; i < data->count_users; i++) {
        //if (i == data->current_user) continue;
        mx_printstr("iter");
        api_is_user_active(store, data);
    }
}

void api_is_user_active(Store *store, users_data *data) {
    GThread *autologin_thread = api_get_user_async(create_active_user, (void *)store, data->users->user_id[data->current_user]);
    g_thread_unref(autologin_thread);
}

void create_active_user(RCResponse *response, void *data) {
    Store *store = (Store *)data;
    bson_error_t bson_error;
        bson_t *bson_response = bson_new_from_json(response->body, -1, &bson_error);
        if (!bson_response) {
            printf("Bson error %s\n", bson_error.message);
            return;
        }

    if (response->code == SUCCESS) {
        const char* username = prepare_string(bson_get_str(bson_response, "name")); 

        GtkBox *mainbox = (GtkBox *)gtk_builder_get_object(store->builder, "UsersBox");
        GtkBox *userbox = (GtkBox *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        GtkImage *user_avatar = (GtkImage *)gtk_image_new();
        GtkLabel *username_label = (GtkLabel *)gtk_label_new(username);

        gtk_box_pack_end(userbox, GTK_WIDGET(user_avatar), false, false, 0);
        gtk_box_pack_end(mainbox, GTK_WIDGET(username_label), false, false, 0);
        
        gtk_box_pack_end(mainbox, GTK_WIDGET(userbox), false, false, 0);

    }


    bson_destroy(bson_response);
    return;
}
