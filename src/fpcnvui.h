/*
    fpcnvui.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2022-2025  W. Schwotzer

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


#ifndef FPCNVUI_INCLUDED
#define FPCNVUI_INCLUDED


#include "warnoff.h"
#ifdef USE_CMAKE
#include "ui_fpcnv.h"
#else
#include "fpcnv_ui.h"
#endif
#include <QObject>
#include "warnon.h"

class QString;
class QDialog;

class FlexplorerConvertUi : public QObject, protected Ui_FlexplorerConvert
{
    Q_OBJECT

public:

    FlexplorerConvertUi();
    ~FlexplorerConvertUi() override = default;
    void setupUi(QDialog &dialog);
    void TransferDataToDialog(const QString &title,
                              const QString &filename,
                              const QString &whatToDo,
                              bool isConvertTextFile,
                              bool isAskUser);
    bool GetConvert() const;
    bool GetAskUser() const;

    QDialog *dialog{nullptr};
};

#endif

