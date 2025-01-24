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
    #pragma  "==== COVERITY_STUB DEFINED ===="

    // Stub for coverity analysis (memory leak defect)
    // https://github.com/zeromq/czmq/blob/master/src/zstr.c
    extern "C" void zstr_free(char **s);
    inline void zstr_free(char **s) {
        if (s && (*s)) {
            free(*s);
            *s = NULL;
        }
    }

    // Stub for coverity analysis (memory leak defect)
    // https://github.com/42ity/fty-common-mlm/blob/release/IPM-2.8.2/include/fty_common_mlm_guards.h
    class ZstrGuard
    {
    public:
        ZstrGuard() : ptr_(nullptr) {}
        explicit ZstrGuard(char* ptr) : ptr_(ptr) {}
        ZstrGuard(const ZstrGuard&) = delete;
        ~ZstrGuard() { destruct(); }
        ZstrGuard* operator=(char* ptr) { destruct(); ptr_ = ptr; return ptr_; }
        operator char*() { return ptr_; }
        char* get() { return ptr_; }
    private:
        void destruct() { ztr_free(&ptr_); }
        char* ptr_{nullptr};
    };

#endif //COVERITY_STUB
