#pragma once
#ifdef __APPLE__
#include <stdint.h>
typedef uint16_t char16_t;
typedef uint32_t char32_t;
#else
#include <uchar.h>
#endif

#define LINUX_HARDWARE_A 38
#define LINUX_HARDWARE_S 39
#define LINUX_HARDWARE_X 53
#define LINUX_HARDWARE_C 54
#define LINUX_HARDWARE_T 28
#define LINUX_HARDWARE_H 43
#define LINUX_HARDWARE_Z 52
#define LINUX_HARDWARE_Q 24
#define LINUX_HARDWARE_F 41
#define LINUX_HARDWARE_E 26
#define LINUX_HARDWARE_0 19
#define LINUX_HARDWARE_F4 70
#define LINUX_HARDWARE_ENTER 36
#define LINUX_HARDWARE_TILDA 49

#include "handlers.h"
#include "settings/handlers.h"
#include "settings/cache/handlers.h"
#include "messages/handlers.h"
#include "auth/handlers.h"

gboolean kb_window_handler(GtkWidget *widget, GdkEvent *event, Store* store);
gboolean kb_terminal_window_handler(GtkWidget *widget, GdkEvent *event, message_t *message);

void apply_kb_handler();
