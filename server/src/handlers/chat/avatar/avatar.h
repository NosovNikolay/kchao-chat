#pragma once 

#include <uchat_server.h>
#include "../../../server/server.h"
#include "../../../middlewares/middlewares.h"
#include "../../../db/db.h"
#include "../../../oracle/oracle.h"

#define CHAT_AVATAR_PREFIX "/chat/avatar"

void attach_handlers_chat_avatar(void);
