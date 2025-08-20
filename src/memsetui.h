/*
    memsetui.h


    Flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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


#ifndef MEMORYSETTINGSUI_INCLUDE
#define MEMORYSETTINGSUI_INCLUDE

#include "memwin.h"
#include "warnoff.h"
#ifdef USE_CMAKE
#include "ui_memset.h"
#else
#include "memset_ui.h"
#endif
#include <QObject>
#include "warnon.h"

class QDialog;
class QLineEdit;
class QAbstractButton;

class MemorySettingsUi : public QObject, public Ui_MemorySettings
{
    Q_OBJECT

public:
    MemorySettingsUi();
    ~MemorySettingsUi() override = default;
    void setupUi(QDialog &dialog);
    void SetData(const MemoryWindow::Config_t &config) const;
    void GetData(MemoryWindow::Config_t &config) const;

protected:
    void OnClicked(QAbstractButton *button);
    void OnStyleChanged(int index);
    void InitializeWidgets();
    void ConnectSignalsWithSlots();

private slots:
    void OnAccepted();
    void OnRejected();

private:
    bool Validate();

    QDialog *dialog{nullptr};
};

#endif

