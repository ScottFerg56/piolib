#ifndef FLogger_h
#define FLogger_h

#include <Arduino.h>

// ANSI Color Codes: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
#define RedText "\e[1;31m"
#define GreenText "\e[1;32m"
#define YellowText "\e[1;33m"
#define BlueText "\e[1;34m"
#define MagentaText "\e[1;35m"
#define CyanText "\e[1;36m"
#define WhiteText "\e[1;37m"
#define ResetText "\e[0m"

#define FLOG_FORMAT(letter, color, format)  "[" #letter "][%s:%u] %s(): " color format ResetText "\r\n" , (unsigned long) pathToFileName(__FILE__), __LINE__, __FUNCTION__
//#define FLOG_FORMAT(letter, format)  "[" #letter "][%s:%u] %s(): " format "\r\n", (unsigned long) pathToFileName(__FILE__), __LINE__, __FUNCTION__
//#define FLOG_FORMAT(letter, format)  "[%6u][" #letter "][%s:%u] %s(): " format "\r\n", (unsigned long) (esp_timer_get_time() / 1000ULL), pathToFileName(__FILE__), __LINE__, __FUNCTION__

enum flog_level
{
    FLOG_NONE,       /* No log output */
    FLOG_FATAL,      /* Fatal errors, put processor into ain infinite wait loop */
    FLOG_ERROR,      /* Critical errors, software module can not recover on its own */
    FLOG_WARN,       /* Error conditions from which recovery measures have been taken */
    FLOG_INFO,       /* Information messages which describe normal flow of events */
    FLOG_DEBUG,      /* Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    FLOG_VERBOSE     /* Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
};

#define flogf(format, ...) FLogger::printf(FLOG_FATAL,   FLOG_FORMAT(F, RedText,    format), ##__VA_ARGS__)
#define floge(format, ...) FLogger::printf(FLOG_ERROR,   FLOG_FORMAT(E, RedText,    format), ##__VA_ARGS__)
#define flogw(format, ...) FLogger::printf(FLOG_WARN,    FLOG_FORMAT(W, YellowText, format), ##__VA_ARGS__)
#define flogi(format, ...) FLogger::printf(FLOG_INFO,    FLOG_FORMAT(I, CyanText,   format), ##__VA_ARGS__)
#define flogd(format, ...) FLogger::printf(FLOG_DEBUG,   FLOG_FORMAT(D, BlueText,   format), ##__VA_ARGS__)
#define flogv(format, ...) FLogger::printf(FLOG_VERBOSE, FLOG_FORMAT(V, WhiteText,  format), ##__VA_ARGS__)

typedef int (*flog_print_t)(const char *);

namespace FLogger
{
    void setLogLevel(flog_level level);
    flog_level getLogLevel();
    void setPrinter(flog_print_t func);
    int printfv(flog_level level, const char *format, va_list);
    int printf(flog_level level, const char *format, ...);
}

#endif // FLogger_h