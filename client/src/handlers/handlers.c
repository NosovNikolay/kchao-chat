#include "handlers.h"

void btn_window_close_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    g_application_quit(G_APPLICATION(store->app));
}

void btn_window_fullscreen_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    if (!store->user_info)
        return;
    if (gtk_window_is_maximized(GTK_WINDOW(store->window))) {
        gtk_window_unmaximize(GTK_WINDOW(store->window));
        store->is_maximized = 0;
    } else {
        gtk_window_maximize(GTK_WINDOW(store->window));
        store->is_maximized = 1;
    }
}

void btn_window_minimize_handler(GtkWidget *widget, Store *store) {
    (void)store;
    gdk_window_iconify(gtk_widget_get_window(widget));
}

void setting_button_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    if (!store->user_info)
        return;

    GtkNotebook *notebook = (GtkNotebook *)gtk_builder_get_object(store->builder, "MainNotebook");

    gint index = gtk_notebook_get_current_page(notebook);

    if (index != 2) {
        open_settings_handler(widget, store);
        open_user_settings_tab_handler(widget, store);
    } else {
        gtk_notebook_set_current_page(notebook, 1);
        gtk_window_set_resizable(GTK_WINDOW(store->window), TRUE);

        gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsButton")), true);
        gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(store->builder, "ChatsButton")), false);
    }
}

void chats_search_handler(GtkWidget *widget, GdkEventButton *event, Store *store) {
    (void)widget;
    (void)event;
    gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(store->builder, "ChatsSearchMenu")));
    gtk_widget_grab_focus(GTK_WIDGET(gtk_builder_get_object(store->builder, "InputSearchChats")));
}

void apply_root_handlers() {
    Store *store = get_store();
    g_signal_connect(gtk_builder_get_object(store->builder, "WindowExitBtn"), "clicked", G_CALLBACK(btn_window_close_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "WindowExpandBtn"), "clicked", G_CALLBACK(btn_window_fullscreen_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "WindowHideBtn"), "clicked", G_CALLBACK(btn_window_minimize_handler), store);

    g_signal_connect(gtk_builder_get_object(store->builder, "InputSearchChats"), "search-changed", G_CALLBACK(chat_search_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "SettingsButton"), "clicked", G_CALLBACK(setting_button_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "ChatsButton"), "clicked", G_CALLBACK(setting_button_handler), store);
    g_signal_connect(gtk_builder_get_object(store->builder, "EventBoxChatSearch"), "button_press_event", G_CALLBACK(chats_search_handler), store);
}
