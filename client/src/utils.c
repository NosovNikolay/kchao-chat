#include "chat/chats.h"
#include "handlers/kb_handler.h"
#include <uchat.h>

void *file_2_data(const char *fname, size_t *len) {
    void *buffer = NULL;
    size_t length;
    FILE *f = fopen(fname, "rb");
    if (!f)
        return buffer;
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length);
    memset(buffer, 0, length);
    if (buffer)
        fread(buffer, 1, length, f);
    fclose(f);
    *len = length;
    return buffer;
}

string_t file_2_str(const char *fname) {
    string_t buffer = NULL;
    size_t length;
    FILE *f = fopen(fname, "r");
    if (!f)
        return buffer;
    fseek(f, 0, SEEK_END);
    length = ftell(f) + 1;
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length);
    memset(buffer, 0, length);
    if (buffer)
        fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

bool str_2_file(const char *fname, const char *str) {
    FILE *f = fopen(fname, "w");
    if (!f)
        return false;
    bool res = fwrite(str, 1, strlen(str), f) ? true : false;
    fclose(f);
    return res;
}

int rand_int(int min, int max) {
    return rand() % (max + 1 - min) + min;
}

void destroy_user_info(user_info_t *uinfo) {
    if (uinfo->name)
        free(uinfo->name);
    if (uinfo->email)
        free(uinfo->email);
    if (uinfo->nickname)
        free(uinfo->nickname);
    if (uinfo->bio)
        free(uinfo->bio);
    if (uinfo->avatar_id)
        free(uinfo->avatar_id);
    free(uinfo);
}

void load_user_info(Store *store, bson_t *data) {
    store->user_info = malloc(sizeof(user_info_t));
    store->user_info->team_avatar = malloc(sizeof(file_t));
    store->user_info->name = prepare_string(bson_get_str(data, "name"));
    if (!store->user_info->name)
        store->user_info->name = mx_strdup("");
    store->user_info->nickname = prepare_string(bson_get_str(data, "nickname"));
    if (!store->user_info->nickname)
        store->user_info->nickname = mx_strdup("");
    store->user_info->avatar_id = prepare_string(bson_get_str(data, "avatar_id"));
    store->user_info->bio = prepare_string(bson_get_str(data, "bio"));
    if (!store->user_info->bio)
        store->user_info->bio = mx_strdup("");
    store->user_info->email = prepare_string(bson_get_str(data, "email"));
}

void upload_img (GtkWidget *wiget, gulong avatar_id) {
    Store *store = get_store();
    widget_id_t *widget_id = malloc(sizeof(widget_id_t));
    widget_id->widget = wiget;
    widget_id->id = avatar_id;
    GThread *thread = api_get_file_async(load_chat_images_cb, widget_id, store->user_info->avatar_id);
    g_thread_unref(thread);
}

void set_settings_user_info(Store *store) {
    GtkLabel *name_label = GTK_LABEL(gtk_builder_get_object(store->builder, "SettingsUserName"));
    GtkLabel *tag_label = GTK_LABEL(gtk_builder_get_object(store->builder, "SettingsUserTag"));
    gtk_label_set_text(name_label, store->user_info->name);
    gtk_label_set_text(tag_label, store->user_info->nickname);

    gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserNameEntry")), store->user_info->name);
    if (store->user_info->bio)
        gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserBioEntry")), store->user_info->bio);
    if (store->user_info->nickname)
        gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserNicknameEntry")), store->user_info->nickname);

    GtkWidget *user_avatar = GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsUserAvatar"));
    gtk_widget_set_size_request(GTK_WIDGET(user_avatar), 70, 70);
    gulong handler_id = g_signal_connect(G_OBJECT(user_avatar), "draw", G_CALLBACK(draw_default_avatar), (int *)70);
    store->user_info->avatar_signal_id = handler_id;

    GtkWidget *user_set_avatar = GTK_WIDGET(gtk_builder_get_object(store->builder, "UserAvatar"));
    gtk_widget_set_size_request(GTK_WIDGET(user_set_avatar), 70, 70);
    handler_id = g_signal_connect(G_OBJECT(user_set_avatar), "draw", G_CALLBACK(draw_default_avatar), (int *)70);
    store->user_info->avatar_set_signal_id = handler_id;
}

void update_user_settings_info(Store *store) {
    GtkLabel *name_label = GTK_LABEL(gtk_builder_get_object(store->builder, "SettingsUserName"));
    GtkLabel *tag_label = GTK_LABEL(gtk_builder_get_object(store->builder, "SettingsUserTag"));
    gtk_label_set_text(name_label, store->user_info->name);
    gtk_label_set_text(tag_label, store->user_info->nickname);

    gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserNameEntry")), store->user_info->name);
    if (store->user_info->bio)
        gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserBioEntry")), store->user_info->bio);
    if (store->user_info->nickname)
        gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(store->builder, "UserNicknameEntry")), store->user_info->nickname);
}

void gtk_widget_set_classname(GtkWidget *wid, const char *class) {
    GtkStyleContext *context = gtk_widget_get_style_context(wid);
    gtk_style_context_add_class(context, class);
}

void gtk_widget_remove_classname(GtkWidget *wid, const char *class) {
    GtkStyleContext *context = gtk_widget_get_style_context(wid);
    gtk_style_context_remove_class(context, class);
}

bool is_user_in_chat(chat_entry *chat, const char *id) {
    for (int i = 0; i < chat->count_participants; i++) {
        if (mx_streq(chat->participants_id[i], id))
            return true;
    }
    return false;
}

int64_t get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
}

long get_age(long milisec) {
    long seconds = time(NULL);
    return seconds - milisec;
}

const bson_value_t *bson_get(bson_t *obj, const char *key) {
    bson_iter_t iter, res;
    if (bson_iter_init(&iter, obj) && bson_iter_find_descendant(&iter, key, &res))
        return bson_iter_value(&res);
    return NULL;
}

bson_t *get_participants(bson_t *chat) {
    if (!chat)
        return NULL;
    const bson_value_t *participants_v = bson_get(chat, "participants");
    return bson_new_from_data(participants_v->value.v_doc.data, participants_v->value.v_doc.data_len);
}

string_t create_success_message(const char *email) {
    string_t email_template = "Email confirmation has been send to {email}";
    return mx_replace_substr(email_template, "{email}", email);
}

string_t create_reset_message(const char *email) {
    string_t email_template = "Password reset email has been send to {email}";
    return mx_replace_substr(email_template, "{email}", email);
}

int bson_get_int(bson_t *bson, const char *key) {
    bson_iter_t bson_iter, res_iter;
    bson_iter_init(&bson_iter, bson);
    if (bson_iter_find_descendant(&bson_iter, key, &res_iter) &&
        BSON_ITER_HOLDS_INT32(&res_iter)) {
        return (int)bson_iter_int32(&res_iter);
    }
    return BSON_NOT_FOUND;
}

bson_t *bson_get_document(bson_t *bson, const char *key) {
    bson_iter_t bson_iter, res_iter;
    bson_iter_init(&bson_iter, bson);
    if (bson_iter_find_descendant(&bson_iter, key, &res_iter) && BSON_ITER_HOLDS_DOCUMENT(&res_iter)) {
        const bson_value_t *doc = bson_iter_value(&res_iter);
        return bson_new_from_data(doc->value.v_doc.data, doc->value.v_doc.data_len);
    }
    return NULL;
}

int64_t bson_get_int64_t(bson_t *bson, const char *key) {
    bson_iter_t bson_iter, res_iter;
    bson_iter_init(&bson_iter, bson);
    if (bson_iter_find_descendant(&bson_iter, key, &res_iter) &&
        BSON_ITER_HOLDS_INT64(&res_iter)) {
        return (int64_t)bson_iter_int64(&res_iter);
    }
    return BSON_NOT_FOUND;
}

string_t get_message_type(int cur) {
    if (cur == 0)
        return mx_strdup("text");
    if (cur == 1)
        return mx_strdup("c");
    if (cur == 2)
        return mx_strdup("c++");
    if (cur == 3)
        return mx_strdup("python");
    if (cur == 4)
        return mx_strdup("node");
    if (cur == 5)
        return mx_strdup("shell");
    return mx_strdup("error type");
}

string_t prepare_string(const char *str) {
    if (!str)
        return NULL;
    string_t result = mx_strtrim(str);
    if (!result || !strlen(result))
        mx_strdel(&result);
    return result;
}

char *bson_get_str(bson_t *bson, const char *key) {
    bson_iter_t bson_iter, res_iter;
    bson_iter_init(&bson_iter, bson);
    if (bson_iter_find_descendant(&bson_iter, key, &res_iter) &&
        BSON_ITER_HOLDS_UTF8(&res_iter)) {
        uint32_t name_size;
        return (char *)bson_iter_utf8(&res_iter, &name_size);
    }
    return NULL;
}

bool bson_get_bool(bson_t *bson, const char *key) {
    bson_iter_t bson_iter, res_iter;
    bson_iter_init(&bson_iter, bson);
    if (bson_iter_find_descendant(&bson_iter, key, &res_iter) &&
        BSON_ITER_HOLDS_BOOL(&res_iter)) {
        return (bool)bson_iter_as_bool(&res_iter);
    }
    mx_printstr("bson not found\n");
    return false;
}

char *bson_get_inside_str(bson_t *bson, const char *fkey, const char *key) {
    bson_iter_t iter;
    bson_iter_t child;
    bson_iter_t res_iter;

    if (bson_iter_init_find(&iter, bson, fkey) &&
        BSON_ITER_HOLDS_DOCUMENT(&iter) && bson_iter_recurse(&iter, &child)) {

        while (1) {
            if (bson_iter_find_descendant(&child, key, &res_iter) &&
                BSON_ITER_HOLDS_UTF8(&res_iter)) {
                uint32_t name_size;
                return (char *)bson_iter_utf8(&res_iter, &name_size);
            }
            if (!bson_iter_next(&child))
                return NULL;
        }
    }
    return NULL;
}
struct kb_enter_ctx {
    gboolean (*cb)(GtkWidget *widget, gpointer data);
    gpointer data;
};

gboolean kb_enter(GtkWidget *widget, GdkEvent *event, struct kb_enter_ctx *ctx) {
    if (event->key.hardware_keycode != LINUX_HARDWARE_ENTER || !ctx->cb)
        return GDK_EVENT_PROPAGATE;
    return ctx->cb(widget, ctx->data);
}

guint widget_on_key_enter(GObject *widget, gboolean (*cb)(GtkWidget *widget, gpointer data), gpointer data) {
    struct kb_enter_ctx *ctx = malloc(sizeof(struct kb_enter_ctx));
    ctx->cb = cb;
    ctx->data = data;
    return g_signal_connect(widget, "key_press_event", G_CALLBACK(kb_enter), (gpointer)ctx);
}

void gtk_remove_text_login(Store *store) {
    GtkEntry *entry = NULL;
    entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryEmail");
    gtk_entry_set_text(entry, "");
    entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryPassword");
    gtk_entry_set_text(entry, "");

    gtk_remove_labels_text_login(store);
}

void gtk_remove_labels_text_login(Store *store) {
    GtkLabel *label = NULL;
    label = (GtkLabel *)gtk_builder_get_object(store->builder, "LoginEmailLabel");
    gtk_label_set_text(label, "");
    label = (GtkLabel *)gtk_builder_get_object(store->builder, "UserMessageLabel");
    gtk_label_set_text(label, "");
}

void gtk_remove_styles_login(Store *store) {
    GtkStyleContext *context = NULL;

    GtkEntry *email_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryEmail");
    context = gtk_widget_get_style_context(GTK_WIDGET(email_entry));
    gtk_style_context_remove_class(context, "InputFailed");

    GtkEntry *pass_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "EntryPassword");
    context = gtk_widget_get_style_context(GTK_WIDGET(pass_entry));
    gtk_style_context_remove_class(context, "InputFailed");

    GtkEntry *message_label = (GtkEntry *)gtk_builder_get_object(store->builder, "UserMessageLabel");
    context = gtk_widget_get_style_context(GTK_WIDGET(message_label));
    gtk_style_context_remove_class(context, "UserErrorLabel");
    gtk_style_context_remove_class(context, "UserSuccessLabel");
}

void gtk_remove_text_register(Store *store) {
    GtkEntry *entry = NULL;
    entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterUsername");
    gtk_entry_set_text(entry, "");
    entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterEmail");
    gtk_entry_set_text(entry, "");
    entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterPassword");
    gtk_entry_set_text(entry, "");
    entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterRepeatPassword");
    gtk_entry_set_text(entry, "");

    gtk_remove_labels_text_register(store);
}

void gtk_remove_labels_text_register(Store *store) {
    GtkLabel *label = NULL;

    label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterEmailLabel");
    gtk_label_set_text(GTK_LABEL(label), "");
    label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterPasswordLabel");
    gtk_label_set_text(GTK_LABEL(label), "");
    label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterRepeatPasswordLabel");
    gtk_label_set_text(GTK_LABEL(label), "");
    label = (GtkLabel *)gtk_builder_get_object(store->builder, "RegisterUsernameLabel");
    gtk_label_set_text(GTK_LABEL(label), "");
}

void gtk_remove_styles_register(Store *store) {
    GtkEntry *pass_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterPassword");
    GtkEntry *pass_repeat_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterRepeatPassword");
    GtkEntry *email_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterEmail");
    GtkEntry *usermane_entry = (GtkEntry *)gtk_builder_get_object(store->builder, "RegisterUsername");

    GtkStyleContext *context = NULL;
    context = gtk_widget_get_style_context(GTK_WIDGET(usermane_entry));
    gtk_style_context_remove_class(context, "InputFailed");
    context = gtk_widget_get_style_context(GTK_WIDGET(pass_entry));
    gtk_style_context_remove_class(context, "InputFailed");
    context = gtk_widget_get_style_context(GTK_WIDGET(pass_repeat_entry));
    gtk_style_context_remove_class(context, "InputFailed");
    context = gtk_widget_get_style_context(GTK_WIDGET(email_entry));
    gtk_style_context_remove_class(context, "InputFailed");
}

string_t open_dialog(GtkWidget *widget) {
    GtkWidget *dialog;
    GtkFileFilter *filter;
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    dialog = gtk_file_chooser_dialog_new("Select image", parent_window, action,
                                         "_Load", GTK_RESPONSE_ACCEPT,
                                         "_Cancel", GTK_RESPONSE_CANCEL, NULL);
    GtkStyleContext *ctx = gtk_widget_get_style_context(dialog);
    gtk_style_context_add_class(ctx, "FileChoser");
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    if (resp == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_widget_destroy(dialog);
        return filename;
    } else
        g_print("You pressed the cancel\n");
    gtk_widget_destroy(dialog);
    return NULL;
}

void go_2_auth(Store *store) {
    gtk_window_set_default_size(GTK_WINDOW(store->window), 400, 600);
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(store->builder, "ChatsTab")));
    gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsTab")));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(store->builder, "MainNotebook")), 0);
    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsButton")), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(store->builder, "ChatsButton")), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(store->window), FALSE);
}

void go_2_chat(Store *store) {
    gtk_widget_show_all(GTK_WIDGET(gtk_builder_get_object(store->builder, "ChatsTab")));
    gtk_widget_show_all(GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsTab")));
    gtk_widget_set_visible(GTK_WIDGET(gtk_builder_get_object(store->builder, "SettingsButton")), TRUE);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(gtk_builder_get_object(store->builder, "MainNotebook")), 1);
    gtk_window_resize(GTK_WINDOW(store->window), 1200, 700);
    gtk_window_set_resizable(GTK_WINDOW(store->window), TRUE);
}

size_t count_dir_size(const char *dir_name) {
    struct stat buf;
    struct dirent *sdir;
    size_t total_size = 0;
    char str[1024];

    DIR *dptr = opendir(dir_name);
    if (!dptr)
        return 0;
    while ((sdir = readdir(dptr))) {
        if (sdir->d_type == 4) {
            if (sdir->d_name[0] != '.') {
                sprintf(str, "%s/%s", dir_name, sdir->d_name);
                total_size += count_dir_size(str);
            }
        } else {
            sprintf(str, "%s/%s", dir_name, sdir->d_name);
            stat(str, &buf);
            total_size += buf.st_size;
        }
    }
    closedir(dptr);
    return total_size;
}

static int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    (void)sb;
    (void)typeflag;
    (void)ftwbuf;
    int rv = remove(fpath);
    if (rv)
        perror(fpath);
    return rv;
}

int rm_dir(const char *dir_name) {
    return nftw(dir_name, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

string_t human_size(size_t bytes) {
    char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;
    double dblBytes = bytes;

    if (bytes > 1024)
        for (i = 0; (bytes / 1024) > 0 && i < length - 1; i++, bytes /= 1024)
            dblBytes = bytes / 1024.0;

    string_t output = mx_strnew(200);
    sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
    return output;
}

char *get_lang_prefix(const char *type) {
    if (!type) return NULL;
    if (mx_streqi(type, "Cpp") || mx_streqi(type, "c++")) {
        return "c++";
    } else if (mx_streqi(type, "Clang") || mx_streqi(type, "c")) {
        return "c";
    } else if (mx_streqi(type, "Node") || mx_streqi(type, "js") || mx_streqi(type, "nodejs")) {
        return "js";
    } else if (mx_streqi(type, "Python") || mx_streqi(type, "py")) {
        return "py";
    } else if (mx_streqi(type, "Shell") || mx_streqi(type, "sh")) {
        return "sh";
    } else
        return NULL;
}
