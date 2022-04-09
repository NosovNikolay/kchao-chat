#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <uchat.h>

#if __has_include("vte/vte.h")
#define VTE_HEADER_INCLUDED 1
#include <vte/vte.h>
#else
#define VTE_HEADER_INCLUDED 0
#endif

GtkWidget *show_terminal();

void run_terminal(GtkWidget *term, const char *command);

GtkWidget *default_exec(const char *compiler, const char *flags, const char *code, const char *ext);
GtkWidget *exec_c(const char *compiler, const char *flags, const char *code);
GtkWidget *exec_sh(const char *compiler, const char *flags, const char *code);
GtkWidget *exec_py(const char *compiler, const char *flags, const char *code);

GtkWidget *exec_message(const char *compiler, const char *flags, const char *code, const char *lang);
