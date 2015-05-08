/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - common.h                                        *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2014 Bobby Smiles                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef COMMON_H
#define COMMON_H

#include "../my_types.h"

/* macro for unused variable warning suppression */
#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

/* macro for inline keyword */
#ifdef _MSC_VER
#define inline __inline
#endif

/*
 * 2015.05.07 cxd4
 *
 * `bool' is not C programming, even if C99 granted the request.
 * Relevant C interpretation of Booleans is:  zero and nonzero.
 *
 * Past that, issues like allocation, packing or padding with Booleans are
 * more than controllable in correct C with or without type definitions,
 * especially in modern optimizing compilers.  The remaining issue is the
 * ability to say "true" and "false"--again external features irrelevant to
 * completeness of C implementation for the specific hardware architecture.
 */
#if !defined(FALSE) && !defined(TRUE)
enum {
    FALSE = 0,
    TRUE = 1
};
#endif
typedef int Boolean;

#endif
