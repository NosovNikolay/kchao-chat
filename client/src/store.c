#include "uchat.h"

static Store _store;

Store* init_store(GtkApplication *app) {
    _store.app = app;
    _store.is_fullscreen = 0;
    _store.is_maximized = 0;
    _store.window = NULL;
    _store.chats = NULL;
    return &_store;
}

Store* get_store() {
    return &_store;
}

void destroy_store() {
    g_object_unref(_store.builder);
    g_object_unref(_store.app);
}
