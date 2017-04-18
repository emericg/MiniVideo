/*!
 * COPYRIGHT (C) 2017 Emeric Grange - All Rights Reserved
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
 * \file      minitraces.h
 * \author    Emeric Grange <emeric.grange@gmail.com>
 * \date      2017
 * \version   0.51
 */

#ifndef MINITRACES_H
#define MINITRACES_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
/* ************************************************************************** */

/*!
 * This function displays informations about MiniTraces, which "trace levels"
 * and "trace modules" are currently enabled. Its output is quite heavy so you
 * should keep this for debug builds.
 */
void MiniTraces_info(void);

/*!
 * \brief This function formats and output traces.
 * \param file: current file where the trace call originated.
 * \param line: line in file.
 * \param func: current function.
 * \param level: trace level.
 * \param module: module called.
 * \param payload: the actual trace string.
 *
 * \note You sould never call this function directly, please use TRACE_xxx macros instead!
 */
void MiniTraces_print(const char *file, const int line, const char *func,
                      const unsigned level, const unsigned module, const char *payload, ...);

/* ************************************************************************** */

// TRACE LEVELS

#define TRACE_LEVEL_ERR     (1 << 0)
#define TRACE_LEVEL_WARN    (1 << 1)
#define TRACE_LEVEL_INFO    (1 << 2)
#define TRACE_LEVEL_1       (1 << 3)
#define TRACE_LEVEL_2       (1 << 4)
#define TRACE_LEVEL_3       (1 << 5)

#define TRACE_LEVEL_DEFAULT (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN)
#define TRACE_LEVEL_DEBUG   (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN | TRACE_LEVEL_INFO)
#define TRACE_LEVEL_ALL     (TRACE_LEVEL_ERR | TRACE_LEVEL_WARN | TRACE_LEVEL_INFO | TRACE_LEVEL_1 | TRACE_LEVEL_2 | TRACE_LEVEL_3)

/* ************************************************************************** */

typedef struct TraceModule_t
{
    char     module_name[8];            //!< Module name.
    char     module_description[32];    //!< Module description.
    unsigned module_output_mask;        //!< Stores the level of traces a module can output, using one or a concatenation of TRACE_LEVEL_xxx macros.
} TraceModule_t;

/* ************************************************************************** */

// Load settings and trace modules
#include "minitraces_conf.h"

/* ************************************************************************** */

#if MINITRACES_LEVEL == 2

// TRACE MACROS, fully enabled
#define TRACE_ERROR( MODULE, ... )   MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_ERR,  MODULE, __VA_ARGS__ )
#define TRACE_WARNING( MODULE, ... ) MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_WARN, MODULE, __VA_ARGS__ )
#define TRACE_INFO( MODULE, ... )    MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_INFO, MODULE, __VA_ARGS__ )
#define TRACE_1( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_1,    MODULE, __VA_ARGS__ )
#define TRACE_2( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_2,    MODULE, __VA_ARGS__ )
#define TRACE_3( MODULE, ... )       MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_3,    MODULE, __VA_ARGS__ )

#elif MINITRACES_LEVEL == 1

// TRACE MACROS, release config
#define TRACE_ERROR( MODULE, ... )   MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_ERR,  MODULE, __VA_ARGS__ )
#define TRACE_WARNING( MODULE, ... ) MiniTraces_print( __FILE__, __LINE__, __FUNCTION__, TRACE_LEVEL_WARN, MODULE, __VA_ARGS__ )
#define TRACE_INFO( MODULE, ... )
#define TRACE_1( MODULE, ... )
#define TRACE_2( MODULE, ... )
#define TRACE_3( MODULE, ... )

#else // MINITRACES_LEVEL == 0

// TRACE MACROS disabled
#define TRACE_ERROR( MODULE, ... )
#define TRACE_WARNING( MODULE, ... )
#define TRACE_INFO( MODULE, ... )
#define TRACE_1( MODULE, ... )
#define TRACE_2( MODULE, ... )
#define TRACE_3( MODULE, ... )

#endif // MINITRACES_LEVEL

/* ************************************************************************** */

#if MINITRACES_COLORS == 1

#define CLR_RESET  "\e[0m" //!< Reset colored output to default color of the terminal

// Regular colored text
#define CLR_BLACK  "\e[0;30m"
#define CLR_RED    "\e[0;31m"
#define CLR_GREEN  "\e[0;32m"
#define CLR_YELLOW "\e[0;33m"
#define CLR_BLUE   "\e[0;34m"
#define CLR_PURPLE "\e[0;35m"
#define CLR_CYAN   "\e[0;36m"
#define CLR_WHITE  "\e[0;37m"

// Bold colored text
#define BLD_BLACK  "\e[1;30m"
#define BLD_RED    "\e[1;31m"
#define BLD_GREEN  "\e[1;32m"
#define BLD_YELLOW "\e[1;33m"
#define BLD_BLUE   "\e[1;34m"
#define BLD_PURPLE "\e[1;35m"
#define BLD_CYAN   "\e[1;36m"
#define BLD_WHITE  "\e[1;37m"

// Bold white text, colored outline
#define OUT_BLACK  "\e[1;37;40m"
#define OUT_RED    "\e[1;37;41m"
#define OUT_GREEN  "\e[1;37;42m"
#define OUT_YELLOW "\e[1;37;43m"
#define OUT_BLUE   "\e[1;37;44m"
#define OUT_PURPLE "\e[1;37;45m"
#define OUT_CYAN   "\e[1;37;46m"
#define OUT_WHITE  "\e[1;30;47m"

#else // MINITRACES_COLORS == 0

#define CLR_RESET
#define CLR_BLACK
#define CLR_RED
#define CLR_GREEN
#define CLR_YELLOW
#define CLR_BLUE
#define CLR_PURPLE
#define CLR_CYAN
#define CLR_WHITE
#define BLD_BLACK
#define BLD_RED
#define BLD_GREEN
#define BLD_YELLOW
#define BLD_BLUE
#define BLD_PURPLE
#define BLD_CYAN
#define BLD_WHITE
#define OUT_BLACK
#define OUT_RED
#define OUT_GREEN
#define OUT_YELLOW
#define OUT_BLUE
#define OUT_PURPLE
#define OUT_CYAN
#define OUT_WHITE

#endif // MINITRACES_COLORS

/* ************************************************************************** */
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MINITRACES_H
