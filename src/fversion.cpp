/*
    fversion.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2026  W. Schwotzer

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


#include "config.h"
#include "fversion.h"
#include "free.h"
#include <fmt/base.h>
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
#include <string>
#include <vector>


ItemPairList_t FlexemuVersions::CreateVersions()
{
    ItemPairList_t versions;
    std::string version;
    std::string compiler;

#if defined(__GNUC__)
#if defined(__clang__)
    compiler = "clang";
    version = fmt::format("{}.{}.{}",
            __clang_major__, __clang_minor__, __clang_patchlevel__);
    versions.emplace_back(compiler, std::vector(1U, version));
#else
    compiler = "gcc";
    version = fmt::format("{}.{}.{}",
            __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    versions.emplace_back(compiler, std::vector(1U, version));
#endif
#else
#if defined(_MSC_VER)
    compiler = "MSVC";
    version = fmt::format("{}.{}.{}",
            _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000);
    versions.emplace_back(compiler, std::vector(1U, version));
#endif
#endif

#ifdef QT_CORE_LIB
    version = fmt::format("{}.{}.{}",
            QT_VERSION_MAJOR, QT_VERSION_MINOR, QT_VERSION_PATCH);
    versions.emplace_back("Qt", std::vector(1U, version));
#endif

    version = fmt::format("{}.{}.{}",
            FMT_VERSION / 10000, (FMT_VERSION / 100) % 100, FMT_VERSION % 100);
    versions.emplace_back("fmtlib", std::vector(1U, version));

#if defined(UNIX) && defined(ADD_JSONCPP_VERSION)
    versions.emplace_back("jsoncpp",
            std::vector(1U, std::string(JSONCPP_VERSION_STRING)));
#endif

#if defined(UNIX) && defined(HAVE_NCURSES_H) && defined(ADD_NCURSES_VERSION)
    version = fmt::format("{}.{}", NCURSES_VERSION, NCURSES_VERSION_PATCH);
    versions.emplace_back("ncurses", std::vector(1U, version));
#endif

#ifdef CMAKE_VERSION
    version = CMAKE_VERSION;
    versions.emplace_back("cmake", std::vector(1U, version));
#endif

    return versions;
}

ItemPairList_t FlexemuVersions::GetVersions()
{
    static ItemPairList_t versions;

    if (versions.empty())
    {
        versions = CreateVersions();
    }

    return versions;
}
