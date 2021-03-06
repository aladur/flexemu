/*
    fsetup.cpp


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
#include "foptman.h"
#include "fsetup_ui.h"
#include "fsetupui.h"
#include "soptions.h"
#include <vector>
#include <memory>
#include <QApplication>
#include <QResource>
#include <QDialog>
#include "winctxt.h"
#include "winmain.h"


#ifdef _WIN32
WinApiContext winApiContext;
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FlexOptionManager optionMan;
    QDialog dialog;
    struct sOptions options;

    Q_INIT_RESOURCE(fsetup_qrc_cpp);

    optionMan.InitOptions(options);
    optionMan.GetOptions(options);
    optionMan.GetEnvironmentOptions(options);

    FlexemuOptionsUi ui;
    ui.setupUi(&dialog);
    ui.TransferDataToDialog(options);

    dialog.resize({0, 0});
    dialog.setModal(true);
    dialog.setSizeGripEnabled(true);
    auto result = dialog.exec();

    if (result == QDialog::Accepted)
    {
        ui.TransferDataFromDialog(options);
        optionMan.WriteOptions(options);
    }

    return 0;
}

