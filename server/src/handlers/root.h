#pragma once

#include <uchat_server.h>
#include "../server/server.h"
#include "../middlewares/middlewares.h"
#include "../poll/poll.h"
#include "../oracle/oracle.h"


#include "auth/auth.h"
#include "chat/chat.h"
#include "user/user.h"
#include "message/message.h"

void attach_handlers(void);
