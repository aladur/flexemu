/*

    typedefs.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

    This file is based on usim-0.91 which is
    Copyright (C) 1994 by R. B. Bellis
*/


#ifndef __typedefs_h__
#define __typedefs_h__

#ifdef _MSC_VER
    #include "confignt.h"
#else
    #include "config.h"
#endif

#if HAVE_INTTYPES_H
    #include <inttypes.h>
#else
    #if HAVE_STDINT_H
        #include <stdint.h>
    #endif
#endif

#if defined(HAVE_INTTYPES_H) || defined(HAVE_STDINT_H)
    typedef uint8_t         Byte;
    typedef int8_t          SByte;
    typedef uint16_t        Word;
    typedef int16_t         SWord;
    typedef uint32_t        DWord;
    typedef int32_t         SDWord;
    typedef uint64_t        QWord;
    typedef int64_t         SQWord;
    #define PRlu64 "%lu"
    #define PRld64 "%ld"
#else
    typedef unsigned char       Byte;
    typedef signed   char       SByte;

    /* 16 bit data types */
    #if (SIZEOF_SHORT == 2)
        typedef unsigned short    Word;
        typedef short         SWord;
    #else
        typedef unsigned int      Word;
        typedef int           SWord;
    #endif

    /* 32 bit data types */
    #if (SIZEOF_INT == 4)
        typedef unsigned int      DWord;
        typedef int           SDWord;
    #else
        #if (SIZEOF_LONG == 4)
            typedef unsigned long   DWord;
            typedef long        SDWord;
        #else
            #error No data type of size 4 found
        #endif
    #endif

    /* 64 bit data types */
    #ifdef _MSC_VER
        typedef unsigned __int64 QWord;
        typedef __int64 SQWord;
        #define PRlu64 "%llu"
        #define PRld64 "%lld"
    #else
        #if (SIZEOF_LONG == 8)
            typedef unsigned long     QWord;
            typedef long          SQWord;
            #define PRlu64 "%lu"
            #define PRld64 "%ld"
        #else
            #if (SIZEOF_LONG_LONG == 8)
                typedef unsigned long long  QWord;
                typedef long long       SQWord;
                #define PRlu64 "%llu"
                #define PRld64 "%lld"
            #else
                /* a class with opterator overloading could help here */
                #error No data type of size 8 found
            #endif
        #endif
    #endif /* #ifdef _MSC_VER */
#endif

#endif /* __typedefs_h__ */

