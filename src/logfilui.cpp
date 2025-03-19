/*
    logfilui.cpp


    Flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2025  W. Schwotzer

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
#include <optional>
#include "warnon.h"

using OptionalWord = std::optional<Word>;

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

    assert(g_enable != nullptr);
    assert(e_minAddress != nullptr);
    assert(e_maxAddress != nullptr);
    assert(e_startAddress != nullptr);
    assert(e_stopAddress != nullptr);
    assert(c_logCycleCount != nullptr);
    assert(c_loopOptimization != nullptr);
    assert(e_logFilename != nullptr);
    assert(r_semicolon != nullptr);
    assert(r_space != nullptr);
    assert(r_tab != nullptr);
    assert(r_csv != nullptr);
    assert(r_text != nullptr);

#if defined(_DEBUG) || defined(DEBUG)
    for (const auto &regCheckBox : regCheckBoxes)
    {
        assert(regCheckBox != nullptr);
    }
#endif

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

    g_enable->setChecked(loggerConfig.isEnabled);
    ::SetData(loggerConfig.minAddr, *e_minAddress);
    ::SetData(loggerConfig.maxAddr, *e_maxAddress);
    ::SetData(loggerConfig.startAddr, *e_startAddress);
    ::SetData(loggerConfig.stopAddr, *e_stopAddress);

    c_logCycleCount->setChecked(loggerConfig.logCycleCount);
    c_loopOptimization->setChecked(loggerConfig.isLoopOptimization);

    switch (loggerConfig.format)
    {
        case Mc6809LoggerConfig::Format::Text:
            r_text->setChecked(true);
            g_csvSeparator->setEnabled(false);
            break;

        case Mc6809LoggerConfig::Format::Csv:
            r_csv->setChecked(true);
            g_csvSeparator->setEnabled(true);
            break;
    }

    switch (loggerConfig.csvSeparator)
    {
        case ' ':
            r_space->setChecked(true);
            break;

        case '\t':
            r_tab->setChecked(true);
            break;

        default:
        case ';':
            r_semicolon->setChecked(true);
            break;
    }

    e_logFilename->setText(QString::fromStdString(
                loggerConfig.logFilePath.u8string()));

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

    loggerConfig.minAddr = ::GetData<OptionalWord>(*e_minAddress);
    loggerConfig.maxAddr = ::GetData<OptionalWord>(*e_maxAddress);
    loggerConfig.startAddr = ::GetData<OptionalWord>(*e_startAddress);
    loggerConfig.stopAddr = ::GetData<OptionalWord>(*e_stopAddress);

    loggerConfig.logCycleCount = c_logCycleCount->isChecked();
    loggerConfig.isLoopOptimization = c_loopOptimization->isChecked();
    loggerConfig.logFilePath = fs::u8path(e_logFilename->text().toStdString());

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

    if (r_csv->isChecked())
    {
        loggerConfig.format = Mc6809LoggerConfig::Format::Csv;
    }
    else if (r_text->isChecked())
    {
        loggerConfig.format = Mc6809LoggerConfig::Format::Text;
    }

    if (r_space->isChecked())
    {
        loggerConfig.csvSeparator = ' ';
    }
    else if (r_tab->isChecked())
    {
        loggerConfig.csvSeparator = '\t';
    }
    else if (r_semicolon->isChecked())
    {
        loggerConfig.csvSeparator = ';';
    }

    loggerConfig.isEnabled = g_enable->isChecked();

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
    connect(r_text, &QRadioButton::clicked,
            this, &Mc6809LoggerConfigUi::OnTextFormat);
    connect(r_csv, &QRadioButton::clicked,
            this, &Mc6809LoggerConfigUi::OnCsvFormat);
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
        r_text->setChecked(true);
        r_semicolon->setChecked(true);
        g_csvSeparator->setEnabled(false);

        for (auto *regCheckBox : regCheckBoxes)
        {
            regCheckBox->setChecked(false);
        }

        g_enable->setChecked(false);
    }
}

void Mc6809LoggerConfigUi::OnTextFormat() const
{
    if (g_enable->isChecked())
    {
        const auto qPath = QDir::toNativeSeparators(e_logFilename->text());
        auto path = qPath.toStdString();
        path = flx::updateFilename(path, "mc6809", ".log");
        e_logFilename->setText(path.c_str());
        g_csvSeparator->setEnabled(false);
    }
}

void Mc6809LoggerConfigUi::OnCsvFormat() const
{
    if (g_enable->isChecked())
    {
        const auto qPath = QDir::toNativeSeparators(e_logFilename->text());
        auto path = qPath.toStdString();
        path = flx::updateFilename(path, "mc6809", ".csv");
        e_logFilename->setText(path.c_str());
        g_csvSeparator->setEnabled(true);
    }
}

void Mc6809LoggerConfigUi::OnSelectFile(QLineEdit &lineEdit)
{
    auto path = QDir::currentPath();
    auto fileInfo = QFileInfo(lineEdit.text());
    QString filter;

    path = lineEdit.text();

    if (r_csv->isChecked())
    {
        filter = tr("CSV Logfiles (*.csv);;All files (*.*)");
    }
    else
    {
        filter = tr("Logfiles (*.log);;All files (*.*)");
    }

    const auto caption = tr("Set Logfile");
    path = QFileDialog::getSaveFileName(dialog, caption, path, filter);

    if  (!path.isEmpty())
    {
        lineEdit.setText(path);
    }
}

