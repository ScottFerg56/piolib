#include "FLogger.h"

namespace FLogger
{
    int defaultPrinter(const char* s) { return Serial.print(s); }
    flog_print_t printer = &defaultPrinter;
    flog_level logLevel = FLOG_ERROR;

    void setLogLevel(flog_level level) { logLevel = level; }
    flog_level getLogLevel() { return logLevel; }
    void setPrinter(flog_print_t func) { printer = func; }

    int printfv(flog_level level, const char *format, va_list arg)
    {
        if (level > logLevel)
            return 0;
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
        while (level == FLOG_FATAL)
        {
            delay(1000);
            (*printer)(temp);
        }
        if(len >= sizeof(loc_buf))
            free(temp);
        return len;
    }

    int printf(flog_level level, const char *format, ...)
    {
        if (level > logLevel)
            return 0;
        int len;
        va_list arg;
        va_start(arg, format);
        len = printfv(level, format, arg);
        va_end(arg);
        return len;
    }
}
