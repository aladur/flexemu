/*
    memsetui.cpp


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


#include "memsetui.h"
#include "bui.h"
#include "efslctle.h"
#include "qtfree.h"
#include <cassert>
#include "warnoff.h"
#include <QLatin1Char>
#include <QString>
#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include "warnon.h"
#include "memwin.h"


MemorySettingsUi::MemorySettingsUi() :
    Ui_MemorySettings()
{
}

void MemorySettingsUi::setupUi(QDialog &p_dialog)
{
    dialog = &p_dialog;
    Ui_MemorySettings::setupUi(dialog);

    assert(e_startAddress != nullptr);
    assert(e_endAddress != nullptr);
    assert(e_windowTitle != nullptr);
    assert(cb_style != nullptr);
    assert(c_withAddress != nullptr);
    assert(c_withAscii != nullptr);
    assert(c_isUpdateWindowSize != nullptr);
    assert(c_withExtraSpace != nullptr);

    InitializeWidgets();
    ConnectSignalsWithSlots();
}

void MemorySettingsUi::InitializeWidgets()
{
    cb_style->addItems(MemoryWindow::GetStyleStrings());
    const auto &text = MemoryWindow::GetStyleStrings().back();
    cb_style->setMinimumContentsLength(cast_from_qsizetype(text.size()));

    ::InstallSelectionEventFilter(*e_startAddress, this);
    ::InstallSelectionEventFilter(*e_endAddress, this);
    const QString inputMask(">hhhh");
    e_startAddress->setInputMask(inputMask);
    e_endAddress->setInputMask(inputMask);

    QString rxString("[^,;]*");
    QRegularExpression regex(rxString);
    auto *validator = new QRegularExpressionValidator(regex, dialog);
    e_windowTitle->setValidator(validator);
    e_windowTitle->setMaxLength(255);
}

void MemorySettingsUi::SetData(const MemoryWindow::Config_t &config) const
{
    const auto latin1Zero = QLatin1Char('0');
    QString text;

    if (dialog == nullptr)
    {
        throw std::logic_error("setupUi(dialog) has to be called before.");
    }

    if (auto index = MemoryWindow::GetStyleValues().indexOf(config.style);
        index >= 0)
    {
        cb_style->setCurrentIndex(index);
    }

    text = QString("%1").arg(config.addressRange.lower(), 4, 16, latin1Zero);
    e_startAddress->setText(text);
    text = QString("%1").arg(config.addressRange.upper(), 4, 16, latin1Zero);
    e_endAddress->setText(text);
    c_withAddress->setChecked(config.withAddress);
    c_withAscii->setChecked(config.withAscii);
    c_isUpdateWindowSize->setChecked(config.isUpdateWindowSize);
    c_withExtraSpace->setChecked(config.withExtraSpace);

    text = MemoryWindow::GetDefaultWindowTitle();
    e_windowTitle->setPlaceholderText(text);
    e_windowTitle->setText(QString::fromStdString(config.windowTitle));
}

void MemorySettingsUi::GetData(MemoryWindow::Config_t &config) const
{
    bool isSuccess;
    DWord startAddress;
    DWord endAddress;
    unsigned value;

    if (auto index = cb_style->currentIndex(); index >= 0)
    {
        config.style = MemoryWindow::GetStyleValues()[index];
    }

    value = e_startAddress->text().toUInt(&isSuccess, 16);
    assert(isSuccess);
    startAddress = static_cast<DWord>(value);
    value = e_endAddress->text().toUInt(&isSuccess, 16);
    assert(isSuccess);
    endAddress = static_cast<DWord>(value);
    if (startAddress > endAddress)
    {
        const auto temp = startAddress;

        startAddress = endAddress;
        endAddress = temp;
    }

    config.addressRange = { startAddress, endAddress };
    config.windowTitle = e_windowTitle->text().toStdString();
    config.withAddress = c_withAddress->isChecked();
    config.withAscii = c_withAscii->isChecked();
    config.isUpdateWindowSize = c_isUpdateWindowSize->isChecked();
    config.withExtraSpace = c_withExtraSpace->isChecked();
}


void MemorySettingsUi::ConnectSignalsWithSlots()
{
    connect(c_buttonBox, &QDialogButtonBox::accepted,
            this, &MemorySettingsUi::OnAccepted);
    connect(c_buttonBox, &QDialogButtonBox::rejected,
            this, &MemorySettingsUi::OnRejected);
}

void MemorySettingsUi::OnAccepted()
{
    if (Validate())
    {
        dialog->done(QDialog::Accepted);
    }
}

void MemorySettingsUi::OnRejected()
{
    dialog->done(QDialog::Rejected);
}

bool MemorySettingsUi::Validate()
{
    if (e_startAddress->text().isEmpty())
    {
        e_startAddress->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(dialog, tr("Flexemu Error"),
                tr("Start address is invalid"));

        return false;
    }

    if (e_endAddress->text().isEmpty())
    {
        e_endAddress->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(dialog, tr("Flexemu Error"),
                tr("End address is invalid"));

        return false;
    }

    return true;
}
