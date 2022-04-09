#pragma once 

#include <uchat_server.h>
#include "../../server/server.h"
#include "../../middlewares/middlewares.h"
#include "../../db/db.h"

#include "member/member.h"
#include "avatar/avatar.h"

#define CHAT_PREFIX "/chat"

void attach_handlers_chat(void);
