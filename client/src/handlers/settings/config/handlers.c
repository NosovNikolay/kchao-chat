#include "handlers.h"

static char *langs[] = {"Clang", "Cpp", "Node", "Python", "Shell", NULL};

static char *get_lang_prefix_from_name(const char *label) {
    if (strstr(label, "Cpp")) {
        return "c++";
    } else if (strstr(label, "Clang")) {
        return "c";
    } else if (strstr(label, "Node")) {
        return "js";
    } else if (strstr(label, "Python")) {
        return "py";
    } else if (strstr(label, "Shell")) {
        return "sh";
    } else
        return NULL;
}

static gboolean settings_config_input_changed(GtkWidget *widget, void *_) {
    (void)_;
    Store *store = get_store();
    const char *value = gtk_entry_get_text(GTK_ENTRY(widget));
    const char *w_name = gtk_widget_get_name(widget);

    char *lang = get_lang_prefix_from_name(w_name);
    if (!lang)
        return GDK_EVENT_PROPAGATE;

    int t;
    if (strstr(w_name, "Runner")) {
        t = 1;
    } else if (strstr(w_name, "Flags")) {
        t = 0;
    } else
        return GDK_EVENT_PROPAGATE;

    RunConfig *run_conf = dict_get(store->run_configuration, lang);
    if (!run_conf) {
        run_conf = (RunConfig *)malloc(sizeof(RunConfig));
        if (t) {
            run_conf->runner = mx_strdup(value);
            run_conf->flags = mx_strdup("");
        } else {
            run_conf->runner = mx_strdup("");
            run_conf->flags = mx_strdup(value);
        }
        dict_set(store->run_configuration, lang, run_conf);
    } else {
        if (t) {
            if (run_conf->runner)
                free(run_conf->runner);
            run_conf->runner = mx_strdup(value);
        } else {
            if (run_conf->flags)
                free(run_conf->flags);
            run_conf->flags = mx_strdup(value);
        }
    }

    string_t conf_str = stringify_configs(store->run_configuration);
    str_2_file(PATH_TO_CONFIGURATION, conf_str);
    free(conf_str);

    return GDK_EVENT_PROPAGATE;
}

void apply_settings_config_handlers() {
    Store *store = get_store();
    string_t configuration_str = file_2_str(PATH_TO_CONFIGURATION);
    store->run_configuration = parse_configs(configuration_str);
    free(configuration_str);
    char buff[100];

    for (size_t i = 0; langs[i]; i++) {
        char *lang = langs[i];
        sprintf(buff, "Input%sRunner", lang);
        GObject *runner = gtk_builder_get_object(store->builder, buff);
        sprintf(buff, "Input%sFlags", lang);
        GObject *flags = gtk_builder_get_object(store->builder, buff);
        char *prefix = get_lang_prefix_from_name(lang);

        RunConfig *run_conf = dict_get(store->run_configuration, prefix);
        if (run_conf) {
            gtk_entry_set_text(GTK_ENTRY(runner), run_conf->runner);
            gtk_entry_set_text(GTK_ENTRY(flags), run_conf->flags);
        }

        g_signal_connect(runner, "changed", G_CALLBACK(settings_config_input_changed), NULL);
        widget_on_key_enter(runner, settings_config_input_changed, NULL);
        g_signal_connect(flags, "changed", G_CALLBACK(settings_config_input_changed), NULL);
        widget_on_key_enter(flags, settings_config_input_changed, NULL);
    }
}
