/*
    warnoff.h


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

// This header can be used to prevent warnings in 3rdParty headers.
// Example:
// #include "warnoff.h"
// #include "3rd_party_header.h"
// #include "warnon.h"

// Disabled warnings in GCC:
// -Wshadow: Warn whenever a local variable or type declaration shadows
//     another variable, parameter, type, class member, or whenever a built-in
//     function is shadowed.
// -Wuseless-cast: Warn when an expression is casted to its own type.

// Disabled warnings in clang:
// -Wshadow: declaration shadows a <specifier>.

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif

// Disabled warnings in MSVC:
// C4127: conditional expression is constant.
// C4251: 'identifier' : class 'type' needs to have dll-interface to be used
//        by clients of class 'type2'.

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4127 4244 4251 4267 4702 )
#endif
