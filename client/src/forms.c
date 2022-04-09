#include "handlers/handlers.h"
#include "uchat.h"


bool is_len_password_valid(const char *password) {
    if (!password)
        return false;
    if (mx_strlen(password) < 8)
        return false;
    return true;
}

bool is_password_valid(const char *password) {
    for (int i = 0; i < mx_strlen(password); i++) {
        if (password[i] < 33 || password[i] > 126)
            return false;
    }
    return true;
}

bool is_email_valid(const char *email) {
    int at = 0;
    int email_len = mx_strlen(email);

    if ((at = mx_get_char_index(email, '@')) == -1)
        return false;
    if (mx_get_char_index(email, '.') == -1)
        return false;
    if ((email_len - 1) - at < 2)
        return false;

    return true;
}

bool is_username_valid(const char *username) {
    if (mx_strlen(username) < 3)
        return false;
    for (int i = 0; i < mx_strlen(username); i++) {
        if (username[i] < 33 || username[i] > 126)
            return false;
    }
    return true;
}
