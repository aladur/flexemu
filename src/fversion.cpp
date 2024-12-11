/*
    fversion.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024  W. Schwotzer

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


#ifdef UNIX
#include "config.h"
#endif
#include "fversion.h"
#include <string>
#ifdef QT_CORE_LIB
#include <QObject>
#endif
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"
#if defined(UNIX) && defined(ADD_JSONCPP_VERSION)
#include <json/version.h>
#endif
#if defined(UNIX) && defined(HAVE_NCURSES_H) && defined(ADD_NCURSES_VERSION)
#include <ncurses.h>
#endif


Versions_t FlexemuVersions::CreateVersions()
{
    Versions_t versions;
    std::string version;
    std::string compiler;

#if defined(__GNUC__)
#if defined(__clang__)
    compiler = "clang";
    version = fmt::format("{}.{}.{}",
            __clang_major__, __clang_minor__, __clang_patchlevel__);
    versions.emplace(compiler, version);
#else
    compiler = "gcc";
    version = fmt::format("{}.{}.{}",
            __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    versions.emplace(compiler, version);
#endif
#else
#if defined(_MSC_VER)
    compiler = "MSVC";
    version = fmt::format("{}.{}.{}",
            _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000);
    versions.emplace(compiler, version);
#endif
#endif

#ifdef QT_CORE_LIB
    version = fmt::format("{}.{}.{}",
            QT_VERSION_MAJOR, QT_VERSION_MINOR, QT_VERSION_PATCH);
    versions.emplace("Qt", version);
#endif

    version = fmt::format("{}.{}.{}",
            FMT_VERSION / 10000, (FMT_VERSION / 100) % 100, FMT_VERSION % 100);
    versions.emplace("fmtlib", version);

#if defined(UNIX) && defined(ADD_JSONCPP_VERSION)
    versions.emplace("jsoncpp", JSONCPP_VERSION_STRING);
#endif

#if defined(UNIX) && defined(HAVE_NCURSES_H) && defined(ADD_NCURSES_VERSION)
    version = fmt::format("{}.{}", NCURSES_VERSION, NCURSES_VERSION_PATCH);
    versions.emplace("ncurses", version);
#endif

    return versions;
}

Versions_t FlexemuVersions::GetVersions()
{
    static Versions_t versions;

    if (versions.empty())
    {
        versions = CreateVersions();
    }

    return versions;
}
