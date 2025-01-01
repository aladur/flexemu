/*
    debug.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

#include <fstream>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"

#define LOG_TEMPLATE(print_line)\
    {\
        std::ofstream log_ofs(DEBUG_FILE, std::ios::out | std::ios::app);\
        if (log_ofs.is_open()) {\
            log_ofs << (print_line);\
            log_ofs.close();\
        }\
    }

#define LOG(fmt_str) LOG_TEMPLATE(fmt::format(fmt_str))
#define LOG_X(fmt_str, a) LOG_TEMPLATE(fmt::format(fmt_str, a))
#define LOG_XX(fmt_str, a, b) LOG_TEMPLATE(fmt::format(fmt_str, a, b))
#define LOG_XXX(fmt_str, a, b, c) LOG_TEMPLATE(fmt::format(fmt_str, a, b, c))


#endif // DEBUG_INCLUDED

