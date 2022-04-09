#include "handlers.h"

static char *settings_buttons[] = {
    "EditProfileButton",
    "SettingsThemes",
    "SettingsCodeConfig",
    "SettingsStorage",
    "SettingsShortcuts",
    "SettingsFAQ",
    NULL}; // !Order important

static int get_settings_tab_index(const char *w_name) {
    for (size_t i = 0; settings_buttons[i]; i++)
        if (mx_streq(w_name, settings_buttons[i]))
            return i;
    return -1;
}

static void activate_tab(const char *w_name, GtkBuilder *builder) {
    for (size_t i = 0; settings_buttons[i]; i++)
        gtk_widget_remove_classname(GTK_WIDGET(gtk_builder_get_object(builder, settings_buttons[i])), "active");
    gtk_widget_set_classname(GTK_WIDGET(gtk_builder_get_object(builder, w_name)), "active");
}

static void open_settings_tab(const char *w_name, GtkBuilder *builder) {
    int idx = get_settings_tab_index(w_name);
    if (idx == -1)
        return;
    activate_tab(w_name, builder);
    GtkNotebook *notebook = (GtkNotebook *)gtk_builder_get_object(builder, "SettingsNotebook");
    gtk_notebook_set_current_page(notebook, idx);
}

void open_settings_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    if (!store->user_info)
        return;
    GtkNotebook *notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "MainNotebook");

    gtk_notebook_set_current_page(notebook, 2);

    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsButton")), false);
    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(store->builder, "ChatsButton")), true);
}

static void settings_button_handler(GtkWidget *widget, Store *store) {
    open_settings_handler(widget, store);
    open_settings_tab(gtk_widget_get_name(widget), store->builder);
}

void open_user_settings_tab_handler(GtkWidget *widget, Store *store) {
    open_settings_handler(widget, store);
    open_settings_tab("EditProfileButton", store->builder);
}

void open_themes_settings_tab_handler(GtkWidget *widget, Store *store) {
    open_settings_handler(widget, store);
    open_settings_tab("SettingsThemes", store->builder);
}

void open_config_settings_tab_handler(GtkWidget *widget, Store *store) {
    open_settings_handler(widget, store);
    open_settings_tab("SettingsCodeConfig", store->builder);
}

void open_storage_settings_tab_handler(GtkWidget *widget, Store *store) {
    open_settings_handler(widget, store);
    open_settings_tab("SettingsStorage", store->builder);
}

void open_shortcuts_settings_tab_handler(GtkWidget *widget, Store *store) {
    open_settings_handler(widget, store);
    open_settings_tab("SettingsShortcuts", store->builder);
}

void open_faq_settings_tab_handler(GtkWidget *widget, Store *store) {
    open_settings_handler(widget, store);
    open_settings_tab("SettingsFAQ", store->builder);
}

static char *get_runner_input_name_by_prefix(const char *prefix) {
    if (mx_streqi(prefix, "c++") || mx_streqi(prefix, "cpp")) {
        return "InputCppRunner";
    } else if (mx_streqi(prefix, "c") || mx_streqi(prefix, "clang")) {
        return "InputClangRunner";
    } else if (mx_streqi(prefix, "node") || mx_streqi(prefix, "nodejs") || mx_streqi(prefix, "js")) {
        return "InputNodeRunner";
    } else if (mx_streqi(prefix, "python") || mx_streqi(prefix, "py")) {
        return "InputPythonRunner";
    } else if (mx_streqi(prefix, "shell") || mx_streqi(prefix, "sh")) {
        return "InputShellRunner";
    } else
        return NULL;
}

void settings_config_select_runner_input(const char *lang_prefix) {
    Store *store = get_store();
    open_config_settings_tab_handler(NULL, store);
    gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(store->builder, get_runner_input_name_by_prefix(lang_prefix))));
}

void apply_settings_handlers() {
    Store *store = get_store();
    for (size_t i = 0; settings_buttons[i]; i++)
        g_signal_connect(gtk_builder_get_object(store->builder, settings_buttons[i]), "clicked", G_CALLBACK(settings_button_handler), store);
}
