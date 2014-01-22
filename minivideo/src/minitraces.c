/*!
 * COPYRIGHT (C) 2014 Emeric Grange - All Rights Reserved
 *
 * This file is part of MiniVideo.
 *
 * MiniVideo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MiniVideo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MiniVideo.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \file      minitraces.c
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 */

// C standard libraries
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "minitraces.h"

/******************************************************************************
                   STATIC FUNCTION IMPLEMENTATION
*******************************************************************************/

static const unsigned int sf_trace_module_count = sizeof(sv_trace_modules) / sizeof(TraceModule_t);

/*!
 * If CLOCK_MONOTONIC_RAW is not available on your system, you can fallback to
 * CLOCK_MONOTONIC or even CLOCK_REALTIME.
 */
static unsigned get_trace_ticks(void)
{
    unsigned time = 0;
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    time = tp.tv_sec * 1000 + tp.tv_nsec/1000000;

    return time;
}

/*!
 * \brief This function displays (normally at boot-up) trace modules configuration
 */
static void sf_print_trace_levels(const unsigned level)
{
    printf("%s%s%s%s%s%s",
           (level & TRACE_LEVEL_ERR)  ? "ERR":"",
           (level & TRACE_LEVEL_WARN) ? " | WARN":"",
           (level & TRACE_LEVEL_INFO) ? " | INFO":"",
           (level & TRACE_LEVEL_1)    ? " | L1":"",
           (level & TRACE_LEVEL_2)    ? " | L2":"",
           (level & TRACE_LEVEL_3)    ? " | L3":"");
    printf("\n");
}

/*!
 * \brief This function displays (normally at boot-up) trace modules configuration
 */
static void sf_dump_trace_config(void)
{
    unsigned i = 0;

    printf(TID "TRACE CONFIGURATION: \n");
    printf(TID "ERROR   %#x\n", TRACE_LEVEL_ERR);
    printf(TID "WARNING %#x\n", TRACE_LEVEL_WARN);
    printf(TID "INFO    %#x\n", TRACE_LEVEL_INFO);
    printf(TID "LEVEL_1 %#x\n", TRACE_LEVEL_1);
    printf(TID "LEVEL_2 %#x\n", TRACE_LEVEL_2);
    printf(TID "LEVEL_3 %#x\n", TRACE_LEVEL_3);

    for (i = 0; i < sf_trace_module_count; i++)
    {
        printf(TID"[%02x][%s] Trace Mask 0x%X: ", i,
               sv_trace_modules[i].module_name,
               sv_trace_modules[i].module_mask);

        sf_print_trace_levels(sv_trace_modules[i].module_mask);
    }
}

/*!
 * Gives out a 2-character description of the trace level.
 * This is more readable than the numeric value of the level.
 * If colors are enabled, the level will be colored.
 */
static const char *sf_trace_level_string(unsigned level)
{
    const char *string = "U";

    switch (level)
    {
        case TRACE_LEVEL_3:     string = BLD_WHITE  "3" CLR_RESET; break;
        case TRACE_LEVEL_2:     string = BLD_WHITE  "2" CLR_RESET; break;
        case TRACE_LEVEL_1:     string = BLD_WHITE  "1" CLR_RESET; break;
        case TRACE_LEVEL_INFO:  string = BLD_GREEN  "I" CLR_RESET; break;
        case TRACE_LEVEL_WARN:  string = BLD_YELLOW "W" CLR_RESET; break;
        case TRACE_LEVEL_ERR:   string = BLD_RED    "E" CLR_RESET; break;
        default: break;
    }

    return string;
}

/******************************************************************************
                   PUBLIC FUNCTION IMPLEMENTATION
*******************************************************************************/

/*!
 * \brief This function displays what trace modules are currently enabled.
 */
void MiniTraces_dump(void)
{
    sf_dump_trace_config();
}

/*!
 * \brief This function formats output traces.
 * \param file  : current file
 * \param line  : line in file
 * \param func  : current function
 * \param level : trace level
 * \param module: module called
 * \param format: payload
 *
 * This function must never be directly called, use macro instead!
 */
void MiniTraces_print(const char *file, const int line, const char *func,
                      const unsigned level, const unsigned module, const char *format, ...)
{
    // Print trace module header
    printf(TID);

    if (module > sf_trace_module_count)
    {
        printf("[TRACE][%s] module[%d] unknown\n", __FUNCTION__, (int)module);
        return;
    }

    if ((sv_trace_modules[module].module_mask & level) == level)
    {
        // Retrieve all arguments
        va_list args;
        va_start(args, format);

        // Format trace header

#if DEBUG_WITH_TIME_STAMP
        clock_t ticks = clock() *1000 / CLOCKS_PER_SEC;
        printf("[%li ms] ", get_trace_ticks());
#endif
        const char *level_string = sf_trace_level_string(level);
        const char *tmp = strrchr(file, '/');
        printf("[%s][%s]" BLD_WHITE "[%s]" CLR_RESET "{%s:%d} > ", level_string, sv_trace_modules[module].module_name, func, tmp ? ++tmp : file, line);

        // Trace outputted
        vprintf(format, args);
        va_end(args);

#if DEBUG_WITH_FORCED_SYNC
        // Force terminal synchronisazion (slow)
        fflush(stdout);
#endif
    }
}
