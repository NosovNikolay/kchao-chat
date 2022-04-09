#include "exec.h"

void init_cache() {
    struct stat st = {0};
    if (stat(CACHE_PATH, &st) == -1)
        mkdir(CACHE_PATH, 0777);
}

GtkWidget *show_terminal() {
#if VTE_HEADER_INCLUDED
    GtkWidget *term = vte_terminal_new();

    GtkWidget *t_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkStyleContext *context = gtk_widget_get_style_context(t_win);
    gtk_widget_set_name(t_win, "TerminalWindow");
    gtk_style_context_add_class(context, "terminal_win");

    gtk_window_set_title(GTK_WINDOW(t_win), "Runner");

    gtk_window_set_default_size(GTK_WINDOW(t_win), 900, 500);
    gtk_container_add(GTK_CONTAINER(t_win), term);
    gtk_widget_show_all(t_win);
    return term;
#else
    mx_printstr("\n\033[34mVTE\033[33m is unavailable, install it and rebuild the app to run the code.\033[0m\n");
    return NULL;
#endif
}

void run_terminal(GtkWidget *term, const char *command) {
#if VTE_HEADER_INCLUDED
    if (!term)
        return;
    char *args[] = {
        getenv("SHELL"),
        "-c",
        (char *)command,
        NULL};
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    vte_terminal_spawn_async(VTE_TERMINAL(term), VTE_PTY_DEFAULT, cwd, args, environ, G_SPAWN_DEFAULT, NULL, NULL, NULL, -1, NULL, NULL, NULL);
#else
    (void)term;
    (void)command;
#endif
}

static string_t create_code_file(const char *ext, const char *code) {
    init_cache();
    char name[30];
    memset(name, 0, 30);
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    sprintf(name, "%lld", (long long)(cur_time.tv_sec * 1000000 + cur_time.tv_usec));

    string_t path = mx_strnew(strlen(CACHE_PATH) + 1 + strlen(name) + 1 + strlen(ext));
    strcat(path, CACHE_PATH);
    strcat(path, "/");
    strcat(path, name);
    if (ext[0] != '.')
        strcat(path, ".");
    strcat(path, ext);
    FILE *fd = fopen(path, "w");
    if (!fd)
        return NULL;
    fwrite(code, 1, strlen(code), fd);
    fclose(fd);
    return path;
}

static string_t create_command(const char *template, const char *compiler, const char *flags, const char *path, const char *executable) {
    string_t path_stage = mx_replace_substr(template, "{path}", path);
    string_t compiler_stage = mx_replace_substr(path_stage, "{compiler}", compiler);
    free(path_stage);
    string_t exec_stage = mx_replace_substr(compiler_stage, "{executable}", executable);
    free(compiler_stage);
    string_t command = mx_replace_substr(exec_stage, "{flags}", flags);
    free(exec_stage);
    return command;
}

GtkWidget *default_exec(const char *compiler, const char *flags, const char *code, const char *ext) {
    static const char *template = "{compiler} {flags} {executable}";

    string_t exec = create_code_file(ext, code);
    if (!exec)
        return NULL;

    GtkWidget *term = show_terminal();

    string_t command = create_command(template, compiler, flags, "", exec);
    run_terminal(term, command);

    free(command);
    return term;
}

GtkWidget *exec_c(const char *compiler, const char *flags, const char *code) {
    static const char *template = "{compiler} {flags} -o {executable}.out {executable};./{executable}.out";
    string_t exec = create_code_file("c", code);
    if (!exec)
        return NULL;

    GtkWidget *term = show_terminal();

    string_t command = create_command(template, compiler, flags, CACHE_PATH, exec);
    run_terminal(term, command);
    free(command);
    return term;
}

GtkWidget *exec_sh(const char *compiler, const char *flags, const char *code) {
    return default_exec(compiler, flags, code, "sh");
}

GtkWidget *exec_py(const char *compiler, const char *flags, const char *code) {
    return default_exec(compiler, flags, code, "py");
}

GtkWidget *exec_js(const char *compiler, const char *flags, const char *code) {
    return default_exec(compiler, flags, code, "js");
}

GtkWidget *exec_message(const char *compiler, const char *flags, const char *code, const char *lang) {
    if (mx_streqi(lang, "C") || mx_streqi(lang, "C++") || mx_streqi(lang, "CPP")) {
        return exec_c(compiler, flags, code);
    } else if (mx_streqi(lang, "sh")) {
        return exec_sh(compiler, flags, code);
    } else if (mx_streqi(lang, "py")) {
        return exec_py(compiler, flags, code);
    } else if (mx_streqi(lang, "js")) {
        return exec_js(compiler, flags, code);
    } else {
        return default_exec(compiler, flags, code, lang);
    }
}
