/*
    fsetup.cpp


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
#include "foptman.h"
#include "ui_fsetup.h"
#include "fsetupui.h"
#include "sguiopts.h"
#include <vector>
#include <memory>
#include <QApplication>
#include <QResource>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FlexOptionManager optionMan;
    QDialog dialog;
    struct sGuiOptions guiOptions;
    struct sOptions options;

    Q_INIT_RESOURCE(fsetup);

    optionMan.InitOptions(&guiOptions, &options, argc, argv);
    optionMan.GetOptions(&guiOptions, &options);
    optionMan.GetEnvironmentOptions(&guiOptions, &options);

    FlexemuOptionsUi ui;
    ui.setupUi(&dialog);
    ui.TransferDataToDialog(guiOptions, options);

    dialog.resize({0, 0});
    dialog.setModal(true);
    dialog.setSizeGripEnabled(true);
    auto result = dialog.exec();

    if (result == QDialog::Accepted)
    {
        ui.TransferDataFromDialog(guiOptions, options);
        optionMan.WriteOptions(&guiOptions, &options);
    }

    return 0;
}

