#pragma once

#define _XOPEN_SOURCE 600

#include <bson/bson.h>
#include <dict.h>
#include <gtk/gtk.h>
#include <libmx.h>
#include <requests-c.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ftw.h>

#define CLIENT_PATH "client/"
#define CACHE_DIR "ckchao_cache"

#define DATA_PATH CLIENT_PATH "data/"
#define THEMES_PATH DATA_PATH "styles/themes/"
#define PATH_TO_BINARY CLIENT_PATH "save.bin"
#define PATH_TO_CONFIGURATION CLIENT_PATH "configuration.txt"
#define CACHE_PATH CLIENT_PATH CACHE_DIR


#define USER_NOT_FOUND 10
#define SUCCESS 200
#define EMAIL_NOT_CONFIRMED -10
#define UNAME_OR_PSW_WRONG -1
#define USER_EXIST -2

#define NO_CONNECTION 0

#define BSON_NOT_FOUND -30

#define UNSER_SENDED "13454324"

typedef struct file_s {
    size_t len;
    void *data;
} file_t;

typedef struct widget_redrow_s {
    int w;
    int h;
    GdkPixbufLoader *loader;
    GdkPixbuf *pixbuf;

} widget_redrow_t;

typedef struct message_ui_s {
    GtkWidget *main_box;
    GtkWidget *name_label;
    GtkWidget *body_box;
    GtkWidget *run_button;
    GtkWidget *run_button_img;
    GtkWidget *text_label;
    GtkWidget *info_box;
    GtkWidget *sent_status_img;
    GtkWidget *time_label;
} message_ui_t;

typedef struct message_s {
    const char *chat_id;
    const char *type;
    const char *text;
    const char *message_id;
    const char *from_user;
    message_ui_t *message_ui;
    int64_t updated_at;
    bool is_edited;
} message_t;

extern char **environ;

typedef struct user_s {
    const char **user_id;
    const char **token;
} User;

typedef struct users_data_s {
    int current_user;
    int count_users;
    User *users;
} users_data;

typedef struct members_ui_s {
    const char *member_id;
    gulong avatar_handler;
    GtkWidget *member_avatar;
    GtkWidget *name_label;
    GtkWidget *member_box;
} members_ui_t;

typedef struct gtk_actve_chat_entry_s {
    int count_messages;
    double scroll_upper;
    message_ui_t **messages;
    GtkWidget *chat_scrolled;
    GtkWidget *chat_viewport;
    GtkWidget *chat_active_box;
    GtkWidget *message_box;
    GtkWidget *members_box;
    GtkWidget *members_label;
    GtkWidget *chat_info_box;
    GtkWidget *chat_image;
    GtkWidget *change_image;
    bool changing_image;
    file_t *img;
    GtkWidget *chat_users;
    const char *chat_info_sec_label_text;
    GtkWidget *chat_info_sec_label;
    GtkWidget *chat_info_chat_name;
    GtkWidget *chat_info_chat_bio;
    GtkWidget *save_button;
    gulong save_button_signal;
    gulong handler_id;
    GtkAdjustment *adjustment;
    gulong adjustment_signal;
    
    members_ui_t **members_ui;

} gtk_actve_chat_entry_t;

typedef struct user_entry_s {
    const char *id;
    const char *name;
    const char *email;
    const char *nickname;
    const char *bio;
    const char *avatar_id;
    GdkPixbuf *avabuf;
    GdkPixbufLoader *pixbufloader;
    gulong ava_handler_id;
} user_entry_t;

typedef struct search_partipiants_ui_s {
    GtkWidget *box;
    GtkWidget *event_box;
    gulong event_box_handler;
    GtkWidget *avatar_drawing;
    gulong avatar_handler;
    GtkWidget *name_label;
} search_partipiants_ui_t;


typedef struct search_partipiants_s {
    search_partipiants_ui_t *ui;
    user_entry_t *user;
} search_partipiants_t;

typedef struct chat_entry_s {
    bool is_loaded;
    int64_t updated_at;

    message_t **not_confirmed_messages;
    int count_not_confirmed;

    gtk_actve_chat_entry_t *active_entry;
    int count_messages;
    message_t **messages_history;
    const char *_id;
    const char *type;
    const char *name;
    const char *description;
    const char *avatar_id;
    const char *admin;
    int *page;
    gulong avatar_handler_id;
    gulong chat_window_signal;

    GdkPixbufLoader *pixbufloader;
    GdkPixbuf *gtk_chat_avatar_buf;
    GtkWidget *gtk_main_chat_box;
    GtkWidget *gtk_event_box;
    GtkWidget *gtk_chat_box;
    GtkWidget *gtk_chat_avatar;
    GtkWidget *gtk_chat_text_box;
    GtkWidget *gtk_chat_name;
    
    int pack_position;
    message_t *last_message;
    const char *last_message_str;
    GtkWidget *gtk_last_message;

    int count_participants;
    const char **participants_id;
} chat_entry;

typedef struct all_chats_s {
    int chats_count;
    chat_entry **chats_arr;
} all_chats_t;

typedef struct user_info {
    char *name;
    char *email;
    char *nickname;
    char *bio;
    char *avatar_id;
    gulong avatar_signal_id;
    gulong avatar_set_signal_id;
    bool setting_new_avatar;
    file_t *avatar_img;

    gulong avatar_team_id;
    file_t *team_avatar;
} user_info_t;

typedef struct store_s {
    bool is_updatable;
    GtkApplication *app;
    GtkWidget *window;
    GtkBuilder *builder;
    GtkCssProvider *css_provider;
    gboolean is_fullscreen;
    gboolean is_maximized;
    all_chats_t *chats;
    const char *whoami;
    Dict *run_configuration;

    int all_users_count;
    user_entry_t **all_users;
    user_info_t *user_info;
} Store;

typedef struct load_users_s {
    int all_users_count;
    user_entry_t **all_users;
} load_users_t;

#define EVENT_TYPE_NEW_MESSAGE "new_message"
#define EVENT_TYPE_MESSAGE_DELETED "message_deleted"
#define EVENT_TYPE_MESSAGE_UPDATED "message_updated"
#define EVENT_TYPE_GROUP_ADDED "group_added"
#define EVENT_TYPE_GROUP_REMOVED "group_removed"
#define EVENT_TYPE_GROUP_CHANGED "chat_changed"
#define EVENT_TYPE_CHAT_DELETED "chat_deleted"
#define EVENT_TYPE_USER_CHANGED "user_changed"

GtkCssProvider *init_css_provider(string_t css_path);

bool is_user_in_data(users_data *data, const char *user_id);

users_data *add_user_to_data(users_data *data, const char *user_id, const char *token);

users_data *init_data(int count_users);

int save_users(users_data *data, const char *filepath);

users_data *load_users(const char *filepath);

void free_user_data(users_data *data);

void user_autologin(Store *store, users_data *data);

void autologin_callback(RCResponse *response, void *data);

void login_init_chats(Store *store, bson_t *bson_user);

void set_user_info(Store *store, bson_t *user_responce);

void load_all_active_users(Store *store, users_data *data);

void api_is_user_active(Store *store, users_data *data);

void create_active_user(RCResponse *response, void *data);

Store *init_store(GtkApplication *app);

Store *get_store();

void destroy_store();

void gtk_remove_text_login(Store *store);
void gtk_remove_labels_text_login(Store *store);
void gtk_remove_styles_login(Store *store);

void gtk_remove_text_register(Store *store);
void gtk_remove_labels_text_register(Store *store);
void gtk_remove_styles_register(Store *store);

bool is_email_valid(const char *email);
bool is_len_password_valid(const char *password);
bool is_password_valid(const char *password);
bool is_username_valid(const char *username);

// utils

int rand_int(int min, int max);

guint widget_on_key_enter(GObject *widget, gboolean (*cb)(GtkWidget *widget, gpointer data), gpointer data);

int64_t get_current_time();

string_t create_success_message(const char *email);
string_t create_reset_message(const char *email);

void *file_2_data(const char *fname, size_t *len);
string_t file_2_str(const char *fname);
bool str_2_file(const char *fname, const char *str);

string_t get_message_type(int cur);
int bson_get_int(bson_t *bson, const char *key);
bson_t *bson_get_document(bson_t *bson, const char *key);
int64_t bson_get_int64_t(bson_t *bson, const char *key);
char *bson_get_str(bson_t *bson, const char *key);
bool bson_get_bool(bson_t *bson, const char *key);
char *bson_get_inside_str(bson_t *bson, const char *fkey, const char *key);
string_t prepare_string(const char *str);

string_t open_dialog(GtkWidget *widget);

// Settings
void load_user_info(Store *store, bson_t *data);
void set_settings_user_info(Store *store);
void destroy_user_info(user_info_t *uinfo);
void update_user_settings_info(Store *store);
// load chats
void load_chats(Store *store);
void *load_participants(Store *store);
void add_user_to_all_users(load_users_t *users, RCResponse *response);
user_entry_t *fill_participant(bson_t *bson_user_info);
void load_participants_cb(load_users_t *users, void *data);
bool is_user_loaded(load_users_t *users, const char *user_id);
int get_user_index(Store *store, const char *id);
void load_user_image_cb(RCResponse *response, void *data);

// refresh
void refresh_chat_data(int chat_id, Store *store);
void refresh_chat_ui(int chat_id, Store *store);
void refresh_group_member_image(Store *store, user_entry_t *user);
void refresh_setting_image(Store *store, user_entry_t *user);
void load_chat_name(chat_entry *chat);
void load_chat_info_name(int chat_id, Store *store);
void load_chat_info_bio(chat_entry *chat);
void load_chat_sec_label(int chat_id, Store *store);

// css
void gtk_widget_set_classname(GtkWidget *wid, const char *class);
void gtk_widget_remove_classname(GtkWidget *wid, const char *class);

// images
gulong widget_redrow(int w, int h, GdkPixbuf *pixbuf, GtkWidget *widget, gulong handler_id);
void drower(GtkWidget *widget, cairo_t *cr, void *data);
void refresh_private_chats_image(Store *store, user_entry_t *user);
void set_settings_user_avatar(GtkWidget *user_avatar, GtkWidget *user_set_avatar);
void upload_img (GtkWidget *wiget, gulong avatar_id);



bson_t *get_participants(bson_t *chat);
const bson_value_t *bson_get(bson_t *obj, const char *key);

int add_message_to_store_last_message(message_t *message, Store *store);

void update_last_message(Store *store, message_t *message);

void load_last_messages(Store *store);

void load_last_messages_cb(RCResponse *response, void *data);

long get_age(long milisec);

void reorder_all_loaded_chats(Store *store);

void go_2_auth(Store *store);
void go_2_chat(Store *store);

string_t human_size(size_t bytes);

size_t count_dir_size(const char *dir_name);
int rm_dir(const char *dir_name);

char *get_lang_prefix(const char *type);

// Cache
void start_cache_watcher();
size_t count_cache_size();
void clear_cache();

//free memory
void free_chat_inst(chat_entry *chat);
void free_chat(chat_entry *chat);
void free_chat_messages(chat_entry *chat);
void free_chat_message(message_t *message);
void free_chat_ui_message(gtk_actve_chat_entry_t *active);
void free_user(user_entry_t *user);
void free_save_file(const char *filepath);
