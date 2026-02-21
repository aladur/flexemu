/*
    fpcnvui.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2022-2026  W. Schwotzer

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


#include "fpcnvui.h"
#include "warnoff.h"
#include <ui_fpcnv.h>
#include <QString>
#include <QDialog>
#include "warnon.h"
#include <stdexcept>

FlexplorerConvertUi::FlexplorerConvertUi() :
    Ui_FlexplorerConvert()
{
}

void FlexplorerConvertUi::setupUi(QDialog &p_dialog)
{
    dialog = &p_dialog;
    Ui_FlexplorerConvert::setupUi(dialog);
}

void FlexplorerConvertUi::TransferDataToDialog(
        const QString &title, const QString &filename,
        const QString &whatToDo, bool isConvert, bool isAskUser)
{
    if (dialog == nullptr)
    {
        throw std::logic_error("setupUi(dialog) has to be called before.");
    }

    dialog->setWindowTitle(title);
    l_file->setText(QString("%1 is a text file").arg(filename));
    c_convert->setText(whatToDo);
    c_convert->setChecked(isConvert);
    c_doNotAsk->setChecked(!isAskUser);
}

bool FlexplorerConvertUi::GetConvert() const
{
    return c_convert->isChecked();
}

bool FlexplorerConvertUi::GetAskUser() const
{
    return !c_doNotAsk->isChecked();
}

