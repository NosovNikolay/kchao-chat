#pragma once

#include <uchat.h>
#include "../../chat/chats.h"
#include "../../api/api.h"
#include "../../exec/exec.h"
#include "../../conf/conf.h"
#include "../kb_handler.h"
#include "../settings/handlers.h"

void send_message_handler(GtkWidget *widget, Store *store);

void run_message_button_handler(GtkWidget *widget, message_t *message);

void apply_messages_handlers();
