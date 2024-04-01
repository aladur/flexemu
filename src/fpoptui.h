/*
    fpoptui.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2020-2024  W. Schwotzer

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


#ifndef FPOPTUI_INCLUDED
#define FPOPTUI_INCLUDED


#include "misc1.h"
#include "efiletim.h"
#include "warnoff.h"
#include "fpopt_ui.h"
#include <QObject>
#include <QString>
#include "warnon.h"
#include "sfpopts.h"

class QLineEdit;

class FlexplorerOptionsUi : public QObject, protected Ui_FlexplorerOptions
{
    Q_OBJECT

public:

    FlexplorerOptionsUi();
    ~FlexplorerOptionsUi() override = default;
    void setupUi(QDialog &dialog);
    void TransferDataToDialog(const struct sFPOptions &options);
    void TransferDataFromDialog(struct sFPOptions &options);

private slots:
    void OnSelectBootSectorFile();
    void OnAccepted();
    void OnRejected();

private:
    void InitializeWidgets();
    void ConnectSignalsWithSlots();
    bool Validate();

    QDialog *dialog{nullptr};
};

#endif

