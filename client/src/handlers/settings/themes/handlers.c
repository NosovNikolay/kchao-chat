#include "handlers.h"

static char *themes[] = {"Default", "Dark", "Light", "Autumn", NULL};

void set_theme(GtkWidget *widget, Store *store) {
    const char *w_name = gtk_widget_get_name(widget);
    char *theme = (char *)w_name + mx_get_char_index(w_name, '_') + 1;
    char buff[100];
    sprintf(buff, "%s%s.css", THEMES_PATH, theme);
    store->css_provider = init_css_provider(buff);
}

void apply_settings_themes_handlers() {
    Store *store = get_store();
    char buff[100];
    for (size_t i = 0; themes[i]; i++) {
        char *theme = themes[i];
        sprintf(buff, "ButtonTheme_%s", theme);
        g_signal_connect(gtk_builder_get_object(store->builder, buff), "clicked", G_CALLBACK(set_theme), store);
    }
}
