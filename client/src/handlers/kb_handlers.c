#include "kb_handler.h"

gboolean kb_window_handler(GtkWidget *widget, GdkEvent *event, Store *store) {
    if (event->key.state & GDK_MOD1_MASK) { /* ALT + ... */
        switch (event->key.hardware_keycode) {
        case LINUX_HARDWARE_E:
            setting_button_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_F:
            btn_window_fullscreen_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_TILDA:
            btn_window_minimize_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_F4:
            btn_window_close_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_A:
            open_user_settings_tab_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_C:
            open_config_settings_tab_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_T:
            open_themes_settings_tab_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_H:
            open_faq_settings_tab_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_S:
            open_shortcuts_settings_tab_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_X:
            open_storage_settings_tab_handler(widget, store);
            return GDK_EVENT_STOP;
        default:
            printf("%u\n", event->key.hardware_keycode);
            return GDK_EVENT_PROPAGATE;
        }
    }
    if (event->key.state & GDK_CONTROL_MASK) { /* CTRL + ... */
        switch (event->key.hardware_keycode) {
        case LINUX_HARDWARE_Z:
            back_button_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_Q:
            btn_window_close_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_ENTER:
            send_message_handler(widget, store);
            return GDK_EVENT_STOP;
        case LINUX_HARDWARE_0:
            clear_cache_handler(widget, store);
            return GDK_EVENT_STOP;
        default:
            printf("%u\n", event->key.hardware_keycode);
            return GDK_EVENT_PROPAGATE;
        }
    }
    return GDK_EVENT_PROPAGATE;
}

gboolean kb_terminal_window_handler(GtkWidget *widget, GdkEvent *event, message_t *message) {
    (void)message;
    if (event->key.state & GDK_MOD1_MASK) { /* ALT + ... */
        switch (event->key.hardware_keycode) {
        case LINUX_HARDWARE_F4:
            gtk_window_close(GTK_WINDOW(widget));
            return GDK_EVENT_STOP;
        default:
            printf("%u\n", event->key.hardware_keycode);
            return GDK_EVENT_PROPAGATE;
        }
    }
    if (event->key.state & GDK_CONTROL_MASK) { /* CTRL + ... */
        switch (event->key.hardware_keycode) {
        case LINUX_HARDWARE_Q:
            gtk_window_close(GTK_WINDOW(widget));
            return GDK_EVENT_STOP;
        default:
            printf("%u\n", event->key.hardware_keycode);
            return GDK_EVENT_PROPAGATE;
        }
    }
    return GDK_EVENT_PROPAGATE;
}

void apply_kb_handler() {
    Store *store = get_store();

    g_signal_connect(G_OBJECT(store->window), "key_press_event", G_CALLBACK(kb_window_handler), store);
}
