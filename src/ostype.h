/*
    ostype.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025-2026  W. Schwotzer

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



#ifndef OSTYPE_INCLUDED
#define OSTYPE_INCLUDED

/* Keep macro to be used within string literal. */
/* NOLINTBEGIN(cppcoreguidelines-macro-usage) */
#ifdef __linux__
#define OSTYPE "Linux"
#else
#ifdef __FreeBSD__
#define OSTYPE "FreeBSD"
#else
#ifdef __OpenBSD__
#define OSTYPE "OpenBSD"
#else
#ifdef __NetBSD__
#define OSTYPE "NetBSD"
#else
#ifdef __BSD__
#define OSTYPE "BSD"
#else
#ifdef _WIN32
#define OSTYPE "Windows"
#else
#ifdef __APPLE__
#define OSTYPE "MacOS"
#else
#define OSTYPE "unknown"
#endif
#endif
#endif
#endif
#endif
#endif
#endif
/* NOLINTEND(cppcoreguidelines-macro-usage) */

#endif
