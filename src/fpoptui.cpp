/*
    fpoptui.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2020  W. Schwotzer

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
#include "fpoptui.h"
#include <stdexcept>
#include <QLineEdit>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

FlexplorerOptionsUi::FlexplorerOptionsUi() :
    Ui_FlexplorerOptions(), dialog(nullptr)
{
}

FlexplorerOptionsUi::~FlexplorerOptionsUi()
{
}

void FlexplorerOptionsUi::setupUi(QDialog &p_dialog)
{
    dialog = &p_dialog;
    Ui_FlexplorerOptions::setupUi(dialog);

    InitializeWidgets();
    ConnectSignalsWithSlots();
}

void FlexplorerOptionsUi::InitializeWidgets()
{
}

void FlexplorerOptionsUi::TransferDataToDialog(const QString &bootSectorFile)
{
    if (dialog == nullptr)
    {
        throw std::logic_error("setupUi(dialog) has to be called before.");
    }

    e_bootSectorFile->setText(bootSectorFile);
}

void FlexplorerOptionsUi::ConnectSignalsWithSlots()
{
    QObject::connect(b_bootSectorFile, &QAbstractButton::clicked,
        this, &FlexplorerOptionsUi::OnSelectBootSectorFile);

    QObject::connect(c_buttonBox, &QDialogButtonBox::accepted,
        this, &FlexplorerOptionsUi::OnAccepted);
    QObject::connect(c_buttonBox, &QDialogButtonBox::rejected,
            this, &FlexplorerOptionsUi::OnRejected);
}

QString FlexplorerOptionsUi::GetBootSectorFile() const
{
    return e_bootSectorFile->text();
}

void FlexplorerOptionsUi::OnSelectBootSectorFile()
{
    QString bootSectorFile(e_bootSectorFile->text());

    bootSectorFile = QFileDialog::getOpenFileName(
        dialog, tr("Select Boot Sector File"), bootSectorFile,
        tr("Boot Sector Files (*);;All files (*.*)"));

    if (!bootSectorFile.isEmpty())
    {
        e_bootSectorFile->setText(bootSectorFile);
    }
}

bool FlexplorerOptionsUi::Validate()
{
    QFileInfo info(e_bootSectorFile->text());

    if (!info.exists())
    {
        e_bootSectorFile->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(dialog, tr("Flexplorer Error"), 
                tr("Boot Sector File does not exist"));

        return false;
    }
    else if (!info.isFile() || (info.size() != 256 && info.size() != 512))
    {
        e_bootSectorFile->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(dialog, tr("Flexplorer Error"), 
                tr("Invalid Boot Sector File"));

        return false;
    }

    return true;
}

void FlexplorerOptionsUi::OnAccepted()
{
    if (Validate())
    {
        dialog->done(QDialog::Accepted);
    }
}

void FlexplorerOptionsUi::OnRejected()
{
    dialog->done(QDialog::Rejected);
}

