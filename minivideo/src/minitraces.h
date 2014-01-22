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
 */

#include "cmake_defines.h"
#include "typedef.h"

#ifndef MINITRACES_H
#define MINITRACES_H
/* ************************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/******************************************************************************
                         PUBLIC INTERFACES
*******************************************************************************/

extern void MiniTraces_dump(void);
extern void MiniTraces_print(const char *file, const int line, const char *func,
                             const unsigned level, const unsigned module, const char *format, ...);

/******************************************************************************
                     GENERIC DEBUG LEVEL DEFINITIONS
*******************************************************************************/

#define TRACE_LEVEL_ERR    (1 << 0)
#define TRACE_LEVEL_WARN   (1 << 1)
#define TRACE_LEVEL_INFO   (1 << 2)
#define TRACE_LEVEL_1      (1 << 3)
#define TRACE_LEVEL_2      (1 << 4)
#define TRACE_LEVEL_3      (1 << 5)

#define TRACE_LEVEL_DEFAULT (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN)
#define TRACE_LEVEL_DEBUG   (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN | TRACE_LEVEL_INFO)
#define TRACE_LEVEL_ALL     (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN | TRACE_LEVEL_INFO | TRACE_LEVEL_1 | TRACE_LEVEL_2 | TRACE_LEVEL_3)

// =============================================================================
//                       USER DEFINED OPTIONS
// =============================================================================

#if ENABLE_COLORS == 1
#define DEBUG_WITH_COLORS      1
#else
#define DEBUG_WITH_COLORS      0
#endif

#define DEBUG_WITH_TIMESTAMPS  0
#define DEBUG_WITH_FILE_INFO   1
#define DEBUG_WITH_FORCED_SYNC 0

// =============================================================================
//                       USER DEFINED TRACE IDENTIFIER
// =============================================================================

// This is used to identify the project if multiple program are using minitraces
// at the same time. Leave blank if you dont need this.

#if ENABLE_COLORS == 1
#define TID ""
#else
#define TID ""
#endif

// =============================================================================
//                       USER DEFINED MODULES
// =============================================================================

typedef struct TraceModule_t
{
    char     module_name[16];
    char     module_description[64];
    unsigned module_mask;
} TraceModule_t;

enum TraceModule_e
{
    MAIN,
    BITS,
    IO,
    TOOL,
    PARSER,
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
    LAST_ID /* auto counter, do not remove */
};

/*!
 * Warning: trace declarations in this table should follow the order of the
 * type definition of MeTraces.h. It should be alphabetic.
 */
static TraceModule_t sv_trace_modules[] =
{
    { "MAIN"      , "Library main functions"     , TRACE_LEVEL_DEBUG },
    { "BITS"      , "Bitstream handling"         , TRACE_LEVEL_DEFAULT },
    { "I/O"       , "Input/output related"       , TRACE_LEVEL_DEFAULT },
    { "TOOL"      , "Useful functions"           , TRACE_LEVEL_DEFAULT },
    { "PARSER"    , "File parsing functions"     , TRACE_LEVEL_DEFAULT },
        { "FILTER", "IDR filter functionnality"  , TRACE_LEVEL_DEFAULT },
    { "MUXER"     , "Output ES or PES to file"   , TRACE_LEVEL_DEFAULT },
    { "H264"      , "H.264 decoder"              , TRACE_LEVEL_DEFAULT },
        { "NALU"  , "NAL Unit decoding"          , TRACE_LEVEL_DEFAULT },
        { "PARAM" , "Parameters Set"             , TRACE_LEVEL_DEFAULT },
        { "SLICE" , "Slice decoding"             , TRACE_LEVEL_DEFAULT },
        { "MACRO" , "MacroBlock decoding"        , TRACE_LEVEL_DEFAULT | TRACE_LEVEL_1 },
        { "SPACE" , "Spatial subdivision"        , TRACE_LEVEL_DEFAULT },
        { "INTRA" , "Intra prediction"           , TRACE_LEVEL_DEFAULT },
        { "INTER" , "Inter prediction"           , TRACE_LEVEL_DEFAULT },
        { "TRANS" , "Spatial transformation"     , TRACE_LEVEL_DEFAULT },
        { "EXPGO" , "Exp-Golomb decoding"        , TRACE_LEVEL_DEFAULT },
        { "CAVLC" , "CAVLC decoding"             , TRACE_LEVEL_DEFAULT },
        { "CABAC" , "CABAC decoding"             , TRACE_LEVEL_DEFAULT },
};

/******************************************************************************
                         COLORS DEFINITIONS
*******************************************************************************/

#include "colors.h"

/******************************************************************************
                         TRACES MACRO DEFINITIONS
*******************************************************************************/

#if ENABLE_DEBUG == 1

#define TRACE_ERROR( MODULE, ... )   MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_ERR,  MODULE, __VA_ARGS__ )
#define TRACE_WARNING( MODULE, ... ) MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_WARN, MODULE, __VA_ARGS__ )
#define TRACE_INFO( MODULE, ... )    MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_INFO, MODULE, __VA_ARGS__ )
#define TRACE_1( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_1,    MODULE, __VA_ARGS__ )
#define TRACE_2( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_2,    MODULE, __VA_ARGS__ )
#define TRACE_3( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_3,    MODULE, __VA_ARGS__ )

#else

#define TRACE_ERROR( MODULE, ... )   MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_ERR,  MODULE, __VA_ARGS__ )
#define TRACE_WARNING( MODULE, ... )
#define TRACE_INFO( MODULE, ... )
#define TRACE_1( MODULE, ... )
#define TRACE_2( MODULE, ... )
#define TRACE_3( MODULE, ... )

#endif

#ifdef __cplusplus
}
#endif // __cplusplus

/* ************************************************************************** */
#endif /* MINITRACES_H */
