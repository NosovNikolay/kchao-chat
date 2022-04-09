#pragma once 

#include <uchat_server.h>
#include "../../server/server.h"
#include "../../email/email.h"
#include "../../middlewares/middlewares.h"
#include "../../db/db.h"
#include "../../crypto/crypto.h"
#include "../../utils/utils.h"

#define CONFIRM_LINK "/auth/confirmEmail?confirmToken="
#define CONFIRM_LINK_RESET "/auth/resetPassword?resetPasswordToken=<token>&email="
#define AUTH_PREFIX "/auth"
#define HOUR_IN_MS 36000000
void attach_handlers_auth(void);
