#include "chats.h"

void load_chat_image(int chat_id, Store *store) {
    if (store->chats->chats_arr[chat_id]->avatar_id) {
        GThread *thread = api_get_file_async(load_chat_images_cb, (void *)(long)chat_id, store->chats->chats_arr[chat_id]->avatar_id);
        g_thread_unref(thread);
    }
}

void draw_default_avatar(GtkWidget *widget, cairo_t *cr, int size) {
    (void)widget;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale("client/data/icons/avatars/avatar11.png", size, size, FALSE, NULL);
    gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
    g_object_unref(G_OBJECT(pixbuf));

    double x = 0,
        y = 0,
        width = size,
        height = size,
        aspect = 1.0, 
        corner_radius = height / 2.0;     
    double radius = corner_radius / aspect;
    double degrees = M_PI / 180.0;

    cairo_new_sub_path (cr);
    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    cairo_fill(cr);
}


void draw_user_avatar(GtkWidget *widget, cairo_t *cr, void *data) {
    (void)widget;
    GdkPixbuf *pixbuf = (GdkPixbuf *)data;
    gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);

    double x = 0,
        y = 0,
        width = 70,
        height = 70,
        aspect = 1.0,  
        corner_radius = height / 2.0;
    double radius = corner_radius / aspect;
    double degrees = M_PI / 180.0;

    cairo_new_sub_path (cr);
    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    cairo_fill(cr);
}

void drower(GtkWidget *widget, cairo_t *cr, void *data) {
    (void)widget;
    widget_redrow_t *ctx = data;

    gdk_cairo_set_source_pixbuf(cr, ctx->pixbuf, 0, 0);

    double x = 0,
        y = 0,
        width = ctx->w,
        height = ctx->h,
        aspect = 1.0,  
        corner_radius = height / 2.0;
    double radius = corner_radius / aspect;
    double degrees = M_PI / 180.0;

    cairo_new_sub_path (cr);
    cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    cairo_fill(cr);
}

gulong widget_redrow(int w, int h, GdkPixbuf *pixbuf ,GtkWidget *widget, gulong handler_id) {
    widget_redrow_t *ctx = malloc(sizeof(widget_redrow_t));
    ctx->h = h;
    ctx->w = w;
    ctx->pixbuf = gdk_pixbuf_scale_simple(pixbuf, ctx->w, ctx->h, GDK_INTERP_TILES);

    g_signal_handler_disconnect(widget, handler_id);
    gulong id = g_signal_connect(G_OBJECT(widget), "draw", G_CALLBACK(drower), (void *)ctx);
    gtk_widget_queue_draw(widget);

    //set size request

    gtk_widget_set_size_request(widget, w, h);

    return id;
}

void load_chat_images_cb(RCResponse *response, void *data) {
    int chat_id = (int)data;
    Store *store = get_store();
    chat_entry *chat = store->chats->chats_arr[chat_id]; 

    guchar *gudata = ((guchar *)((char *)(response->body)));
    GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
    GError *error = NULL;

    if (!gdk_pixbuf_loader_write(loader, gudata, response->body_len, &error)) { 
        printf("Error:\n%s\n", error->message); 
    }   
    
    chat->gtk_chat_avatar_buf = gdk_pixbuf_loader_get_pixbuf(loader);

    chat->avatar_handler_id = widget_redrow(70, 70, chat->gtk_chat_avatar_buf, chat->gtk_chat_avatar, chat->avatar_handler_id);
    chat->active_entry->handler_id = widget_redrow(180, 180, chat->gtk_chat_avatar_buf, chat->active_entry->chat_image, chat->active_entry->handler_id);
}

