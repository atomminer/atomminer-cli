/*
 * AtomMiner util Lib
 * Copyright AtomMiner, 2018,
 * All Rights Reserved
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF ATOMMINER
 *
 *      Author: AtomMiner - null3128
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <mutex>
#include "log.h"
#include "types.h"

#include "fmt/format.h"

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#define LOG_NN 0x10 /* unique value */
#else
enum {
    LOG_ERR,
    LOG_WARNING,
    LOG_NOTICE,
    LOG_INFO,
    LOG_DEBUG,
    /* custom notices */
    LOG_NN = 0x10,
};
#endif

#define CL_N    "\x1B[0m"
#define CL_RED  "\x1B[31m"
#define CL_GRN  "\x1B[32m"
#define CL_YLW  "\x1B[33m"
#define CL_BLU  "\x1B[34m"
#define CL_MAG  "\x1B[35m"
#define CL_CYN  "\x1B[36m"

#define CL_BLK  "\x1B[22;30m" /* black */
#define CL_RD2  "\x1B[22;31m" /* red */
#define CL_GR2  "\x1B[22;32m" /* green */
#define CL_BRW  "\x1B[22;33m" /* brown */
#define CL_BL2  "\x1B[22;34m" /* blue */
#define CL_MA2  "\x1B[22;35m" /* magenta */
#define CL_CY2  "\x1B[22;36m" /* cyan */
#define CL_SIL  "\x1B[22;37m" /* gray */
#define CL_WHT  "\x1B[01;37m" /* white */

#ifdef WIN32
#define CL_GRY  "\x1B[01;30m" /* dark gray */
#else
#define CL_GRY  "\x1B[90m"    /* dark gray selectable in putty */
#endif

std::mutex g_atmlogglobalmutex;

void _llog(int nLevel, const char *ss)
{
    const char* color = "";
    struct tm tm;
    time_t now = time(NULL);

    lock_t lock(g_atmlogglobalmutex);

    localtime_r(&now, &tm);

    switch(nLevel)
    {
        case LOG_ERR:     color = CL_RED; break;
        case LOG_WARNING: color = CL_YLW; break;
        case LOG_NOTICE:  color = CL_WHT; break;
        case LOG_DEBUG:   color = CL_GRY; break;
        case LOG_NN:      color = CL_CYN; break;
        case LOG_INFO:
        default:
            break;
    }

    //int len = 64 + (int) strlen(fmt) + 2;
    //f = (char*) malloc(len);
    fprintf(stdout, "\r[%d-%02d-%02d %02d:%02d:%02d]%s %s%s\n",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        color,
        ss,
        CL_N
    );

    fflush(stdout);
}

#define CHECK_S \
if(s.length() == 0) \
    return; \
if(s[s.length()-1] == '\n') \
    s.erase(s.length()-1, 1); \

void log(std::string s)
{
    CHECK_S;
    _llog(LOG_INFO, s.c_str());
}

// error
void loge(std::string s)
{
    CHECK_S;
    _llog(LOG_ERR, s.c_str());
}

void logw(std::string s)
{
    CHECK_S;
    _llog(LOG_WARNING, s.c_str());
}
void logi(std::string s)
{
    CHECK_S;
    _llog(LOG_INFO, s.c_str());
}
void logd(std::string s)
{
    CHECK_S;
    _llog(LOG_DEBUG, s.c_str());
}

void logproto(std::string s)
{
    CHECK_S;
    _llog(LOG_DEBUG, s.c_str());
}

void logn(std::string s)
{
    CHECK_S;
    _llog(LOG_NN, s.c_str());
}

static char gProgressSymbolArr[] = "\\|/*";
static uint32_t   gProgressSymbolCounter = 0;

void logprogress(std::string s)
{
    if(s.length() == 0)
        return;
    gProgressSymbolCounter++;

    s = fmt::format("{} {}", (char)gProgressSymbolArr[gProgressSymbolCounter % (sizeof(gProgressSymbolArr)-1)], s);
    fprintf(stdout, "\33[2K\r%s" CL_N, s.c_str());
    fflush(stdout);
}

void logprogressK(std::string s)
{
    if(s.length() > 0 && s[s.length()-1] != '\n')
        s += "\n";
    fprintf(stdout, "\33[2K\r" CL_GRN "+" CL_N " %s" CL_N, s.c_str());
    fflush(stdout);
}

void logprogressF(std::string s)
{
    if(s.length() > 0 && s[s.length()-1] != '\n')
        s += "\n";
    fprintf(stdout, "\33[2K\r" CL_RED "-" CL_N " %s" CL_N, s.c_str());
    fflush(stdout);
}
