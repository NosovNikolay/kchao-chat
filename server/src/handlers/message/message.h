#pragma once 

#include <uchat_server.h>
#include "../../server/server.h"
#include "../../middlewares/middlewares.h"
#include "../../db/db.h"

#define MESSAGE_PREFIX "/message"

void attach_handlers_message(void);
