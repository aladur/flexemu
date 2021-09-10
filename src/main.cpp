/*
    main.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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
#include "soptions.h"
#include "winctxt.h"
#include "winmain.h"
#include "cvtwchar.h"
#include <QApplication>


#ifdef _WIN32
WinApiContext winApiContext;
#endif


// define an exception handler when new fails

#ifdef _MSC_VER
// with MSC it's possible to retry memory allocation
int std_new_handler(size_t /* [[maybe_unused]] size_t n */)
{
    int result;

    result = MessageBox(
        nullptr,
        ConvertToUtf16String(gMemoryAllocationErrorString).c_str(),
        ConvertToUtf16String(PROGRAMNAME " error").c_str(),
        MB_RETRYCANCEL | MB_ICONWARNING);

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
    int return_code = EXIT_RESTART;
    bool isRestarted = false;

#ifdef _MSC_VER
    set_new_handler(std_new_handler);
#else
    std::set_new_handler(std_new_handler);
#endif

    Q_INIT_RESOURCE(flexemu_qrc_cpp);


    while (return_code == EXIT_RESTART)
    {
        struct sOptions options;
        FlexOptionManager optionMan;

        try
        {
            optionMan.InitOptions(options);
            optionMan.GetOptions(options);
            optionMan.GetEnvironmentOptions(options);
            optionMan.GetCommandlineOptions(options, argc, argv);
            // write options but only if options file not already exists
            optionMan.WriteOptions(options, true);
            if (isRestarted)
            {
                options.startup_command = "";
                options.term_mode = false;
            }

            QApplication app(argc, argv);
            ApplicationRunner runner(options);

            if (!(return_code = runner.startup()))
            {
                return_code = app.exec();
            }
            isRestarted = true;
        }

#ifdef _WIN32
        catch (std::bad_alloc UNUSED(&e))
        {
            MessageBox(
                nullptr,
                ConvertToUtf16String(gMemoryAllocationErrorString).c_str(),
                ConvertToUtf16String(PROGRAMNAME " error").c_str(),
                 MB_OK | MB_ICONERROR);
            return_code = 1;
            break;
        }
#endif
        catch (std::exception &ex)
        {
            std::stringstream msg;

            msg << PROGRAMNAME << ": An error has occured: " << ex.what();

#ifdef _WIN32
            MessageBox(
                nullptr,
                ConvertToUtf16String(msg.str()).c_str(),
                ConvertToUtf16String(PROGRAMNAME " error").c_str(),
                MB_OK | MB_ICONERROR);
#else
            fprintf(stderr, "%s\n", msg.str().c_str());
#endif
            return_code = 1;
            break;
        }
    };

    return return_code;
}
