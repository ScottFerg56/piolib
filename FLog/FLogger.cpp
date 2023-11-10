#include "FLogger.h"

FLogger FLogger::flog;
flog_print_t FLogger::printer;

int FLogger::printfv(const char *format, va_list arg)
{
    static char loc_buf[64];
    char * temp = loc_buf;
    uint32_t len;
    va_list copy;
    va_copy(copy, arg);
    len = vsnprintf(NULL, 0, format, copy);
    va_end(copy);
    if (len >= sizeof(loc_buf))
    {
        temp = (char*)malloc(len+1);
        if(temp == NULL)
            return 0;
    }
    vsnprintf(temp, len+1, format, arg);
    (*printer)(temp);
    if(len >= sizeof(loc_buf))
        free(temp);
    return len;
}

int FLogger::printf(const char *format, ...)
{
    int len;
    va_list arg;
    va_start(arg, format);
    len = printfv(format, arg);
    va_end(arg);
    return len;
}
