#pragma once

#include <dict.h>
#include <libmx.h>

typedef struct run_conf_s {
    string_t runner;
    string_t flags;
} RunConfig;

Dict *parse_configs(const char *str);

string_t stringify_config(const char *lang, RunConfig *conf);

string_t stringify_configs(Dict *confs);

void destroy_run_config(RunConfig *rc);

void destroy_run_configs(Dict *rcs);
