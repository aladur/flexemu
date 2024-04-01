/*
    fpcnvui.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2022-2024  W. Schwotzer

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


#include "misc1.h"
#include "warnoff.h"
#include "fpcnv_ui.h"
#include <QObject>
#include <QString>
#include "warnon.h"

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

