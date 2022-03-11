/*
    fpmain.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2022  W. Schwotzer

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


#include "fpwin.h"
#include "warnoff.h"
#include <QApplication>
#include <QResource>
#include <QMessageBox>
#include "warnon.h"
#include "winmain.h"
#include "winctxt.h"
#include "winmain.h"


#ifdef _WIN32
WinApiContext winApiContext;
#endif

static void LoadFiles(int argc, char *argv[], FLEXplorer &window)
{
    for (int i = 1; i < argc; ++i)
    {
        bool isLast = (i == argc - 1);
        if (!window.OpenContainerForPath(QString(argv[i]), isLast))
        {
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(fpmain_qrc_cpp);

    QApplication app(argc, argv);
    FLEXplorer window;
    int return_code = EXIT_FAILURE;

    const auto icon = QIcon(":/resource/flexplorer.png");
    app.setWindowIcon(icon);

    try
    {
        window.show();

        LoadFiles(argc, argv, window);
        app.processEvents();

        return_code = app.exec();
    }
    catch (std::exception &ex)
    {
        QMessageBox::critical(&window, QObject::tr("FLEXplorer Error"),
                QObject::tr("An unrecoverable error has occured:\n") +
                ex.what());
    }

    return return_code;
}

