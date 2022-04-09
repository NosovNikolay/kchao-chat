#include "async.h"

static gboolean thread_stop(gpointer _data) {
    thread_data *data = (thread_data *)_data;
    if (data->callback)
        data->callback(data->res, data->cb_data);
    g_free(data);
    return FALSE;
}

static gpointer thread_func(gpointer _data) {
    thread_data *data = (thread_data *)_data;
    data->res = data->func(data->data);
    g_idle_add(thread_stop, data);
    return NULL;
}

GThread *promise(GThreadFunc func, Callback callback, gpointer user_data, gpointer cb_data) {
    thread_data *data = g_malloc(sizeof(thread_data));
    data->data = user_data;
    data->func = func;
    data->callback = callback;
    data->res = NULL;
    data->cb_data = cb_data;
    return g_thread_new("promise", thread_func, data);
}
