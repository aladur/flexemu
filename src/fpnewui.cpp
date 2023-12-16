/*
    fpnewui.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2023  W. Schwotzer

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
#include "fpnewui.h"
#include "mdcrtape.h"
#include "filecnts.h"
#include "warnoff.h"
#include <QLineEdit>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include "warnon.h"

FlexplorerNewUi::FlexplorerNewUi() :
    Ui_FlexplorerNew(), dialog(nullptr), format(TYPE_DSK_CONTAINER)
{
}

FlexplorerNewUi::~FlexplorerNewUi()
{
}

void FlexplorerNewUi::setupUi(QDialog &p_dialog)
{
    dialog = &p_dialog;
    Ui_FlexplorerNew::setupUi(dialog);

    InitializeWidgets();
    AddTracksValidator(*e_tracks);
    AddSectorsValidator(*e_sectors);
    ConnectSignalsWithSlots();
}

void FlexplorerNewUi::InitializeWidgets()
{
    r_dskFile->setChecked(true);

    int index = 0;
    cb_diskFormat->addItem("[Set Tracks and Sectors]", index);
    for (const auto *description : flex_format_descriptions)
    {
        cb_diskFormat->addItem(description, ++index);
    }
    cb_diskFormat->setMaxVisibleItems(cb_diskFormat->count());
    cb_diskFormat->setCurrentIndex(0);
}

void FlexplorerNewUi::TransferDataToDialog(int p_format,
                                           int tracks, int sectors,
                                           const QString &path)
{
    if (dialog == nullptr)
    {
        throw std::logic_error("setupUi(dialog) has to be called before.");
    }

    format = p_format;
    switch (format)
    {
        case TYPE_FLX_CONTAINER:
            r_flxFile->setChecked(true);
            break;
        case TYPE_MDCR_CONTAINER:
            r_mdcrFile->setChecked(true);
            break;
        default:
            format = TYPE_DSK_CONTAINER;
            r_dskFile->setChecked(true);
            break;
    }

    auto text = QString::asprintf("%d", tracks);
    e_tracks->setText(text);
    text = QString::asprintf("%d", sectors);
    e_sectors->setText(text);
    e_path->setText(path);
}

void FlexplorerNewUi::ConnectSignalsWithSlots()
{
    connect(r_dskFile, &QAbstractButton::toggled,
            this, &FlexplorerNewUi::OnDskFileFormat);
    connect(r_flxFile, &QAbstractButton::toggled,
            this, &FlexplorerNewUi::OnFlxFileFormat);
    connect(r_mdcrFile, &QAbstractButton::toggled,
            this, &FlexplorerNewUi::OnMdcrFileFormat);
    connect(e_tracks, &QLineEdit::textChanged,
            this, &FlexplorerNewUi::OnTrkSecChanged);
    connect(e_sectors, &QLineEdit::textChanged,
            this, &FlexplorerNewUi::OnTrkSecChanged);
    connect(cb_diskFormat,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::currentIndexChanged),
#else
            QOverload<int>::of(&QComboBox::currentIndexChanged),
#endif
            this, &FlexplorerNewUi::OnFormatChanged);
    connect(b_path, &QAbstractButton::clicked,
            this, &FlexplorerNewUi::OnSelectPath);

    connect(c_buttonBox, &QDialogButtonBox::accepted,
            this, &FlexplorerNewUi::OnAccepted);
    connect(c_buttonBox, &QDialogButtonBox::rejected,
            this, &FlexplorerNewUi::OnRejected);
}

int FlexplorerNewUi::GetTracks() const
{
    return e_tracks->text().toInt();
}

int FlexplorerNewUi::GetSectors() const
{
    return e_sectors->text().toInt();
}

QString FlexplorerNewUi::GetPath() const
{
    return e_path->text();
}

int FlexplorerNewUi::GetFormat() const
{
    return format;
}


void FlexplorerNewUi::OnDskFileFormat(bool value)
{
    if (value)
    {
        format = TYPE_DSK_CONTAINER;
        UpdateFormatTrkSecEnable(false);
    }
}

void FlexplorerNewUi::OnFlxFileFormat(bool value)
{
    if (value)
    {
        format = TYPE_FLX_CONTAINER;
        UpdateFormatTrkSecEnable(false);
    }
}

void FlexplorerNewUi::OnMdcrFileFormat(bool value)
{
    if (value)
    {
        format = TYPE_MDCR_CONTAINER;
        UpdateFormatTrkSecEnable(true);
    }

}

void FlexplorerNewUi::UpdateFormatTrkSecEnable(bool isMdcrFormat)
{
    bool isFreeDiskFormat = (cb_diskFormat->currentIndex() == 0);

    cb_diskFormat->setEnabled(!isMdcrFormat);
    e_tracks->setEnabled(!isMdcrFormat && isFreeDiskFormat);
    e_sectors->setEnabled(!isMdcrFormat && isFreeDiskFormat);
}

void FlexplorerNewUi::OnFormatChanged(int index)
{
    bool isMdcrFormat = r_mdcrFile->isChecked();
    bool isFreeDiskFormat = (index == 0);

    if (!isFreeDiskFormat)
    {
        auto trk_sec = flex_formats[index - 1];                             
        auto text = QString::asprintf("%d", static_cast<int>(trk_sec.trk));
        e_tracks->setText(text);
        text = QString::asprintf("%d", static_cast<int>(trk_sec.sec));
        e_sectors->setText(text);
    }

    e_tracks->setEnabled(!isMdcrFormat && isFreeDiskFormat);
    e_sectors->setEnabled(!isMdcrFormat && isFreeDiskFormat);
}

void FlexplorerNewUi::OnTrkSecChanged()
{
    auto tracks = GetTracks();
    auto sectors = GetSectors();

    bool isWarning = true;
    for (const auto &st : flex_formats)
    {
        if (st.trk == tracks && st.sec == sectors)
        {
            isWarning = false;
            break;
        }
    }

    auto msg = tr("Uncommon FLEX disk format. "
                  "This may cause compatibility issues!");
    auto richText = QString::asprintf("<p style=\"color:orange\">%s</p>",
                        msg.toUtf8().data());
    l_formatWarning->setText(isWarning ? richText : "");
}

void FlexplorerNewUi::OnSelectPath()
{
    QString path(e_path->text());

    if (format == TYPE_MDCR_CONTAINER)
    {
        path = QFileDialog::getSaveFileName(
            dialog, tr("Select a MDCR file"), path,
            tr("MDCR containers (*.mdcr);;All files (*.*)"));
    }
    else
    {
        path = QFileDialog::getSaveFileName(
            dialog, tr("Select a Disk file"), path,
            tr("FLEX file containers (*.dsk *.flx *.wta);;All files (*.*)"));
    }

    if (!path.isEmpty())
    {
        e_path->setText(path);
    }
}

void FlexplorerNewUi::AddTracksValidator(QLineEdit &lineEdit)
{
    e_tracks->setValidator(new QIntValidator(2, 255, &lineEdit));

}

void FlexplorerNewUi::AddSectorsValidator(QLineEdit &lineEdit)
{
    e_sectors->setValidator(new QIntValidator(6, 255, &lineEdit));
}

bool FlexplorerNewUi::Validate()
{
    if (e_tracks->isEnabled() && !e_tracks->hasAcceptableInput())
    {
        auto validator =
            static_cast<const QIntValidator *>(e_tracks->validator());

        e_tracks->setFocus(Qt::OtherFocusReason);
        auto message = QString::asprintf(
                              "Track count is not in the valid\n"
                              "range of %d ... %d.",
                              validator->bottom(), validator->top());
        QMessageBox::critical(dialog, tr("Flexplorer Error"), message);

        return false;
    }

    if (e_sectors->isEnabled() && !e_sectors->hasAcceptableInput())
    {
        auto validator =
            static_cast<const QIntValidator *>(e_sectors->validator());

        e_sectors->setFocus(Qt::OtherFocusReason);
        auto message = QString::asprintf(
                              "Sector count is not in the valid\n"
                              "range of %d ... %d.",
                              validator->bottom(), validator->top());
        QMessageBox::critical(dialog, tr("Flexplorer Error"), message);

        return false;
    }

    if (e_path->text().isEmpty())
    {
        e_path->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(dialog, tr("Flexplorer Error"), 
                tr("File path is invalid"));

        return false;
    }
    else if (QFileInfo::exists(e_path->text()))
    {
        e_path->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(dialog, tr("Flexplorer Error"), 
                tr("File already exists"));

        return false;
    }

    return true;
}

void FlexplorerNewUi::OnAccepted()
{
    if (Validate())
    {
        dialog->done(QDialog::Accepted);
    }
}

void FlexplorerNewUi::OnRejected()
{
    dialog->done(QDialog::Rejected);
}

