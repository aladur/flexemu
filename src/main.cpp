/*
    main.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2020  W. Schwotzer

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

#include "misc1.h"
#include <new>
#include <sstream>
#ifdef _MSC_VER
    #include <new.h>
#endif

#include "foptman.h"
#include "flexerr.h"
#include "apprun.h"
#include "sguiopts.h"
#include "winctxt.h"


#ifdef _WIN32
WinApiContext winApiContext;
#endif

// define an exception handler when new fails

#ifdef _MSC_VER
// with MSC it's possible to retry memory allocation
int std_new_handler(size_t /* [[maybe_unused]] size_t n */)
{
    int result;

    result = MessageBox(nullptr, gMemoryAllocationErrorString,
                        PROGRAMNAME " error", MB_RETRYCANCEL | MB_ICONWARNING);

    if (result == IDRETRY)
    {
        return 1;    // retry once more
    }

    throw std::bad_alloc();
}
#else
void std_new_handler()
{
    throw std::bad_alloc();
}
#endif

int main(int argc, char *argv[])
{
    struct sOptions options;
    struct sGuiOptions guiOptions;
    FlexOptionManager optionMan;
    int return_code = 0;

#ifdef _MSC_VER
    set_new_handler(std_new_handler);
#else
    std::set_new_handler(std_new_handler);
#endif

    try
    {
        optionMan.InitOptions(&guiOptions, &options, argc, argv);
        optionMan.GetOptions(&guiOptions, &options);
        optionMan.GetEnvironmentOptions(&guiOptions, &options);
        optionMan.GetCommandlineOptions(&guiOptions, &options, argc, argv);
        // write options but only if options file not already exists
        optionMan.WriteOptions(&guiOptions, &options, true);

        ApplicationRunner runner(guiOptions, options);

        return_code = runner.run();
    }
#ifdef _WIN32
    catch (std::bad_alloc UNUSED(&e))
    {
        MessageBox(nullptr, gMemoryAllocationErrorString,
                   PROGRAMNAME " error", MB_OK | MB_ICONERROR);
        return_code = 1;
    }
#endif
    catch (std::exception &ex)
    {
        std::stringstream msg;

        msg << PROGRAMNAME << ": An error has occured: " << ex.what();

#ifdef _WIN32
        MessageBox(nullptr, msg.str().c_str(),
                   PROGRAMNAME " error", MB_OK | MB_ICONERROR);
#else
        fprintf(stderr, "%s\n", msg.str().c_str());
#endif
        return_code = 1;
    }

    return return_code;
}

#ifdef _WIN32
void scanCmdLine(LPSTR lpCmdLine, int *argc, char **argv, size_t max_count)
{
    *argc = 1;
    *(argv + 0) = "flexemu";
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
    char *argv[50];

    winApiContext.hPrevInstance = hPrevInstance;
    winApiContext.hInstance = hInstance;
    winApiContext.nCmdShow = nCmdShow;

    scanCmdLine(lpCmdLine, &argc, (char **)argv, sizeof(argv)/sizeof(argv[0]));

    return main(argc, argv);
}

#endif // #ifdef _WIN32

