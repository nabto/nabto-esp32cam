#include <modules/log/unabto_log_header.h>

#include <modules/log/unabto_basename.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

int unabto_log_header(const char* file, unsigned int line)
{
    time_t sec;
    unsigned int ms;
    struct timeval tv;
    struct tm tm;
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec;
    ms = tv.tv_usec/1000;

    localtime_r(&sec, &tm);

    return printf("%02u:%02u:%02u:%03u %s(%u) ",
                  tm.tm_hour, tm.tm_min, tm.tm_sec, ms,
                  unabto_basename(file), (line));
}
