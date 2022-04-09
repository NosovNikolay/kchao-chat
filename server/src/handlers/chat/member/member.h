#pragma once 

#include <uchat_server.h>
#include "../../../server/server.h"
#include "../../../middlewares/middlewares.h"
#include "../../../db/db.h"

#define CHAT_MEMBER_PREFIX "/chat/member"

void attach_handlers_chat_member(void);
