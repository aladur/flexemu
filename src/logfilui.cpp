/*
    logfilui.cpp


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


#include "logfilui.h"
#include "efslctle.h"
#include "scpulog.h"
#include "bui.h"
#include <stdexcept>
#include <cassert>
#include "warnoff.h"
#include <QAbstractButton>
#include <QLineEdit>
#include <QFileInfo>
#include <QFileDialog>
#include "warnon.h"

LogfileSettingsUi::LogfileSettingsUi() :
    Ui_LogfileSettings(), dialog(nullptr)
{
}

void LogfileSettingsUi::setupUi(QDialog &p_dialog)
{
    dialog = &p_dialog;
    Ui_LogfileSettings::setupUi(dialog);

    assert(e_minAddress != nullptr);
    assert(e_maxAddress != nullptr);
    assert(e_startAddress != nullptr);
    assert(e_stopAddress != nullptr);
    assert(c_logCycleCount != nullptr);
    assert(e_logFilename != nullptr);

    InitializeWidgets();
    ConnectSignalsWithSlots();
}

void LogfileSettingsUi::InitializeWidgets()
{
    ::InstallSelectionEventFilter(*e_minAddress, this);
    ::InstallSelectionEventFilter(*e_maxAddress, this);
    ::InstallSelectionEventFilter(*e_startAddress, this);
    ::InstallSelectionEventFilter(*e_stopAddress, this);
    ::InstallSelectionEventFilter(*e_logFilename, this);

    QString inputMask(">hhhh");
    e_minAddress->setInputMask(inputMask);
    e_maxAddress->setInputMask(inputMask);
    e_startAddress->setInputMask(inputMask);
    e_stopAddress->setInputMask(inputMask);
}

void LogfileSettingsUi::SetData(const s_cpu_logfile &settings)
{
    if (dialog == nullptr)
    {
        throw std::logic_error("setupUi(dialog) has to be called before.");
    }

    ::SetData(settings.minAddr, *e_minAddress);
    ::SetData(settings.maxAddr, *e_maxAddress);
    ::SetData(settings.startAddr, *e_startAddress);
    ::SetData(settings.stopAddr, *e_stopAddress);

    c_logCycleCount->setChecked(settings.logCycleCount);
    e_logFilename->setText(settings.logFileName.c_str());
}

s_cpu_logfile LogfileSettingsUi::GetData() const
{
    s_cpu_logfile settings;

    settings.minAddr = ::GetData<uint>(*e_minAddress);
    settings.maxAddr = ::GetData<uint>(*e_maxAddress);
    settings.startAddr = ::GetData<uint>(*e_startAddress);
    settings.stopAddr = ::GetData<uint>(*e_stopAddress);

    settings.logCycleCount = c_logCycleCount->isChecked();
    settings.logFileName = e_logFilename->text().toUtf8().data();

    return settings;
}

void LogfileSettingsUi::ConnectSignalsWithSlots()
{
    QObject::connect(c_buttonBox, &QDialogButtonBox::accepted,
        this, &LogfileSettingsUi::OnAccepted);
    QObject::connect(c_buttonBox, &QDialogButtonBox::rejected,
        this, &LogfileSettingsUi::OnRejected);
    QObject::connect(c_buttonBox, &QDialogButtonBox::clicked,
        this, &LogfileSettingsUi::OnClicked);
    QObject::connect(b_logFilename, &QAbstractButton::clicked,
        [&](){ OnSelectFile(*e_logFilename); });
}

void LogfileSettingsUi::OnAccepted()
{
    dialog->done(QDialog::Accepted);
}

void LogfileSettingsUi::OnRejected()
{
    dialog->done(QDialog::Rejected);
}

void LogfileSettingsUi::OnClicked(QAbstractButton *button)
{
    assert(button != nullptr);

    if (button->text() == "Reset")
    {
        e_minAddress->clear();
        e_maxAddress->clear();
        e_startAddress->clear();
        e_stopAddress->clear();
        c_logCycleCount->setChecked(false);
        e_logFilename->clear();
    }
}

void LogfileSettingsUi::OnSelectFile(QLineEdit &lineEdit)
{
    auto path = QDir::currentPath();
    auto fileInfo = QFileInfo(lineEdit.text());

    if (fileInfo.isAbsolute() && QFile::exists(lineEdit.text()))
    {
        path = lineEdit.text();
    }

    path = QFileDialog::getSaveFileName(
           dialog, tr("Select a Logfile"), path,
           tr("Logfiles (*.log);;All files (*.*)"));

    if  (!path.isEmpty())
    {
        lineEdit.setText(path);
    }
}

