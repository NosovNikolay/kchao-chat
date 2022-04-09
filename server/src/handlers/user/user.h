#pragma once 

#include <uchat_server.h>
#include "../../server/server.h"
#include "../../middlewares/middlewares.h"
#include "../../db/db.h"

#include "avatar/avatar.h"

#define USER_PREFIX "/user"

void attach_handlers_user(void);
