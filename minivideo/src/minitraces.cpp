/*!
 * COPYRIGHT (C) 2018 Emeric Grange - All Rights Reserved
 *
 * This file is part of MiniTraces.
 *
 * MiniTraces is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MiniTraces is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MiniTraces.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \file      minitraces.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 * \version   0.51
 */

// MiniTraces header
#include "minitraces.h"

// C standard libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
#define MINITRACES_EOL "\n\r"
#else // linux, macOS, and everyone else...
#define MINITRACES_EOL "\n"
#endif

/* ************************************************************************** */
/* ************************************************************************** */

#if MINITRACES_TIMESTAMPS

#include <time.h>

#ifdef defined(__APPLE__) || defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/*!
 * \brief Print trace tick with millisecond precision.
 *
 * \note If CLOCK_MONOTONIC_RAW is not available on your system, you can fallback
 * to CLOCK_MONOTONIC or even CLOCK_REALTIME.
 */
static void print_trace_tick(void)
{
    long long int time = 0;

#if defined(__APPLE__) || defined(__MACH__) // OS X does not have clock_gettime > use clock_get_time

    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    if (clock_get_time(cclock, &mts));
    {
        time = (mts.tv_sec * 1000) + (mts.tv_nsec / 1000000);
    }
    mach_port_deallocate(mach_task_self(), cclock);

#else

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        time = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    }

#endif

    printf("[%lld]", time);
}

/*!
 * \brief Print trace time with hh:mm:ss format, with second precision.
 */
static void print_trace_time(void)
{
    time_t timer;
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    if (tm_info != NULL)
    {
        char buffer[12];
        strftime(buffer, 12, "%H:%M:%S", tm_info);
        printf("[%s]", buffer);
    }
    else
    {
        printf("[0]");
    }
}

#endif // MINITRACES_TIMESTAMPS

/* ************************************************************************** */

/*!
 * \brief This function displays trace level enabled for a particular module.
 */
static void print_trace_levels(const unsigned level)
{
    printf("%s%s%s%s%s%s", (level & TRACE_LEVEL_ERR)  ? "ERR":"",
                           (level & TRACE_LEVEL_WARN) ? " | WARN":"",
                           (level & TRACE_LEVEL_INFO) ? " | INFO":"",
                           (level & TRACE_LEVEL_1)    ? " | LVL 1":"",
                           (level & TRACE_LEVEL_2)    ? " | LVL 2":"",
                           (level & TRACE_LEVEL_3)    ? " | LVL 3":"");
    printf("\n");
}

/* ************************************************************************** */

/*!
 * Gives out a 1 letter description of the trace level.
 */
static const char *get_trace_level_string(unsigned level)
{
    const char *string = "U";

    switch (level)
    {
        case TRACE_LEVEL_ERR:   string = BLD_RED    "E" CLR_RESET; break;
        case TRACE_LEVEL_WARN:  string = BLD_YELLOW "W" CLR_RESET; break;
        case TRACE_LEVEL_INFO:  string = BLD_GREEN  "I" CLR_RESET; break;
        case TRACE_LEVEL_1:     string = BLD_WHITE  "1" CLR_RESET; break;
        case TRACE_LEVEL_2:     string = BLD_WHITE  "2" CLR_RESET; break;
        case TRACE_LEVEL_3:     string = BLD_WHITE  "3" CLR_RESET; break;

        default: break;
    }

    return string;
}

/* ************************************************************************** */

//! Count all of the user-defined trace modules
static const unsigned int trace_module_count = sizeof(trace_modules_table) / sizeof(TraceModule_t);

/* ************************************************************************** */
/* ************************************************************************** */

void MiniTraces_info(void)
{
    printf(PID BLD_GREEN "\nMiniTraces_infos()" CLR_RESET " version 0.4\n");

    printf(PID "\n* TRACE LEVELS ENABLED:\n");
    TRACE_ERROR(MAIN, "ERROR traces enabled");
    TRACE_WARNING(MAIN, "WARNING traces enabled");
    TRACE_INFO(MAIN, "INFO traces enabled");
    TRACE_1(MAIN, "LVL 1 traces enabled");
    TRACE_2(MAIN, "LVL 2 traces enabled");
    TRACE_3(MAIN, "LVL 3 traces enabled");

    printf(PID "\n* TRACE MODULES CONFIGURATION:\n");
    for (unsigned i = 0; i < trace_module_count; i++)
    {
        printf(PID "[%02x][%5s] Trace Mask 0x%X: ", i,
               trace_modules_table[i].module_name,
               trace_modules_table[i].module_output_mask);

        print_trace_levels(trace_modules_table[i].module_output_mask);
    }
}

/* ************************************************************************** */

void MiniTraces_print(const char *file, const int line, const char *func,
                      const unsigned level, const unsigned module, const char *payload, ...)
{
#if MINITRACES_LEVEL > 0
    if (module > trace_module_count)
    {
        printf("[TRACE][%s] module[%d] unknown\n", __FUNCTION__, (int)module);
        return;
    }
#endif

    if ((trace_modules_table[module].module_output_mask & level) == level)
    {
        // Trace program identifier
        ////////////////////////////////////////////////////////////////////////

        printf("%s", PID);

        // Trace header
        ////////////////////////////////////////////////////////////////////////

        // Print the trace timestamp
#if MINITRACES_TIMESTAMPS == 1
        print_trace_tick();
#elif MINITRACES_TIMESTAMPS == 2
        print_trace_time();
#endif

        // Print trace module_name, 5 chars, left padded
        const char *level_string = get_trace_level_string(level);
        printf("[%s][%5s]", level_string, trace_modules_table[module].module_name);

#if MINITRACES_FUNC_INFO
        // Print the function where the trace came from
        printf(BLD_WHITE "[%s]" CLR_RESET, func);
#endif

#if MINITRACES_FILE_INFO
        // Print the line of code that triggered the trace output
        const char *tmp = strrchr(file, '/');
        printf("{%s:%d}", tmp ? ++tmp : file, line);
#endif

        // Trace body
        ////////////////////////////////////////////////////////////////////////

        // Customizable header / body separator
        printf(" ");

        va_list args;
        va_start(args, payload);
        vprintf(payload, args);
        va_end(args);

        // End of the line
        printf(MINITRACES_EOL);

#if MINITRACES_FORCED_SYNC
        // Force terminal synchronisazion (very slow!)
        fflush(stdout);
#endif
    }
}

/* ************************************************************************** */
