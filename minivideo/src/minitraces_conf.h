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
 * \file      minitraces_conf.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 * \version   0.51
 */

#ifndef MINITRACES_CONF_H
#define MINITRACES_CONF_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
/* ************************************************************************** */

// =============================================================================
// GENERAL SETTINGS
// =============================================================================

// Import setting macros from minivideo, and map them to the ones from MiniTraces.
#include "minivideo_settings.h"

#if ENABLE_DEBUG == 1
#define MINITRACES_LEVEL    2   // Enables all traces levels
#else
#define MINITRACES_LEVEL    1   // Only error and warnings traces
#endif

// Enable terminal colored output
#if ENABLE_COLORS == 1
#undef MINITRACES_COLORS
#define MINITRACES_COLORS   1
#else
#undef ENABLE_COLORS
#define MINITRACES_COLORS   0
#endif

// Advanced debugging features
#define MINITRACES_TIMESTAMPS       0   //!< 0: disabled, 1: trace tick (in milliseconds), 2: trace time (hh:mm:ss)
#define MINITRACES_FUNC_INFO        0
#define MINITRACES_FILE_INFO        1
#define MINITRACES_FORCED_SYNC      0
#define MINITRACES_STRICT_PADDING   0   //!< Not implemented yet

// =============================================================================
// PROGRAM IDENTIFIER
// =============================================================================

/*!
 * This string will be used to easily identify from which program a trace comes
 * from if multiple program are outputting traces with MiniTraces at the same time.
 * You can use brackets, spaces, colors...
 * Example: #define PID OUT_BLUE "[MINITRACE]" CLR_RESET " "
 *
 * Leave it blank if you don't need this feature!
 */
#define PID ""

// =============================================================================
// TRACE MODULES
// =============================================================================

/*!
 * \brief This is the list of module you can use when creating a trace with a TRACE_xxx macro.
 * \note The content of this enum must ALWAYS be in sync with the trace_modules_table[] below.
 *
 * When a TRACE_xxx macro is called with a given module, it will try to match it
 * with a TraceModule_t entry inside the trace_modules_table[] to get access to
 * the module parameters.
 */
enum TraceModule_e
{
    MAIN,
    IO,
        BITS,
        BYTES,
    TOOL,
    DEMUX,
        MKV,
        MP4,
        MP3,
        AVI,
        ASF,
        WAV,
        MPS,
        MTS,
        RIF,
        AIF,
        CAF,
        AU,
        FILTR,
        MAPPR,
    MUXER,
    DEPAK,
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

/*!
 * \brief This is the list of trace modules defined in your project.
 * \note The content of this enum must ALWAYS be in sync with the TraceModule_e enum above.
 *
 * - The first field is the "public" module's name, as it will be outputted by MiniTraces.
 * - The second field holds the description of the module.
 * - The last field indicate the level of traces a module can output, using one or a concatenation of TRACE_LEVEL_xxx macros.
 */
static TraceModule_t trace_modules_table[] =
{
    { "MAIN"      , "MiniVideo library main"    , TRACE_LEVEL_DEFAULT },
    { "I/O"       , "Input/Output related"      , TRACE_LEVEL_DEFAULT },
        { "BITS"  , "Bitstream handling"        , TRACE_LEVEL_DEFAULT },
        { "BYTES" , "Bytestream handling"       , TRACE_LEVEL_DEFAULT },
    { "TOOLS"     , "Various useful functions"  , TRACE_LEVEL_DEFAULT },
    { "DEMUX"     , "File parsing functions"    , TRACE_LEVEL_DEFAULT },
        { "MKV"   , "MKV parser"                , TRACE_LEVEL_DEFAULT },
        { "MP4"   , "MP4 parser"                , TRACE_LEVEL_DEFAULT },
        { "MP3"   , "MP3 parser"                , TRACE_LEVEL_DEFAULT },
        { "AVI"   , "AVI parser"                , TRACE_LEVEL_DEFAULT },
        { "ASF"   , "ASF parser"                , TRACE_LEVEL_DEFAULT },
        { "WAV"   , "WAVE parser"               , TRACE_LEVEL_DEFAULT },
        { "MPS"   , "MPEG-PS parser"            , TRACE_LEVEL_DEFAULT },
        { "MTS"   , "MPEG-TS parser"            , TRACE_LEVEL_DEFAULT },
        { "RIF"   , "RIFF parser utils"         , TRACE_LEVEL_DEFAULT },
        { "AIF"   , "AIFF parser"               , TRACE_LEVEL_DEFAULT },
        { "CAF"   , "CAF parser"                , TRACE_LEVEL_DEFAULT },
        { "AU"    , "AU parser"                 , TRACE_LEVEL_DEFAULT },
        { "FILTR" , "IDR filtering functions"   , TRACE_LEVEL_DEFAULT },
        { "MAPPR" , "XML Mapper"                , TRACE_LEVEL_DEFAULT },
    { "MUX"       , "Output ES or PES to file"  , TRACE_LEVEL_DEFAULT },
    { "DEPAK"     , "Depacketizers"             , TRACE_LEVEL_DEFAULT },
    { "H.264"     , "H.264 decoder"             , TRACE_LEVEL_DEFAULT },
        { "NAL-U" , "NAL Unit decoding"         , TRACE_LEVEL_DEFAULT },
        { "PARAM" , "Parameters Set"            , TRACE_LEVEL_DEFAULT },
        { "SLICE" , "Slice decoding"            , TRACE_LEVEL_DEFAULT },
        { "MACRO" , "Macroblock decoding"       , TRACE_LEVEL_DEFAULT },
        { "SPACE" , "Spatial subdivision"       , TRACE_LEVEL_DEFAULT },
        { "INTRA" , "Intra prediction"          , TRACE_LEVEL_DEFAULT },
        { "INTER" , "Inter prediction"          , TRACE_LEVEL_DEFAULT },
        { "TRANS" , "Spatial transformation"    , TRACE_LEVEL_DEFAULT },
        { "EXPGO" , "Exp-Golomb decoding"       , TRACE_LEVEL_DEFAULT },
        { "CAVLC" , "CAVLC decoding"            , TRACE_LEVEL_DEFAULT },
        { "CABAC" , "CABAC decoding"            , TRACE_LEVEL_DEFAULT },
};

/* ************************************************************************** */
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MINITRACES_CONF_H
