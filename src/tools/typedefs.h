/*
    typedefs.h


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


#ifndef TYPEDEFS_INCLUDED
#define TYPEDEFS_INCLUDED

#include "config.h"
#if HAVE_INTTYPES_H
    #include <inttypes.h>
#else
    #if HAVE_STDINT_H
        #include <stdint.h>
    #endif
#endif

#if defined(HAVE_INTTYPES_H) || defined(HAVE_STDINT_H)
    typedef uint8_t Byte;
    typedef int8_t SByte;
    typedef uint16_t Word;
    typedef int16_t SWord;
    typedef uint32_t DWord;
    typedef int32_t SDWord;
#else
    typedef unsigned char Byte;
    typedef signed   char SByte;

    /* 16 bit data types */
    #if (SIZEOF_SHORT == 2)
        typedef unsigned short Word;
        typedef short SWord;
    #else
        typedef unsigned int Word;
        typedef int SWord;
    #endif

    /* 32 bit data types */
    #if (SIZEOF_INT == 4)
        typedef unsigned int DWord;
        typedef int SDWord;
    #else
        #if (SIZEOF_LONG == 4)
            typedef unsigned long DWord;
            typedef long SDWord;
        #else
            #error No data type of size 4 found
        #endif
    #endif

#endif

#endif /* TYPEDEFS_INCLUDED */

