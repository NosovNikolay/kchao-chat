#include <uchat.h>

static gboolean cache_updater(gpointer str) {
    gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(get_store()->builder, "LabelCacheSize")), (string_t)str);
    free(str);
    return FALSE;
}

static gpointer cache_watcher(gpointer _) {
    (void)_;
    while (true) {
        g_idle_add(cache_updater, (gpointer)human_size(count_cache_size()));
        sleep(1);
    }
}

void start_cache_watcher() {
    g_thread_unref(g_thread_new("cache_watcher", cache_watcher, NULL));
}

size_t count_cache_size() {
    return count_dir_size(CACHE_PATH);
}

void clear_cache() {
    rm_dir(CACHE_PATH);
}
