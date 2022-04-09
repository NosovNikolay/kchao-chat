#pragma once 

#include <uchat_server.h>
#include "../../../server/server.h"
#include "../../../middlewares/middlewares.h"
#include "../../../db/db.h"
#include "../../../oracle/oracle.h"

#define USER_AVATAR_PREFIX "/user/avatar"

void attach_handlers_user_avatar(void);
