#pragma once

#include <uchat.h>
#include "../../chat/chats.h"

void create_team_button_handler(GtkWidget *widget, Store *store);
void set_team_avatar(GtkWidget *widget, GdkEventButton *event, void *size);

void apply_teams_handlers();

void create_result_list(bson_t *bson_search);
void add_user_to_search(user_entry_t *user);
search_partipiants_ui_t *create_result_list_ui(const char *name);
void connect_ui(search_partipiants_t **part);

void select_partipiant(GtkWidget *widget, GdkEventButton *event, search_partipiants_t **part);
search_partipiants_t *add_partipiant_to_selected(search_partipiants_t *part);

void delete_partipiant(GtkWidget *widget, GdkEventButton *event, search_partipiants_t *part);
void fill_user_array(bson_t *array);
void free_selected_box(void);

user_entry_t *user_new_from_bson(bson_t *bson_user);
