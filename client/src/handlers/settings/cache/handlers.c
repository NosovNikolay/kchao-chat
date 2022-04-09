#include "handlers.h"

void clear_cache_handler(GtkWidget *widget, Store *store) {
    (void)widget;
    (void)store;
    clear_cache();
}

void apply_settings_cache_handlers() {
    Store *store = get_store();
    g_signal_connect(gtk_builder_get_object(store->builder, "ButtonClearCache"), "clicked", G_CALLBACK(clear_cache_handler), store);
}
