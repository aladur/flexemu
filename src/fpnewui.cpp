/*
    fpnewui.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2024  W. Schwotzer

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

    e_tracks->setValue(tracks);
    e_sectors->setValue(sectors);

    if (path.isEmpty())
    {
        e_path->setText(defaultPath + PATHSEPARATORSTRING +
            "new." + GetCurrentFileExtension());
    }
    else
    {
        e_path->setText(path);
    }
    UpdateFilename();
}

void FlexplorerNewUi::ConnectSignalsWithSlots()
{
    connect(r_dskFile, &QAbstractButton::toggled,
            this, &FlexplorerNewUi::OnDskFileFormat);
    connect(r_flxFile, &QAbstractButton::toggled,
            this, &FlexplorerNewUi::OnFlxFileFormat);
    connect(r_mdcrFile, &QAbstractButton::toggled,
            this, &FlexplorerNewUi::OnMdcrFileFormat);
    connect(e_tracks,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
#else
            QOverload<int>::of(&QSpinBox::valueChanged),
#endif
            this, &FlexplorerNewUi::OnTrackChanged);
    connect(e_sectors,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
#else
            QOverload<int>::of(&QSpinBox::valueChanged),
#endif
            this, &FlexplorerNewUi::OnSectorChanged);
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
    return e_tracks->value();
}

int FlexplorerNewUi::GetSectors() const
{
    return e_sectors->value();
}

QString FlexplorerNewUi::GetPath() const
{
    auto path = QDir::toNativeSeparators(e_path->text());

    auto pIdx = path.lastIndexOf(PATHSEPARATOR);
    if (pIdx < 0)
    {
        path = defaultPath + PATHSEPARATORSTRING + path;
    }
    auto index = path.lastIndexOf('.');
    if (index < 0 || index < pIdx)
    {
        path += "." + GetCurrentFileExtension();
    }

    return path;
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
        UpdateFilename();
    }
}

void FlexplorerNewUi::OnFlxFileFormat(bool value)
{
    if (value)
    {
        format = TYPE_FLX_CONTAINER;
        UpdateFormatTrkSecEnable(false);
        UpdateFilename();
    }
}

void FlexplorerNewUi::OnMdcrFileFormat(bool value)
{
    if (value)
    {
        format = TYPE_MDCR_CONTAINER;
        UpdateFormatTrkSecEnable(true);
        UpdateFilename();
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
        e_tracks->setValue(trk_sec.trk + 1);
        e_sectors->setValue(trk_sec.sec);
    }

    e_tracks->setEnabled(!isMdcrFormat && isFreeDiskFormat);
    e_sectors->setEnabled(!isMdcrFormat && isFreeDiskFormat);
}

void FlexplorerNewUi::OnSectorChanged(int sectors)
{
    OnTrkSecChanged(GetTracks(), sectors);
}

void FlexplorerNewUi::OnTrackChanged(int tracks)
{
    OnTrkSecChanged(tracks, GetSectors());
}

void FlexplorerNewUi::OnTrkSecChanged(int tracks, int sectors)
{
    bool isWarning = true;
    for (const auto &st : flex_formats)
    {
        if ((st.trk + 1) == tracks && st.sec == sectors)
        {
            isWarning = false;
            break;
        }
    }

    auto msg = tr("Uncommon FLEX disk format. "
                  "This may cause compatibility issues!");
    auto richText = QString("<p style=\"color:orange\">%1</p>").arg(msg);
    l_formatWarning->setText(isWarning ? richText : "");
}

void FlexplorerNewUi::OnSelectPath()
{
    QString caption = tr("Save disk file");
    QString filter =
                tr("FLEX disk image files (*.dsk *.flx *.wta);;All files (*.*)");

    if (format == TYPE_MDCR_CONTAINER)
    {
        caption = tr("Save MDCR file");
        filter = tr("MDCR containers (*.mdcr);;All files (*.*)");
    }

    QFileDialog fileDialog(dialog, caption, defaultPath, filter);

    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.selectFile(e_path->text());
    fileDialog.setDefaultSuffix(GetCurrentFileExtension());

    if (fileDialog.exec() == QDialog::Accepted)
    {
        defaultPath =
            QDir::toNativeSeparators(fileDialog.directory().absolutePath());
        auto files = fileDialog.selectedFiles();
        if (files.size() > 0)
        {
            e_path->setText(QDir::toNativeSeparators(files[0]));
        }
    }
}

bool FlexplorerNewUi::Validate()
{
    if (e_path->text().isEmpty())
    {
        e_path->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(dialog, tr("Flexplorer Error"), 
                tr("File path is invalid"));

        return false;
    }

    if (QFileInfo::exists(e_path->text()))
    {
        e_path->setFocus(Qt::OtherFocusReason);
        auto path = QDir::toNativeSeparators(e_path->text());
        auto fileName = getFileName(path.toStdString());
        auto result = QMessageBox::question(dialog, tr("Confirm Save"),
            QString(fileName.c_str()) +
            tr(" already exists.\nDo You want to replace it?"));

        return result == QMessageBox::Yes;
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

QString FlexplorerNewUi::GetDefaultPath() const
{
    return defaultPath;
}

void FlexplorerNewUi::SetDefaultPath(const QString &path)
{
    defaultPath = QDir::toNativeSeparators(path);
}

void FlexplorerNewUi::UpdateFilename()
{
    auto fileExtension = QString(".") + GetCurrentFileExtension();

    auto path = QDir::toNativeSeparators(e_path->text());
    if (path.isEmpty())
    {
        path = "new" + fileExtension;
    }
    if (path.lastIndexOf(PATHSEPARATOR) < 0)
    {
        path = defaultPath + PATHSEPARATORSTRING + path;
    }
    auto pIdx = path.lastIndexOf(PATHSEPARATOR);
    auto index = path.lastIndexOf('.');
    if (index >= 0 && index > pIdx)
    {
        path = path.left(index) + fileExtension;
    }
    else if (pIdx >= 0 && pIdx < path.size() - 1)
    {
        path = path + fileExtension;
    }
    else if (pIdx >= 0 && pIdx == path.size() - 1)
    {
        path = path + "new" + fileExtension;
    }
    e_path->setText(path);
}

QString FlexplorerNewUi::GetCurrentFileExtension() const
{
    switch (format)
    {
    case TYPE_DSK_CONTAINER:
        return "dsk";
    case TYPE_FLX_CONTAINER:
        return "flx";
    case TYPE_MDCR_CONTAINER:
        return "mdcr";
    }

    return "";
}

