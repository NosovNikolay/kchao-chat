#pragma once

#include <uchat.h>

void open_user_settings_tab_handler(GtkWidget *widget, Store *store);

void open_themes_settings_tab_handler(GtkWidget *widget, Store *store);

void open_config_settings_tab_handler(GtkWidget *widget, Store *store);

void open_storage_settings_tab_handler(GtkWidget *widget, Store *store);

void open_shortcuts_settings_tab_handler(GtkWidget *widget, Store *store);

void open_faq_settings_tab_handler(GtkWidget *widget, Store *store);

void open_settings_handler(GtkWidget *widget, Store *store);

void settings_config_select_runner_input(const char *lang_prefix);

void apply_settings_handlers();
