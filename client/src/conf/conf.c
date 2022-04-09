#include "conf.h"

Dict *parse_configs(const char *str) {
    Dict *conf = create_dict();
    if (!str)
        return conf;
    string_t pretty_str = mx_replace_substr(str, "\r\n", "\n");
    string_t *lines = mx_strsplit(pretty_str, '\n');
    if (!lines)
        return conf;
    for (size_t i = 0; lines[i]; i++) {
        string_t *args = mx_strsplit_extended(lines[i], ':', 2, false);
        if (!args[0] || !args[1] || dict_key_index(conf, args[0]) != -1) {
            mx_del_strarr(&args);
            continue;
        }
        RunConfig *rc = (RunConfig *)malloc(sizeof(RunConfig));
        rc->runner = mx_strdup(args[1]);
        rc->flags = mx_strdup(args[2] ? args[2] : "");
        dict_set(conf, args[0], (void *)rc);
        mx_del_strarr(&args);
    }
    mx_del_strarr(&lines);
    free(pretty_str);
    return conf;
}

string_t stringify_config(const char *lang, RunConfig *conf) {
    if (!lang)
        return mx_strdup("");
    char *runner = conf->runner ? conf->runner : "";
    char *flags = conf->flags ? conf->flags : "";
    string_t buff = malloc(strlen(lang) + strlen(runner) + strlen(flags) + 4);
    sprintf(buff, "%s:%s:%s\n", lang, runner, flags);
    return buff;
}

string_t stringify_configs(Dict *confs) {
    string_t buff = mx_strdup("");
    for (ssize_t i = 0; i < confs->len; i++) {
        string_t old_buff = buff;
        string_t line = stringify_config(confs->keys[i], (RunConfig *)confs->values[i]);
        buff = mx_strjoin(old_buff, line);
        free(old_buff);
        free(line);
    }
    return buff;
}

void destroy_run_config(RunConfig *rc) {
    if (rc->flags)
        free(rc->flags);
    if (rc->runner)
        free(rc->runner);
    free(rc);
}

void destroy_run_configs(Dict *rcs) {
    dict_destroy(rcs, (dictc_cleaner)destroy_run_config);
}
