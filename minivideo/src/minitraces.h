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
 * \file      minitraces.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2014
 * \version   0.3
 */

#ifndef MINITRACES_H
#define MINITRACES_H
/* ************************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Settings
#include "cmake_defines.h"
#include "typedef.h"

/* ************************************************************************** */

extern void MiniTraces_dump(void);
extern void MiniTraces_print(const char *file, const int line, const char *func,
                             const unsigned level, const unsigned module, const char *format, ...);

/* ************************************************************************** */

// =============================================================================
//                       USER DEFINED SETTINGS
// =============================================================================

// Enable terminal colored output
#if ENABLE_COLORS == 1
#define DEBUG_WITH_COLORS      1
#else
#define DEBUG_WITH_COLORS      0
#endif

// Advanced debugging settings
#define DEBUG_WITH_TIMESTAMPS  0
#define DEBUG_WITH_FUNC_INFO   1
#define DEBUG_WITH_FILE_INFO   1
#define DEBUG_WITH_FORCED_SYNC 0

// =============================================================================
//                       USER DEFINED TRACE PROGRAM IDENTIFIER
// =============================================================================

// This is used to identify the project if multiple program are using minitraces
// at the same time. Leave blank if you don't need this feature!

#define TID ""

// =============================================================================
//                       USER DEFINED MODULES
// =============================================================================

/*!
 * WARNING: The content of this enum should ALWAYS be in sync with the "sv_trace_modules" structure in minitraces.h.
 * The following modules sould follow the exact same order that the one in "sv_trace_modules" structure.
 */
enum TraceModule_e
{
    MAIN,
    BITS,
    IO,
    TOOL,
    DEMUX,
        AVI,
        MP4,
        MKV,
        MPS,
        FILTER,
    MUXER,
    H264,
        NALU,
        PARAM,
        SLICE,
        MB,
        SPATIAL,
        INTRA,
        INTER,
        TRANS,
        EXPGO,
        CAVLC,
        CABAC,
};

/* ************************************************************************** */

// GENERIC DEBUG LEVEL DEFINITIONS

#define TRACE_LEVEL_ERR     (1 << 0)
#define TRACE_LEVEL_WARN    (1 << 1)
#define TRACE_LEVEL_INFO    (1 << 2)
#define TRACE_LEVEL_1       (1 << 3)
#define TRACE_LEVEL_2       (1 << 4)
#define TRACE_LEVEL_3       (1 << 5)

#define TRACE_LEVEL_DEFAULT (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN)
#define TRACE_LEVEL_DEBUG   (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN | TRACE_LEVEL_INFO)
#define TRACE_LEVEL_ALL     (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN | TRACE_LEVEL_INFO | TRACE_LEVEL_1 | TRACE_LEVEL_2 | TRACE_LEVEL_3)

// TRACE MACROS

#if ENABLE_DEBUG == 1

#define TRACE_ERROR( MODULE, ... )   MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_ERR,  MODULE, __VA_ARGS__ )
#define TRACE_WARNING( MODULE, ... ) MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_WARN, MODULE, __VA_ARGS__ )
#define TRACE_INFO( MODULE, ... )    MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_INFO, MODULE, __VA_ARGS__ )
#define TRACE_1( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_1,    MODULE, __VA_ARGS__ )
#define TRACE_2( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_2,    MODULE, __VA_ARGS__ )
#define TRACE_3( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_3,    MODULE, __VA_ARGS__ )

#else

#define TRACE_ERROR( MODULE, ... )   MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_ERR,  MODULE, __VA_ARGS__ )
#define TRACE_WARNING( MODULE, ... ) MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_WARN, MODULE, __VA_ARGS__ )
#define TRACE_INFO( MODULE, ... )
#define TRACE_1( MODULE, ... )
#define TRACE_2( MODULE, ... )
#define TRACE_3( MODULE, ... )

#endif /* ENABLE_DEBUG */

#ifdef __cplusplus
}
#endif // __cplusplus

/* ************************************************************************** */
#endif /* MINITRACES_H */
