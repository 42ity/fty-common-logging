/*  =========================================================================
    fty_coverity_stub - Coverity stub

    Copyright (C) 2014 - 2023 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
 */

#pragma once

#ifdef COVERITY_STUB
    #pragma  ================================ COVERITY_STUB DEFINED ===============================

    // Stub for coverity memory leak detection in zeromq/czmq library
    // https://github.com/zeromq/czmq/blob/master/src/zstr.c
    inline void zstr_free(char **s) {
        if (s && (*s)) {
            free(*s);
            *s = NULL;
        }
    }

#endif
