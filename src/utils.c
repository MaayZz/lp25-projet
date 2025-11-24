#include "utils.h"
#include <stdarg.h>

void log_error(const char *fmt, ...)
{
    va_list args;

    fprintf(stderr, "[ERROR] ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void log_info(const char *fmt, ...)
{
    va_list args;

    fprintf(stdout, "[INFO] ");
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}
