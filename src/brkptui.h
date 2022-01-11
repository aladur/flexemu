/*
    brkptui.h


    Flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2022  W. Schwotzer

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


#ifndef BRKPTUI_INCLUDE
#define BRKPTUI_INCLUDE

#include "warnoff.h"
#include "brkpt_ui.h"
#include <QObject>
#include "warnon.h"

class QDialog;
class QLineEdit;
class QAbstractButton;

using BPArray = std::array<unsigned int, 2>;

class BreakpointSettingsUi : public QObject, protected Ui_BreakpointSettings
{
    Q_OBJECT

public:
    BreakpointSettingsUi();
    ~BreakpointSettingsUi() = default;
    void setupUi(QDialog &dialog);
    void SetData(const BPArray &breakpoints);
    BPArray GetData() const;

protected:
    void OnClicked(QAbstractButton *button);
    void InitializeWidgets();
    void ConnectSignalsWithSlots();

private slots:
    void OnAccepted();
    void OnRejected();

private:
    QDialog *dialog;
};

#endif

