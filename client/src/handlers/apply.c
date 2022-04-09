#include "apply.h"

void apply_handlers() {
    apply_root_handlers();
    apply_auth_handlers();
    apply_messages_handlers();
    apply_teams_handlers();
    apply_settings_handlers();
    apply_settings_user_handlers();
    apply_settings_themes_handlers();
    apply_settings_cache_handlers();
    apply_settings_config_handlers();
    apply_kb_handler();
}
