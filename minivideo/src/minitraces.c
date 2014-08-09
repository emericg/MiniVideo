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
 * \version   0.3
 */

// C standard libraries
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "minitraces.h"

/* ************************************************************************** */

typedef struct TraceModule_t
{
    char     module_name[8];
    char     module_description[64];
    unsigned module_mask;
} TraceModule_t;

/*!
 * WARNING: The content of this structure should ALWAYS be in sync with the "TraceModule_e" enum in minitraces.h.
 * The following modules sould follow the exact same order that the one in "TraceModule_e" enum.
 */
static TraceModule_t sv_trace_modules[] =
{
    { "MAIN"      , "Library main functions"     , TRACE_LEVEL_DEBUG },
    { "BITS"      , "Bitstream handling"         , TRACE_LEVEL_DEFAULT /*TRACE_LEVEL_DEBUG | TRACE_LEVEL_1*/ },
    { "I/O"       , "Input/Output related"       , TRACE_LEVEL_DEFAULT },
    { "TOOLS"     , "Various useful functions"   , TRACE_LEVEL_DEFAULT },
    { "DEMUX"     , "File parsing functions"     , TRACE_LEVEL_DEFAULT },
        { "AVI"   , "AVI parser"                 , TRACE_LEVEL_DEFAULT },
        { "MP4"   , "MP4 parser"                 , TRACE_LEVEL_DEFAULT },
        { "MKV"   , "MKV parser"                 , TRACE_LEVEL_DEFAULT },
        { "MPS"   , "MPEG-PS parser"             , TRACE_LEVEL_DEFAULT },
        { "FILTR" , "IDR filtering functions"    , TRACE_LEVEL_DEFAULT },
    { "MUXER"     , "Output ES or PES to file"   , TRACE_LEVEL_DEFAULT },
    { "H.264"     , "H.264 decoder"              , TRACE_LEVEL_DEFAULT },
        { "NAL-U" , "NAL Unit decoding"          , TRACE_LEVEL_DEFAULT },
        { "PARAM" , "Parameters Set"             , TRACE_LEVEL_DEFAULT },
        { "SLICE" , "Slice decoding"             , TRACE_LEVEL_DEFAULT },
        { "MACRO" , "MacroBlock decoding"        , TRACE_LEVEL_DEFAULT },
        { "SPACE" , "Spatial subdivision"        , TRACE_LEVEL_DEFAULT },
        { "INTRA" , "Intra prediction"           , TRACE_LEVEL_DEFAULT },
        { "INTER" , "Inter prediction"           , TRACE_LEVEL_DEFAULT },
        { "TRANS" , "Spatial transformation"     , TRACE_LEVEL_DEFAULT },
        { "EXPGO" , "Exp-Golomb decoding"        , TRACE_LEVEL_DEFAULT },
        { "CAVLC" , "CAVLC decoding"             , TRACE_LEVEL_DEFAULT },
        { "CABAC" , "CABAC decoding"             , TRACE_LEVEL_DEFAULT },
};

/* ************************************************************************** */
/* ************************************************************************** */

static const unsigned int sf_trace_module_count = sizeof(sv_trace_modules) / sizeof(TraceModule_t);

#if DEBUG_WITH_TIMESTAMPS

#include <time.h>

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
#endif /* DEBUG_WITH_TIMESTAMPS */

/* ************************************************************************** */

/*!
 * \brief This function displays trace modules configuration (usually at program start-up)
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

/* ************************************************************************** */

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
        printf(TID "[%02x][%s] Trace Mask 0x%X: ", i,
               sv_trace_modules[i].module_name,
               sv_trace_modules[i].module_mask);

        sf_print_trace_levels(sv_trace_modules[i].module_mask);
    }
}

/* ************************************************************************** */

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

/* ************************************************************************** */
/* ************************************************************************** */

/*!
 * \brief This function displays what trace modules are currently enabled.
 */
void MiniTraces_dump(void)
{
    sf_dump_trace_config();
}

/* ************************************************************************** */

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
#if ENABLE_DEBUG
    if (module > sf_trace_module_count)
    {
        printf("[TRACE][%s] module[%d] unknown\n", __FUNCTION__, (int)module);
        return;
    }
#endif

    if ((sv_trace_modules[module].module_mask & level) == level)
    {
        // Trace program identifier
        ////////////////////////////////////////////////////////////////////////

        printf("%s", TID);

        // Trace header
        ////////////////////////////////////////////////////////////////////////

#if DEBUG_WITH_TIMESTAMPS
        // Print the trace precise timestamp
        clock_t ticks = clock() *1000 / CLOCKS_PER_SEC;
        printf("[%li ms] ", get_trace_ticks());
#endif

        // Trace module, 5 chars, left padded
        const char *level_string = sf_trace_level_string(level);
        printf("[%s][%5s]", level_string, sv_trace_modules[module].module_name);

#if DEBUG_WITH_FUNC_INFO
        // Print the function where the trace came from
        printf(BLD_WHITE "[%s]" CLR_RESET, func);
#endif

#if DEBUG_WITH_FILE_INFO
        // Print the line of code that triggered the trace output
        const char *tmp = strrchr(file, '/');
        printf("{%s:%d}", tmp ? ++tmp : file, line);
#endif

        // Customizable header / body separator
        ////////////////////////////////////////////////////////////////////////

        printf(" ");

        // Trace body
        ////////////////////////////////////////////////////////////////////////

        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);

#if DEBUG_WITH_FORCED_SYNC
        // Force terminal synchronisazion (very slow!)
        fflush(stdout);
#endif
    }
}

/* ************************************************************************** */
