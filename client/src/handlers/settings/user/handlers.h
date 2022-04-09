#pragma once

#include <uchat.h>
#include "../../../api/api.h"

void apply_settings_user_handlers(void);
void settings_user_input_changed(GtkWidget *widget, Store *store);

void logout_button_handler(GtkWidget *widget, Store *store);
