#ifndef FLogger_h
#define FLogger_h

#include <Arduino.h>

#define FLOG_FORMAT(letter, format)  "[" #letter "] %s(): " format "\r\n", __FUNCTION__
//#define FLOG_FORMAT(letter, format)  "[" #letter "][%s:%u] %s(): " format "\r\n", (unsigned long) pathToFileName(__FILE__), __LINE__, __FUNCTION__
//#define FLOG_FORMAT(letter, format)  "[%6u][" #letter "][%s:%u] %s(): " format "\r\n", (unsigned long) (esp_timer_get_time() / 1000ULL), pathToFileName(__FILE__), __LINE__, __FUNCTION__

#define flogd(format, ...) FLogger::flog.printf(FLOG_FORMAT(D, format), ##__VA_ARGS__)
#define plot(name1, name2, v) Serial.printf(">%s %s:%i\r\n", name1, name2, (int)(v))

typedef int (*flog_print_t)(const char *);

class FLogger
{
protected:
    static flog_print_t printer;

    static int default_printer(const char* s)
    {
        return Serial.print(s);
    }

    FLogger() { printer = &default_printer; }


public:
    static FLogger flog;
    static int printfv(const char *format, va_list);
    static int printf(const char *format, ...);

    static void SetPrinter(flog_print_t func)
    {
        printer = func;
    }

    FLogger(FLogger &other) = delete;           // not cloneable
    void operator=(const FLogger &) = delete;   // not assignable
};

#endif // FLogger_h