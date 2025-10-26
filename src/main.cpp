/*
    main.cpp


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

#include "misc1.h"
#ifdef _MSC_VER
    #include <new.h>
#endif
#include "foptman.h"
#include "apprun.h"
#include "soptions.h"
#include "termimpf.h"
#ifdef _WIN32
#include "winctxt.h"
#endif
#include "winmain.h"
#include "ffilecnt.h"
#include "warnoff.h"
#include <QApplication>
#include <QMessageBox>
#include "warnon.h"
#include <cstdlib>
#include <new>
#include <exception>
#include <utility>
#include <sstream>


#ifdef _WIN32
WinApiContext winApiContext;
#endif

// define a customized new handler which is called whenever a memory
// allocation attempt fails.
#ifdef _MSC_VER
#define RETURN_VALUE 1
int flexemu_new_handler(size_t /* [[maybe_unused]] size_t n */)
#else
#define RETURN_VALUE
static void flexemu_new_handler()
#endif
{
    if (QMessageBox::warning(nullptr, PROGRAMNAME " warning",
        "<b>Memory allocation failed.</b><br>\n"
        "Increasing available memory by e.g.<br>\n"
        "closing other applications may<br>\n"
        "prevent this.",
        QMessageBox::Retry | QMessageBox::Abort, QMessageBox::Retry) ==
            QMessageBox::Retry)
    {
        return RETURN_VALUE; // return means to retry memory allocation.
    }

    std::terminate();
}

int main(int argc, char *argv[])
{
    int return_code = EXIT_RESTART;
    bool isRestarted = false;

#ifdef _MSC_VER
    _set_new_handler(flexemu_new_handler);
#else
    std::set_new_handler(flexemu_new_handler);
#endif

    Q_INIT_RESOURCE(flexemu);

    FlexDisk::InitializeClass();

    while (return_code == EXIT_RESTART)
    {
        struct sOptions options;
        QApplication app(argc, argv);

        try
        {
            FlexemuOptions::InitOptions(options);
            FlexemuOptions::GetOptions(options);
            FlexemuOptions::GetCommandlineOptions(options, argc, argv);
            // write options but only if options file not already exists
            FlexemuOptions::WriteOptions(options, true);
            if (isRestarted)
            {
                options.startup_command = "";
                options.term_mode = false;
            }

            const auto termType =
                static_cast<TerminalType>(options.terminalType);
            auto termImpl = TerminalImplFactory::Create(termType, options);

            ApplicationRunner runner(options, std::move(termImpl));

            QApplication::setQuitOnLastWindowClosed(false);
            return_code = runner.startup(app);
            if (!return_code)
            {
                return_code = QApplication::exec();
            }

            isRestarted = true;
        }
        catch (std::exception &ex)
        {
            std::stringstream msg;

            msg << "<b>An exception has occured.</b><br>" << ex.what();

            QMessageBox::critical(nullptr, PROGRAMNAME " critical",
                QString::fromStdString(msg.str()),
                QMessageBox::Abort, QMessageBox::Abort);

            return_code = 1;
        }
    }

    return return_code;
}
