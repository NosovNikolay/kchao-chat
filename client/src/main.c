#include <uchat.h>
#include "api/api.h"
#include "handlers/apply.h"

static GtkBuilder *init_builder(Store *store, string_t template) {
    GError *error = NULL;
    GtkBuilder *builder = gtk_builder_new();
    string_t path = mx_strjoin(DATA_PATH, template);
    if (!gtk_builder_add_from_file(builder, path, &error)) {
        g_critical("Can`t load template: %s ", error->message);
        g_error_free(error);
    }
    mx_strdel(&path);
    gtk_builder_connect_signals(builder, store);
    return builder;
}

GtkCssProvider *init_css_provider(string_t css_path) {
    GError *error = NULL;
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    if (!gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider), css_path, &error)) {
        g_critical("Can`t load CSS: %s ", error->message);
        g_error_free(error);
    }
    return provider;
}

static void create_window(GtkApplication *app, gpointer data) {
    Store *store = (Store *)data;
    store->builder = init_builder(store, "templates/template.glade");

    gtk_builder_connect_signals(store->builder, store);

    store->window = GTK_WIDGET(gtk_builder_get_object(store->builder, "Window"));
    if (!store->window)
        g_critical("Can`t find window");

    store->css_provider = init_css_provider(THEMES_PATH "Default.css");

    go_2_auth(store);

    gtk_application_add_window(app, GTK_WINDOW(store->window));
    
    apply_handlers();

    gtk_widget_show(store->window);
    users_data *user_data = load_users(PATH_TO_BINARY);
    user_autologin(store, user_data);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    mx_redirect_script_home((char *)argv[0]);
    init_api(argv[1], NULL);
    start_cache_watcher();
    if (argc > 1)
        argc--;
    GtkApplication *app = gtk_application_new("ckchao.chat", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(app, "activate", G_CALLBACK(create_window), init_store(app));
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    destroy_store();
    destroy_api();
    return status;
}
