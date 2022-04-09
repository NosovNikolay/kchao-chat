#include "email.h"

FILE *str_to_file(const char *str) {
    return fmemopen((void *)str, strlen(str), "r");
}
