/*
    logfilui.cpp


    Flexemu, an MC6809 emulator running FLEX
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

Mc6809LoggerConfigUi::Mc6809LoggerConfigUi() :
    Ui_LogfileSettings()
{
}

void Mc6809LoggerConfigUi::setupUi(QDialog &p_dialog)
{
    dialog = &p_dialog;
    Ui_LogfileSettings::setupUi(dialog);

    regCheckBoxes = {
        c_reg_cc, c_reg_a, c_reg_b, c_reg_dp,
        c_reg_x, c_reg_y, c_reg_u, c_reg_s
    };

    assert(e_minAddress != nullptr);
    assert(e_maxAddress != nullptr);
    assert(e_startAddress != nullptr);
    assert(e_stopAddress != nullptr);
    assert(c_logCycleCount != nullptr);
    assert(e_logFilename != nullptr);

    for (const auto &regCheckBox : regCheckBoxes)
    {
        assert(regCheckBox != nullptr);
    }

    InitializeWidgets();
    ConnectSignalsWithSlots();
}

void Mc6809LoggerConfigUi::InitializeWidgets()
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

void Mc6809LoggerConfigUi::SetData(const Mc6809LoggerConfig &loggerConfig)
{
    if (dialog == nullptr)
    {
        throw std::logic_error("setupUi(dialog) has to be called before.");
    }

    ::SetData(loggerConfig.minAddr, *e_minAddress);
    ::SetData(loggerConfig.maxAddr, *e_maxAddress);
    ::SetData(loggerConfig.startAddr, *e_startAddress);
    ::SetData(loggerConfig.stopAddr, *e_stopAddress);

    c_logCycleCount->setChecked(loggerConfig.logCycleCount);
    e_logFilename->setText(loggerConfig.logFileName.c_str());

    auto logRegister = LogRegister::CC;
    for (auto *regCheckBox : regCheckBoxes)
    {
        bool isChecked =
            (loggerConfig.logRegisters & logRegister) != LogRegister::NONE;
        regCheckBox->setChecked(isChecked);
        logRegister <<= 1;
    }
}

Mc6809LoggerConfig Mc6809LoggerConfigUi::GetData() const
{
    Mc6809LoggerConfig loggerConfig;

    loggerConfig.minAddr = ::GetData<BOptionalWord>(*e_minAddress);
    loggerConfig.maxAddr = ::GetData<BOptionalWord>(*e_maxAddress);
    loggerConfig.startAddr = ::GetData<BOptionalWord>(*e_startAddress);
    loggerConfig.stopAddr = ::GetData<BOptionalWord>(*e_stopAddress);

    loggerConfig.logCycleCount = c_logCycleCount->isChecked();
    loggerConfig.logFileName = e_logFilename->text().toStdString();

    loggerConfig.logRegisters = LogRegister::NONE;
    auto logRegister = LogRegister::CC;
    for (auto *regCheckBox : regCheckBoxes)
    {
        if (regCheckBox->isChecked())
        {
            loggerConfig.logRegisters |= logRegister;
        }
        logRegister <<= 1;
    }

    return loggerConfig;
}

void Mc6809LoggerConfigUi::ConnectSignalsWithSlots()
{
    connect(c_buttonBox, &QDialogButtonBox::accepted,
            this, &Mc6809LoggerConfigUi::OnAccepted);
    connect(c_buttonBox, &QDialogButtonBox::rejected,
            this, &Mc6809LoggerConfigUi::OnRejected);
    connect(c_buttonBox, &QDialogButtonBox::clicked,
            this, &Mc6809LoggerConfigUi::OnClicked);
    connect(b_logFilename, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_logFilename); });
}

void Mc6809LoggerConfigUi::OnAccepted()
{
    dialog->done(QDialog::Accepted);
}

void Mc6809LoggerConfigUi::OnRejected()
{
    dialog->done(QDialog::Rejected);
}

void Mc6809LoggerConfigUi::OnClicked(QAbstractButton *button)
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

        for (auto *regCheckBox : regCheckBoxes)
        {
            regCheckBox->setChecked(false);
        }
    }
}

void Mc6809LoggerConfigUi::OnSelectFile(QLineEdit &lineEdit)
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

