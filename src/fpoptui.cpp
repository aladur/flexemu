/*
    fpoptui.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2020-2023  W. Schwotzer

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
#include "warnoff.h"
#include <QDir>
#include <QLineEdit>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include "warnon.h"

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

void FlexplorerOptionsUi::TransferDataToDialog(const struct sFPOptions &options)
{
    if (dialog == nullptr)
    {
        throw std::logic_error("setupUi(dialog) has to be called before.");
    }

    e_bootSectorFile->setText(options.bootSectorFile.c_str());

    auto index = static_cast<int>(options.ft_access);
    index = (index == 3) ? 2 : index;
    cb_fileTimeAccess->setCurrentIndex(index);

    c_injectTextFileConvert->setChecked(options.injectTextFileConvert);
    c_injectTextFileAskUser->setChecked(options.injectTextFileAskUser);
    c_extractTextFileConvert->setChecked(options.extractTextFileConvert);
    c_extractTextFileAskUser->setChecked(options.extractTextFileAskUser);
}

void FlexplorerOptionsUi::TransferDataFromDialog(struct sFPOptions &options)
{
    options.bootSectorFile =
        QDir::toNativeSeparators(e_bootSectorFile->text()).toStdString();

    auto index = cb_fileTimeAccess->currentIndex();
    index = (index == 2) ? 3 : index;
    options.ft_access = static_cast<FileTimeAccess>(index);

    options.injectTextFileConvert = c_injectTextFileConvert->isChecked();
    options.injectTextFileAskUser = c_injectTextFileAskUser->isChecked();
    options.extractTextFileConvert = c_extractTextFileConvert->isChecked();
    options.extractTextFileAskUser = c_extractTextFileAskUser->isChecked();
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

void FlexplorerOptionsUi::OnSelectBootSectorFile()
{
    QString bootSectorFile(e_bootSectorFile->text());

    bootSectorFile = QFileDialog::getOpenFileName(
        dialog, tr("Select Boot Sector File"), bootSectorFile,
        tr("Boot Sector Files (*);;All files (*.*)"));
    bootSectorFile = QDir::toNativeSeparators(bootSectorFile);

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

