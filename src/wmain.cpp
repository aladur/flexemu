/*
    wmain.cpp


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


#include "wmain.h"
#ifdef _WIN32
#include "cvtwchar.h"
#include <string>
#include <vector>

int wmain(int argc, wchar_t *wargv[])
{
    std::vector<std::string> args;
    std::vector<char *> argv;

    // On Windows use wmain() using wide characters to support unicode file
    // and directory path parameters. Convert them to UTF-8 strings for
    // portability.
    args.reserve(argc);
    argv.reserve(argc);
    for (int index = 0; index < argc; ++index)
    {
        args.push_back(ConvertToUtf8String(wargv[index]));
        argv.push_back(&args[index].at(0));
    }

    return flx::main(argc, argv.data());
}
#else
int main(int argc, char *argv[])
{
    return flx::main(argc, argv);
}
#endif
