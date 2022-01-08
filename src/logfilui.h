/*
    logfilui.h


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


#ifndef LOGFILUI_INCLUDED
#define LOGFILUI_INCLUDED

#include "logfil_ui.h"
#include <QObject>

class QDialog;
class QAbstractButton;
class QLineEdit;
struct s_cpu_logfile;

class LogfileSettingsUi : public QObject, protected Ui_LogfileSettings
{
    Q_OBJECT

public:

    LogfileSettingsUi();
    ~LogfileSettingsUi() = default;
    void setupUi(QDialog &dialog);
    void SetData(const s_cpu_logfile &settings);
    s_cpu_logfile GetData() const;

protected:
    void OnClicked(QAbstractButton *button);
    void InitializeWidgets();
    void ConnectSignalsWithSlots();
    void OnSelectFile(QLineEdit &lineEdit);

private slots:
    void OnAccepted();
    void OnRejected();

private:
    QDialog *dialog;
};

#endif

