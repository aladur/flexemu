/*
    fpmain.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2024  W. Schwotzer

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
#include "fpwin.h"
#include "warnoff.h"
#include <QApplication>
#include <QResource>
#include <QMessageBox>
#include <QTimer>
#include "warnon.h"
#include "winmain.h"
#include "winctxt.h"
#include "sfpopts.h"
#include "fpoptman.h"
#include "ffilecnt.h"
#include <iostream>


#ifdef _WIN32
WinApiContext winApiContext;
#endif

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(fpmain_qrc_cpp);
    sFPOptions options;

    FlexDisk::InitializeClass();
    QApplication app(argc, argv);
    FlexplorerOptions::InitOptions(options);
    FlexplorerOptions::ReadOptions(options);
    FLEXplorer window(options);
    int return_code = EXIT_FAILURE;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg{argv[i]};

        if (arg.compare("-V") == 0)
        {
            flx::print_versions(std::cout, "FLEXplorer");
            exit(EXIT_SUCCESS);
        }
    }

    const auto icon = QIcon(":/resource/flexplorer.png");
    QApplication::setWindowIcon(icon);

    try
    {
        ProcessArgumentsFtor functor(window, argc, argv);

        window.show();

        QTimer::singleShot(10, functor);

        return_code = QApplication::exec();
    }
    catch (std::exception &ex)
    {
        QMessageBox::critical(&window, QObject::tr("FLEXplorer Error"),
                QObject::tr("An unrecoverable error has occured:\n") +
                ex.what());
    }
    FlexplorerOptions::WriteOptions(options);

    return return_code;
}

