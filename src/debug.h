/*
    debug.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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

#include <stdio.h>

#define LOG_TEMPLATE(print_line)\
    {\
        FILE *logfp;\
        logfp = fopen(DEBUG_FILE, "a");\
        if (logfp != nullptr) {\
            print_line;\
            fclose(logfp);\
        }\
    }

#define LOG(format)              LOG_TEMPLATE(fprintf(logfp, format))
#define LOG_X(format, a)         LOG_TEMPLATE(fprintf(logfp, format, a))
#define LOG_XX(format, a, b)     LOG_TEMPLATE(fprintf(logfp, format, a, b))
#define LOG_XXX(format, a, b, c) LOG_TEMPLATE(fprintf(logfp, format, a, b, c))


#endif // DEBUG_INCLUDED

