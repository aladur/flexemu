/*
    winmain.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2024  W. Schwotzer

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

#ifdef _WIN32
#include "misc1.h"
#include "winctxt.h"
#include "winmain.h"


void scanCmdLine(LPSTR lpCmdLine, int* argc, char** argv, size_t max_count)
{
    static char argv0[8]{ "flexemu" };

    *argc = 1;
    *(argv + 0) = argv0;
    bool is_double_quote = false;

    while (*lpCmdLine && (*lpCmdLine == ' ' || *lpCmdLine == '\t'))
    {
        lpCmdLine++;
    }

    while (*lpCmdLine && *argc < (int)max_count)
    {
        if (!is_double_quote && *lpCmdLine == '"')
        {
            lpCmdLine++;
            is_double_quote = true;
        }

        *(argv + *argc) = lpCmdLine;

        while (*lpCmdLine &&
            ((is_double_quote && *lpCmdLine != '"') ||
                (!is_double_quote && *lpCmdLine != ' ' &&
                    *lpCmdLine != '\t' && *lpCmdLine != '"')))
        {
            lpCmdLine++;
        }

        is_double_quote = (!is_double_quote && *lpCmdLine == '"');

        if (*lpCmdLine)
        {
            *(lpCmdLine++) = '\0';
        }

        while (*lpCmdLine && (*lpCmdLine == ' ' || *lpCmdLine == '\t'))
        {
            lpCmdLine++;
        }

        (*argc)++;
    }
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    int argc;
    char* argv[50];

    winApiContext.hPrevInstance = hPrevInstance;
    winApiContext.hInstance = hInstance;
    winApiContext.nCmdShow = nCmdShow;

    scanCmdLine(lpCmdLine, &argc, (char**)argv, sizeof(argv) / sizeof(argv[0]));

    return main(argc, argv);
}

#endif // #ifdef _WIN32

