#pragma once

#include <uchat.h>

typedef void (*Callback)(gpointer, gpointer);

typedef struct thread_data_s {
    GThreadFunc func;
    Callback callback;
    gpointer data;
    gpointer res;
    gpointer cb_data;
} thread_data;

GThread *promise(GThreadFunc func, Callback callback, gpointer user_data, gpointer cb_data);

#define PROMISE(func, callback, user_data, cb_data) promise((GThreadFunc)(func), (Callback)callback, (gpointer)user_data, (gpointer)(cb_data))
