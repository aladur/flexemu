/*
    fsetupui.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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
#include "e2.h"
#include "soptions.h"
#include "filecnts.h"
#include "termimpf.h"
#include "colors.h"
#include "flexerr.h"
#include <string>
#include <memory>
#include <stdexcept>
#include "warnoff.h"
#include <QtGlobal>
#include <QAbstractButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontMetrics>
#include <QDir>
#include <QIcon>
#include "warnon.h"
#include <cassert>
#include <array>

#ifdef _MSC_VER
    #include <direct.h>
#endif

#include "fsetupui.h"
#include <filesystem>

namespace fs = std::filesystem;


void FlexemuOptionsUi::ConnectSignalsWithSlots()
{
    connect(b_diskMonitorDir, &QAbstractButton::clicked,
            this, &FlexemuOptionsUi::OnSelectDiskMonitorDir);
    connect(b_monitorPgm, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_monitorPgm, FileType::HexBinaryFile); });

    static const auto fType = FileType::FlexDiskFile;
    connect(b_drive0, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_drive0, fType); });
    connect(b_drive1, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_drive1, fType); });
    connect(b_drive2, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_drive2, fType); });
    connect(b_drive3, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_drive3, fType); });

    static const auto dType = FileType::FlexDiskDirectory;
    connect(b_drive0Dir, &QAbstractButton::clicked,
            [&](){ OnSelectDirectory(*e_drive0, dType); });
    connect(b_drive1Dir, &QAbstractButton::clicked,
            [&](){ OnSelectDirectory(*e_drive1, dType); });
    connect(b_drive2Dir, &QAbstractButton::clicked,
            [&](){ OnSelectDirectory(*e_drive2, dType); });
    connect(b_drive3Dir, &QAbstractButton::clicked,
            [&](){ OnSelectDirectory(*e_drive3, dType); });

    static const auto cType = FileType::CassetteFile;
    connect(b_mdcrDrive0, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_mdcrDrive0, cType); });
    connect(b_mdcrDrive1, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_mdcrDrive1, cType); });

    connect(r_ramExtNone, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateRamDependencies);
    connect(r_ramExt2x96, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateRamDependencies);
    connect(r_ramExt2x288, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateRamDependencies);

    connect(r_eurocom2v5, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateHardwareDependencies);
    connect(r_eurocom2v7, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateHardwareDependencies);

    connect(r_frequencyOriginal, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::OnFrequencyOriginal);
    connect(r_frequencyFast, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::OnFrequencyFast);
    connect(r_frequencySet, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::OnFrequencySet);

    connect(c_multiColorScheme, &QCheckBox::stateChanged,
            this, &FlexemuOptionsUi::OnMultiColorSchemeChanged);
    connect(cb_nColors,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::currentIndexChanged),
#else
            QOverload<int>::of(&QComboBox::currentIndexChanged),
#endif
            this, &FlexemuOptionsUi::OnNColorsChanged);

    connect(c_buttonBox, &QDialogButtonBox::accepted,
            this, &FlexemuOptionsUi::OnAccepted);
    connect(c_buttonBox, &QDialogButtonBox::rejected,
            this, &FlexemuOptionsUi::OnRejected);

    connect(e_tracks,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
#else
            QOverload<int>::of(&QSpinBox::valueChanged),
#endif
            this, &FlexemuOptionsUi::OnTrackChanged);
    connect(e_sectors,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
#else
            QOverload<int>::of(&QSpinBox::valueChanged),
#endif
            this, &FlexemuOptionsUi::OnSectorChanged);

    connect(cb_diskFormat,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::currentIndexChanged),
#else
            QOverload<int>::of(&QComboBox::currentIndexChanged),
#endif
            this, &FlexemuOptionsUi::OnFormatChanged);

    connect(c_isDirectoryDiskActive, &QCheckBox::toggled,
            this, &FlexemuOptionsUi::OnDirectoryDiskActiveChanged);

    connect(r_scrollingTerminal, &QAbstractButton::toggled,
            [&](){ OnTerminalTypeChanged(TerminalType::Scrolling); });

    connect(r_ncursesTerminal, &QAbstractButton::toggled,
            [&](){ OnTerminalTypeChanged(TerminalType::NCurses); });
}

FlexemuOptionsUi::FlexemuOptionsUi()
    : Ui_FlexemuSetup()

{
}

void FlexemuOptionsUi::TransferDataToDialog(const struct sOptions &options)
{
    int index = -1;

    if (dialog == nullptr)
    {
        QMessageBox::critical(nullptr, PROGRAMNAME " Logic Error",
                              "setupUi(dialog) with a valid dialog instance "
                              "has to be called before.");
        return;
    }

    readOnlyOptions = AddDependentReadOnlyOptions(options.readOnlyOptionIds);

    InitializeHardwareHyperlink(options.doc_dir);

    for (int x = 1; x <= SCREEN_SIZES; ++x)
    {
        const auto iconPath = QString(":/resource/screen%1.png").arg(x);
        const auto screenSizeIcon = QIcon(iconPath);

        auto text = QString("x%1").arg(x);

        cb_screenSize->addItem(screenSizeIcon, text);

        if (options.pixelSize == x)
        {
            index = x - 1;
        }
    }
    cb_screenSize->setMaxVisibleItems(cb_screenSize->count());
    cb_screenSize->setCurrentIndex(std::max(index, 0));

    index = -1;
    int n = 0;
    constexpr static std::array<int, 3> ncolor_count{ 2, 8, 64 };
    for (auto nColors : ncolor_count)
    {
        auto text = QString("%1").arg(nColors);
        cb_nColors->addItem(text);

        if (options.nColors == nColors)
        {
            index = n;
        }
        ++n;
    }
    cb_nColors->setCurrentIndex(std::max(index, 0));

    index = -1;
    const auto lcOptionColor = flx::tolower(options.color);
    for (int i = 0; i < static_cast<int>(flx::color_count); i++)
    {
        DWord colorRGBValue;

        const auto *colorName = flx::colors[i].colorName;
        flx::getColorForName(colorName, colorRGBValue);
        cb_color->addItem(tr(colorName));
        QPixmap pixmap(16,16);
        pixmap.fill(QColor(colorName));
        QIcon colorIcon(pixmap);
        cb_color->setItemIcon(i, colorIcon);

        if (!lcOptionColor.compare(colorName))
        {
            index = i;
        }
    }
    cb_color->setMaxVisibleItems(cb_color->count());
    cb_color->setCurrentIndex(std::max(index, 0));

    bool isMultiColorSchemeChecked =
        (0 == lcOptionColor.compare("default"));

    c_multiColorScheme->setChecked(isMultiColorSchemeChecked);

    c_isInverse->setChecked(options.isInverse != 0);
    c_isConfirmExit->setChecked(options.isConfirmExit != 0);

    c_undocumented->setChecked(options.use_undocumented);

    e_monitorPgm->setText(QString::fromStdString(options.hex_file.u8string()));
    e_diskMonitorDir->setText(QString::fromStdString(
                options.disk_dir.u8string()));

    e_drive0->setText(QString::fromStdString(options.drives[0].u8string()));
    e_drive1->setText(QString::fromStdString(options.drives[1].u8string()));
    e_drive2->setText(QString::fromStdString(options.drives[2].u8string()));
    e_drive3->setText(QString::fromStdString(options.drives[3].u8string()));

    e_mdcrDrive0->setText(QString::fromStdString(
                options.mdcrDrives[0].u8string()));
    e_mdcrDrive1->setText(QString::fromStdString(
                options.mdcrDrives[1].u8string()));

    c_canFormatDrive0->setChecked(options.canFormatDrives[0]);
    c_canFormatDrive1->setChecked(options.canFormatDrives[1]);
    c_canFormatDrive2->setChecked(options.canFormatDrives[2]);
    c_canFormatDrive3->setChecked(options.canFormatDrives[3]);

    if (options.isRamExtension)
    {
        if (options.isHiMem)
        {
            r_ramExt2x288->setChecked(true);
        }
        else
        {
            r_ramExt2x96->setChecked(true);
        }
    }
    else
    {
        r_ramExtNone->setChecked(true);
    }

    if (!options.isRamExtension)
    {
        cb_nColors->setCurrentIndex(0);
    }

    c_flexibleMmu->setChecked(options.isHiMem && options.isFlexibleMmu);

    c_useRtc->setChecked(options.useRtc);

    r_eurocom2v5->setChecked(options.isEurocom2V5);
    r_eurocom2v7->setChecked(!options.isEurocom2V5);

    r_frequencySet->setChecked(true);
    if (options.frequency < 0.0F)
    {
        r_frequencyOriginal->setChecked(true);
    }
    else if (options.frequency == 0.0F)
    {
        r_frequencyFast->setChecked(true);
    }
    else
    {
        QString frequency_string = QLocale::c().toString(options.frequency);

        r_frequencySet->setChecked(true);
        e_frequency->setText(frequency_string);
    }

    auto setFileTime = (options.fileTimeAccess ==
           (FileTimeAccess::Get | FileTimeAccess::Set));
    c_fileTime->setChecked(setFileTime);

    c_isDisplaySmooth->setChecked(options.isSmooth != 0);

    r_scrollingTerminal->setChecked(options.terminalType == 1);
    r_ncursesTerminal->setChecked(options.terminalType == 2);
    c_terminalIgnoreNUL->setChecked(options.isTerminalIgnoreNUL);
    c_terminalIgnoreESC->setChecked(options.isTerminalIgnoreESC);

    index = 0;
    int selected_index = 0;
    cb_diskFormat->addItem(tr("[Set Tracks and Sectors]"), index);
    for (const auto *description : flex_format_descriptions)
    {
        if (flex_formats[index].trk + 1 == options.directoryDiskTracks &&
            flex_formats[index].sec == options.directoryDiskSectors)
        {
            selected_index = index + 1;
        }

        cb_diskFormat->addItem(description, ++index);
    }
    cb_diskFormat->setMaxVisibleItems(cb_diskFormat->count());
    cb_diskFormat->setCurrentIndex(selected_index);

    e_tracks->setValue(options.directoryDiskTracks);
    e_sectors->setValue(options.directoryDiskSectors);

    c_isDirectoryDiskActive->setChecked(true);
    c_isDirectoryDiskActive->setChecked(options.isDirectoryDiskActive);

    SetOptionsReadOnly(readOnlyOptions);
}

void FlexemuOptionsUi::SetOptionsReadOnly(const std::vector<FlexemuOptionId>
        &p_readOnlyOptions)
{
    for (auto readOnlyOptionId : p_readOnlyOptions)
    {
        switch (readOnlyOptionId)
        {
            case FlexemuOptionId::Drive0:
                e_drive0->setEnabled(false);
                b_drive0->setEnabled(false);
                b_drive0Dir->setEnabled(false);
                break;

            case FlexemuOptionId::Drive1:
                e_drive1->setEnabled(false);
                b_drive1->setEnabled(false);
                b_drive1Dir->setEnabled(false);
                break;

            case FlexemuOptionId::Drive2:
                e_drive2->setEnabled(false);
                b_drive2->setEnabled(false);
                b_drive2Dir->setEnabled(false);
                break;

            case FlexemuOptionId::Drive3:
                e_drive3->setEnabled(false);
                b_drive3->setEnabled(false);
                b_drive3Dir->setEnabled(false);
                break;

            case FlexemuOptionId::CanFormatDrive0:
                c_canFormatDrive0->setEnabled(false);
                break;

            case FlexemuOptionId::CanFormatDrive1:
                c_canFormatDrive1->setEnabled(false);
                break;

            case FlexemuOptionId::CanFormatDrive2:
                c_canFormatDrive2->setEnabled(false);
                break;

            case FlexemuOptionId::CanFormatDrive3:
                c_canFormatDrive3->setEnabled(false);
                break;

            case FlexemuOptionId::MdcrDrive0:
                e_mdcrDrive0->setEnabled(false);
                b_mdcrDrive0->setEnabled(false);
                break;

            case FlexemuOptionId::MdcrDrive1:
                e_mdcrDrive1->setEnabled(false);
                b_mdcrDrive1->setEnabled(false);
                break;

            case FlexemuOptionId::HexFile:
                e_monitorPgm->setEnabled(false);
                b_monitorPgm->setEnabled(false);
                break;

            case FlexemuOptionId::DiskDirectory:
                e_diskMonitorDir->setEnabled(false);
                b_diskMonitorDir->setEnabled(false);
                break;

            case FlexemuOptionId::IsUseUndocumented:
                c_undocumented->setEnabled(false);
                break;

            case FlexemuOptionId::Frequency:
                r_frequencyOriginal->setEnabled(false);
                r_frequencyFast->setEnabled(false);
                r_frequencySet->setEnabled(false);
                e_frequency->setEnabled(false);
                break;

            case FlexemuOptionId::Color:
                cb_color->setEnabled(false);
                c_multiColorScheme->setEnabled(false);
                break;

            case FlexemuOptionId::NColors:
                cb_nColors->setEnabled(false);
                break;

            case FlexemuOptionId::IsInverse:
                c_isInverse->setEnabled(false);
                break;

            case FlexemuOptionId::PixelSize:
                cb_screenSize->setEnabled(false);
                break;

            case FlexemuOptionId::IsRamExt2x96:
            case FlexemuOptionId::IsRamExt2x288:
                r_ramExtNone->setEnabled(false);
                r_ramExt2x96->setEnabled(false);
                r_ramExt2x288->setEnabled(false);
                break;

            case FlexemuOptionId::IsFlexibleMmu:
                c_flexibleMmu->setEnabled(false);
                break;

            case FlexemuOptionId::IsEurocom2V5:
                r_eurocom2v5->setEnabled(false);
                r_eurocom2v7->setEnabled(false);
                break;

            case FlexemuOptionId::IsUseRtc:
                c_useRtc->setEnabled(false);
                break;

            case FlexemuOptionId::FileTimeAccess:
                c_fileTime->setEnabled(false);
                break;

            case FlexemuOptionId::IsDisplaySmooth:
                c_isDisplaySmooth->setEnabled(false);
                break;

            case FlexemuOptionId::IsTerminalIgnoreESC:
                c_terminalIgnoreESC->setEnabled(false);
                break;

            case FlexemuOptionId::IsTerminalIgnoreNUL:
                c_terminalIgnoreNUL->setEnabled(false);
                break;

            case FlexemuOptionId::TerminalType:
                r_scrollingTerminal->setEnabled(false);
                r_ncursesTerminal->setEnabled(false);
                break;

            case FlexemuOptionId::IsDirectoryDiskActive:
                c_isDirectoryDiskActive->setEnabled(false);
                break;

            case FlexemuOptionId::DirectoryDiskTrkSec:
                e_tracks->setEnabled(false);
                e_sectors->setEnabled(false);
                break;

            case FlexemuOptionId::PrintFont:
            case FlexemuOptionId::IsPrintPageBreakDetected:
            case FlexemuOptionId::PrintOrientation:
            case FlexemuOptionId::PrintPageSize:
            case FlexemuOptionId::PrintUnit:
            case FlexemuOptionId::PrintOutputWindowGeometry:
            case FlexemuOptionId::PrintPreviewDialogGeometry:
            case FlexemuOptionId::PrintConfigs:
            case FlexemuOptionId::IconSize:
            case FlexemuOptionId::IsStatusBarVisible:
            case FlexemuOptionId::IsConfirmExit:
                break;
        }
    }
}

std::vector<FlexemuOptionId> FlexemuOptionsUi::AddDependentReadOnlyOptions(
    const std::vector<FlexemuOptionId> &readOnlyOptions)
{
    std::vector<FlexemuOptionId> result(readOnlyOptions);
    std::vector<FlexemuOptionId> dependents;

    auto addDependent = [&](FlexemuOptionId option){
        if (std::find(result.begin(), result.end(), option) == result.end())
        {
            dependents.push_back(option);
        }
    };

    // Loop at least has to be executed once.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
    do
    {
        for (auto readOnlyOptionId : result)
        {
            dependents.clear();
            switch (readOnlyOptionId)
            {
                case FlexemuOptionId::IsRamExt2x96:
                    addDependent(FlexemuOptionId::IsRamExt2x288);
                    addDependent(FlexemuOptionId::IsFlexibleMmu);
                    addDependent(FlexemuOptionId::IsEurocom2V5);
                    break;

                case FlexemuOptionId::IsRamExt2x288:
                    addDependent(FlexemuOptionId::IsRamExt2x96);
                    addDependent(FlexemuOptionId::IsFlexibleMmu);
                    addDependent(FlexemuOptionId::IsEurocom2V5);
                    break;

                case FlexemuOptionId::IsFlexibleMmu:
                    addDependent(FlexemuOptionId::IsRamExt2x96);
                    addDependent(FlexemuOptionId::IsRamExt2x288);
                    addDependent(FlexemuOptionId::IsEurocom2V5);
                    break;

                case FlexemuOptionId::Drive0:
                case FlexemuOptionId::Drive1:
                case FlexemuOptionId::Drive2:
                case FlexemuOptionId::Drive3:
                case FlexemuOptionId::CanFormatDrive0:
                case FlexemuOptionId::CanFormatDrive1:
                case FlexemuOptionId::CanFormatDrive2:
                case FlexemuOptionId::CanFormatDrive3:
                case FlexemuOptionId::MdcrDrive0:
                case FlexemuOptionId::MdcrDrive1:
                case FlexemuOptionId::IsUseRtc:
                case FlexemuOptionId::IsDirectoryDiskActive:
                case FlexemuOptionId::DirectoryDiskTrkSec:
                    addDependent(FlexemuOptionId::IsEurocom2V5);
                    break;

                case FlexemuOptionId::Color:
                    addDependent(FlexemuOptionId::NColors);
                    addDependent(FlexemuOptionId::IsRamExt2x96);
                    addDependent(FlexemuOptionId::IsRamExt2x288);
                    break;

                case FlexemuOptionId::NColors:
                    addDependent(FlexemuOptionId::Color);
                    addDependent(FlexemuOptionId::IsRamExt2x96);
                    addDependent(FlexemuOptionId::IsRamExt2x288);
                    break;

                case FlexemuOptionId::TerminalType:
                    addDependent(FlexemuOptionId::IsTerminalIgnoreESC);
                    addDependent(FlexemuOptionId::IsTerminalIgnoreNUL);
                    break;

                case FlexemuOptionId::DiskDirectory:
                case FlexemuOptionId::HexFile:
                case FlexemuOptionId::IsEurocom2V5:
                case FlexemuOptionId::IsUseUndocumented:
                case FlexemuOptionId::Frequency:
                case FlexemuOptionId::IsInverse:
                case FlexemuOptionId::PixelSize:
                case FlexemuOptionId::FileTimeAccess:
                case FlexemuOptionId::IsDisplaySmooth:
                case FlexemuOptionId::IsConfirmExit:
                case FlexemuOptionId::IsTerminalIgnoreESC:
                case FlexemuOptionId::IsTerminalIgnoreNUL:
                case FlexemuOptionId::PrintFont:
                case FlexemuOptionId::IsPrintPageBreakDetected:
                case FlexemuOptionId::PrintOrientation:
                case FlexemuOptionId::PrintPageSize:
                case FlexemuOptionId::PrintUnit:
                case FlexemuOptionId::PrintOutputWindowGeometry:
                case FlexemuOptionId::PrintPreviewDialogGeometry:
                case FlexemuOptionId::PrintConfigs:
                case FlexemuOptionId::IconSize:
                case FlexemuOptionId::IsStatusBarVisible:
                    break;
            }
        }

        std::copy(dependents.begin(), dependents.end(),
                std::back_inserter(result));
    } while (!dependents.empty());

    return result;
}

bool FlexemuOptionsUi::IsReadOnly(FlexemuOptionId optionId)
{
    return std::find(readOnlyOptions.cbegin(), readOnlyOptions.cend(),
                     optionId) != readOnlyOptions.cend();
}

void FlexemuOptionsUi::setupUi(QDialog *p_dialog)
{
    dialog = p_dialog;

    Ui_FlexemuSetup::setupUi(p_dialog);
    InitializeUi();
    AddFrequencyValidator(*e_frequency);
    ConnectSignalsWithSlots();
    // This dialog width calculation tries to make all available tabs
    // visible. It is just a rough estimation.
    QFontMetrics metrics(dialog->font());
    e_drive2->setMinimumWidth(metrics.boundingRect('x').width() * 52);

#ifdef _WIN32
    // On Windows hide terminal Tab because there is no terminal mode.
    for (int index = 0; index < c_tabWidget->count(); ++index)
    {
        if (c_tabWidget->tabText(index) == "Terminal")
        {
            c_tabWidget->setTabVisible(index, false);
            break;
        }
    }
#endif
}

bool FlexemuOptionsUi::Validate()
{
    if (e_monitorPgm->text().isEmpty())
    {
        const char *monitor =  r_eurocom2v7->isChecked() ?
                               "neumon54.hex" : "mon24z.s19";

        c_tabWidget->setCurrentIndex(3);
        e_monitorPgm->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(dialog, tr("FSetup Error"),
                              QString("Without a monitor program flexemu "
                                      "will not work.\nHint: ") + monitor +
                                      " is a good choice.");
        return false;
    }

    if (r_frequencySet->isChecked() && !e_frequency->hasAcceptableInput())
    {
        c_tabWidget->setCurrentIndex(0);
        e_frequency->setFocus(Qt::OtherFocusReason);
        e_frequency->setSelection(0, 20);
        const auto *validator =
            dynamic_cast<const QDoubleValidator *>(e_frequency->validator());

        assert(validator != nullptr);
        auto message = tr("CPU Frequency is not in the valid\n"
                          "range of %1 ... %2 MHz")
                          .arg(validator->bottom(), 0, 'f', 1)
                          .arg(validator->top(), 0, 'f', 1);
        QMessageBox::critical(dialog, tr("FSetup Error"), message);

        return false;
    }

    if (r_eurocom2v7->isChecked() &&
        e_drive0->isEnabled() && e_drive0->text().isEmpty())
    {
        c_tabWidget->setCurrentIndex(3);
        e_drive0->setFocus(Qt::OtherFocusReason);
        auto button =
            QMessageBox::warning(dialog, tr("FSetup Warning"),
                                 tr("Without a disk in drive 0 FLEX will not "
                                    "boot.\nHint: system.dsk would be a good "
                                    "choice.\nSave changes?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::Yes);
        return button == QMessageBox::Yes;
    }

    return true;
}

void FlexemuOptionsUi::TransferDataFromDialog(struct sOptions &options)
{
    bool success = false;

    if (!IsReadOnly(FlexemuOptionId::PixelSize))
    {
        options.pixelSize = cb_screenSize->currentIndex() + 1;
    }

    if (!IsReadOnly(FlexemuOptionId::NColors) &&
        !IsReadOnly(FlexemuOptionId::Color))
    {
        auto n = cb_nColors->currentText().toInt(&success);
        options.nColors = (success ? n : 2);

        if (c_multiColorScheme->isChecked() && cb_nColors->currentIndex() > 0)
        {
             options.color = "default";
        }
        else
        {
            options.color = cb_color->currentText().toStdString();

            for (size_t i = 0; i < flx::color_count; i++)
            {
                auto colorName = tr(flx::colors[i].colorName);

                if (colorName == cb_color->currentText())
                {
                    options.color = flx::colors[i].colorName;
                }
            }
        }
    }

    if (!IsReadOnly(FlexemuOptionId::IsInverse))
    {
        options.isInverse = c_isInverse->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::IsConfirmExit))
    {
        options.isConfirmExit = c_isConfirmExit->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::IsUseUndocumented))
    {
        options.use_undocumented = c_undocumented->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::DiskDirectory))
    {
        const auto text = e_diskMonitorDir->text();
        options.disk_dir =
            fs::u8path(QDir::toNativeSeparators(text).toStdString());
    }

    if (!IsReadOnly(FlexemuOptionId::DiskDirectory))
    {
        const auto text = e_monitorPgm->text();
        options.hex_file =
            fs::u8path(QDir::toNativeSeparators(text).toStdString());
    }

    if (!IsReadOnly(FlexemuOptionId::Drive0))
    {
        const auto text = e_drive0->text();
        options.drives[0] =
            fs::u8path(QDir::toNativeSeparators(text).toStdString());
    }
    if (!IsReadOnly(FlexemuOptionId::Drive1))
    {
        const auto text = e_drive1->text();
        options.drives[1] =
            fs::u8path(QDir::toNativeSeparators(text).toStdString());
    }
    if (!IsReadOnly(FlexemuOptionId::Drive2))
    {
        const auto text = e_drive2->text();
        options.drives[2] =
            fs::u8path(QDir::toNativeSeparators(text).toStdString());
    }
    if (!IsReadOnly(FlexemuOptionId::Drive3))
    {
        const auto text = e_drive3->text();
        options.drives[3] =
            fs::u8path(QDir::toNativeSeparators(text).toStdString());
    }

    if (!IsReadOnly(FlexemuOptionId::CanFormatDrive0))
    {
        options.canFormatDrives[0] = c_canFormatDrive0->isChecked();
    }
    if (!IsReadOnly(FlexemuOptionId::CanFormatDrive1))
    {
        options.canFormatDrives[1] = c_canFormatDrive1->isChecked();
    }
    if (!IsReadOnly(FlexemuOptionId::CanFormatDrive2))
    {
        options.canFormatDrives[2] = c_canFormatDrive2->isChecked();
    }
    if (!IsReadOnly(FlexemuOptionId::CanFormatDrive3))
    {
        options.canFormatDrives[3] = c_canFormatDrive3->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::MdcrDrive0))
    {
        const auto text = e_mdcrDrive0->text();
        options.mdcrDrives[0] =
            fs::u8path(QDir::toNativeSeparators(text).toStdString());
    }
    if (!IsReadOnly(FlexemuOptionId::MdcrDrive1))
    {
        const auto text = e_mdcrDrive1->text();
        options.mdcrDrives[1] =
            fs::u8path(QDir::toNativeSeparators(text).toStdString());
    }

    if (!IsReadOnly(FlexemuOptionId::IsRamExt2x96) &&
        !IsReadOnly(FlexemuOptionId::IsRamExt2x288) &&
        !IsReadOnly(FlexemuOptionId::IsFlexibleMmu))
    {
        auto ramExt = r_ramExt2x96->isChecked();
        auto hiMem = r_ramExt2x288->isChecked();
        options.isRamExtension = ramExt || hiMem;
        options.isHiMem = hiMem;

        if (options.isHiMem)
        {
            auto flexibleMmu =
                c_flexibleMmu->isEnabled() && c_flexibleMmu->isChecked();
            options.isFlexibleMmu = flexibleMmu;
        }
    }

    if (!IsReadOnly(FlexemuOptionId::IsUseRtc))
    {
        auto useRtc = c_useRtc->isEnabled() && c_useRtc->isChecked();
        options.useRtc = useRtc;
    }

    if (!IsReadOnly(FlexemuOptionId::IsEurocom2V5))
    {
        options.isEurocom2V5 = r_eurocom2v5->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::IsUseRtc))
    {
        if (r_frequencyOriginal->isChecked())
        {
                options.frequency = -1.0F;
        }
        else if (r_frequencyFast->isChecked())
        {
                options.frequency = 0.0F;
        }
        else if (r_frequencySet->isChecked())
        {
            // success == false should be prevented by Validate().
            auto frequency =
                QLocale::c().toFloat(e_frequency->text(), &success);
            options.frequency = (success ? frequency : -1.0F);
        }
    }

    if (!IsReadOnly(FlexemuOptionId::FileTimeAccess))
    {
        options.fileTimeAccess = c_fileTime->isChecked() ?
            FileTimeAccess::Get | FileTimeAccess::Set : FileTimeAccess::NONE;
    }

    if (!IsReadOnly(FlexemuOptionId::IsDisplaySmooth))
    {
        options.isSmooth = c_isDisplaySmooth->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::TerminalType))
    {
        options.terminalType = 0;

        if (r_scrollingTerminal->isChecked())
        {
            options.terminalType = 1;
        }
        else if (r_ncursesTerminal->isChecked())
        {
            options.terminalType = 2;
        }
    }


    if (!IsReadOnly(FlexemuOptionId::IsTerminalIgnoreNUL))
    {
        options.isTerminalIgnoreNUL = c_terminalIgnoreNUL->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::IsTerminalIgnoreESC))
    {
        options.isTerminalIgnoreESC = c_terminalIgnoreESC->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::DirectoryDiskTrkSec))
    {
        options.directoryDiskTracks = e_tracks->value();
        options.directoryDiskSectors = e_sectors->value();
    }

    if (!IsReadOnly(FlexemuOptionId::IsDirectoryDiskActive))
    {
        options.isDirectoryDiskActive = c_isDirectoryDiskActive->isChecked();
    }
}

QString FlexemuOptionsUi::GetRelativePath(
        const QString &directory,
        const QString &filePath)
{
    auto result = filePath;

    if (!result.isEmpty() &&
        !directory.isEmpty() &&
        (result.indexOf(directory) == 0) &&
        (result.indexOf(PATHSEPARATOR) >= 0))
    {
        result = result.mid(directory.length(),
                       result.length() - directory.length());

        if (result.indexOf(PATHSEPARATOR) == 0)
        {
            result = result.mid(1, result.length() - 1);
        }
    }

    return result;
}

void FlexemuOptionsUi::OnSelectFile(QLineEdit &lineEdit, FileType type)
{
    QString filter;

    auto diskDir = e_diskMonitorDir->text();
    auto path = diskDir;
    auto fileInfo = QFileInfo(lineEdit.text());

    if (fileInfo.isAbsolute() && fileInfo.isFile())
    {
        path = lineEdit.text();
    }

    switch (type)
    {
        case FileType::FlexDiskFile:
            filter = "*.dsk";
            path = QFileDialog::getOpenFileName(
                dialog, tr("Select a Disk image file"), path,
                tr("FLEX disk image files (*.dsk *.flx *.wta);;"
                   "All files (*.*)"),
                &filter);
            break;

        case FileType::CassetteFile:
            filter = "*.mdcr";
            path = QFileDialog::getOpenFileName(
                dialog, tr("Select a MDCR file"), path,
                tr("MDCR containers (*.mdcr);;All files (*.*)"),
                &filter);
            break;

        case FileType::HexBinaryFile:
            filter = "*.hex";
            path = QFileDialog::getOpenFileName(
                dialog, tr("Select a monitor program"), path,
                tr("All monitor files (*.hex *.s19 *.srec *.mot *.cmd *.bin);;"
                    "Intel HEX files (*.hex);;"
                    "Motorola S-Record files (*.s19 *.srec *.mot);;"
                    "FLEX binary files (*.cmd *.bin);;"
                    "All files (*.*)"),
                &filter);
            break;

        case FileType::FlexDiskDirectory:
            throw FlexException(FERR_WRONG_PARAMETER);
    }

    path = QDir::toNativeSeparators(path);
    path = GetRelativePath(diskDir, path);

    if (!path.isEmpty())
    {
        lineEdit.setText(path);
    }
}

void FlexemuOptionsUi::OnSelectDirectory(QLineEdit &lineEdit, FileType type)
{
    auto diskDir = e_diskMonitorDir->text();
    auto path = diskDir;
    auto fileInfo = QFileInfo(lineEdit.text());

    if (fileInfo.isAbsolute() && fileInfo.isDir())
    {
        path = lineEdit.text();
    }

    switch (type)
    {
        case FileType::FlexDiskDirectory:
            path = QFileDialog::getExistingDirectory(dialog,
                tr("Select a FLEX directory disk"), path);
            break;

        case FileType::FlexDiskFile:
        case FileType::CassetteFile:
        case FileType::HexBinaryFile:
            throw FlexException(FERR_WRONG_PARAMETER);
    }

    path = QDir::toNativeSeparators(path);
    path = GetRelativePath(diskDir, path);

    if (!path.isEmpty())
    {
        lineEdit.setText(path);
    }
}

void FlexemuOptionsUi::OnSelectDiskMonitorDir()
{
    auto new_disk_dir =
             QFileDialog::getExistingDirectory(dialog,
                 tr("Select folder with DSK or monitor files"),
                 e_diskMonitorDir->text(),
                 QFileDialog::ShowDirsOnly);

    if (!new_disk_dir.isEmpty())
    {
        e_diskMonitorDir->setText(new_disk_dir);
    }
}

void FlexemuOptionsUi::UpdateHardwareDependencies()
{
    c_useRtc->setEnabled(r_eurocom2v7->isChecked());

    e_drive0->setEnabled(r_eurocom2v7->isChecked());
    e_drive1->setEnabled(r_eurocom2v7->isChecked());
    e_drive2->setEnabled(r_eurocom2v7->isChecked());
    e_drive3->setEnabled(r_eurocom2v7->isChecked());
    b_drive0->setEnabled(r_eurocom2v7->isChecked());
    b_drive1->setEnabled(r_eurocom2v7->isChecked());
    b_drive2->setEnabled(r_eurocom2v7->isChecked());
    b_drive3->setEnabled(r_eurocom2v7->isChecked());
    b_drive0Dir->setEnabled(r_eurocom2v7->isChecked());
    b_drive1Dir->setEnabled(r_eurocom2v7->isChecked());
    b_drive2Dir->setEnabled(r_eurocom2v7->isChecked());
    b_drive3Dir->setEnabled(r_eurocom2v7->isChecked());

    e_mdcrDrive0->setEnabled(!r_eurocom2v7->isChecked());
    e_mdcrDrive1->setEnabled(!r_eurocom2v7->isChecked());
    b_mdcrDrive0->setEnabled(!r_eurocom2v7->isChecked());
    b_mdcrDrive1->setEnabled(!r_eurocom2v7->isChecked());

    c_canFormatDrive0->setEnabled(r_eurocom2v7->isChecked());
    c_canFormatDrive1->setEnabled(r_eurocom2v7->isChecked());
    c_canFormatDrive2->setEnabled(r_eurocom2v7->isChecked());
    c_canFormatDrive3->setEnabled(r_eurocom2v7->isChecked());

    c_isDirectoryDiskActive->setEnabled(r_eurocom2v7->isChecked());
    cb_diskFormat->setEnabled(r_eurocom2v7->isChecked());

    UpdateRamDependencies();
}

void FlexemuOptionsUi::UpdateRamDependencies()
{
    r_ramExtNone->setEnabled(r_eurocom2v7->isChecked());
    r_ramExt2x96->setEnabled(r_eurocom2v7->isChecked());
    r_ramExt2x288->setEnabled(r_eurocom2v7->isChecked());
    c_flexibleMmu->setEnabled(r_ramExt2x288->isEnabled() &&
                              r_ramExt2x288->isChecked());

    UpdateColorDependencies();
}

void FlexemuOptionsUi::UpdateColorDependencies()
{
    bool canEnableNColors =
        (r_ramExt2x96->isEnabled() && r_ramExt2x96->isChecked()) ||
        (r_ramExt2x288->isEnabled() && r_ramExt2x288->isChecked());

    cb_nColors->setEnabled(canEnableNColors);

    if (canEnableNColors)
    {
        c_multiColorScheme->setEnabled(cb_nColors->currentIndex() != 0);
        bool color_enabled = !(c_multiColorScheme->isEnabled() &&
                             c_multiColorScheme->isChecked());
        cb_color->setEnabled(color_enabled);
    }
    else
    {
        c_multiColorScheme->setEnabled(false);
        cb_color->setEnabled(true);
    }
}

void FlexemuOptionsUi::OnFrequencyOriginal(bool value)
{
    if (value)
    {
        bool ok;
        auto text = QString::fromStdString(
                    flx::toString<float>(ORIGINAL_FREQUENCY, ok));

        e_frequency->setText(text);
    }
}

void FlexemuOptionsUi::OnFrequencyFast(bool value)
{
    if (value)
    {
        e_frequency->setText("");
    }
}

void FlexemuOptionsUi::OnFrequencySet(bool value)
{
    e_frequency->setReadOnly(!value);
    e_frequency->setEnabled(value);
}

void FlexemuOptionsUi::OnMultiColorSchemeChanged(int /* state */)
{
    UpdateColorDependencies();
}

void FlexemuOptionsUi::OnNColorsChanged(int /* index */)
{
    UpdateColorDependencies();
}

void FlexemuOptionsUi::AddFrequencyValidator(QLineEdit &lineEdit)
{
    auto *validator = new QDoubleValidator(0.1, 10000.0, 6, &lineEdit);

    validator->setLocale(QLocale::c());
    validator->setNotation(QDoubleValidator::StandardNotation);
    lineEdit.setValidator(validator);
}

QUrl FlexemuOptionsUi::CreateDocumentationUrl(
        const fs::path &doc_dir, const fs::path &html_file)
{
    const auto path = doc_dir / html_file;

    return QUrl::fromLocalFile(QString::fromStdString(path.u8string()));
}

QString FlexemuOptionsUi::CreateHref(const QString &encoded_url,
                                     const QString &description)
{
    return QString(u8"<a href=\"") + encoded_url +
           u8"\">" + description + u8"</a>";
}

void FlexemuOptionsUi::InitializeHardwareHyperlink(const fs::path &doc_dir)
{
    auto url = CreateDocumentationUrl(doc_dir, fs::u8path(u8"e2hwdesc.htm"));
    auto href = CreateHref(url.toString(), u8"Eurocom II hardware description");

    l_hardwareLink->setOpenExternalLinks(true);
    l_hardwareLink->setTextFormat(Qt::RichText);
    l_hardwareLink->setText(href);
}

void FlexemuOptionsUi::OnAccepted()
{
    if(Validate())
    {
        dialog->done(QDialog::Accepted);
    }
}

void FlexemuOptionsUi::OnRejected()
{
    dialog->done(QDialog::Rejected);
}

void FlexemuOptionsUi::SetTabIndex(int index)
{
    if (c_tabWidget == nullptr)
    {
        QMessageBox::critical(nullptr, PROGRAMNAME " Logic Error",
                              "setupUi(dialog) with a valid dialog instance "
                              "has to be called before.");
        return;
    }

    c_tabWidget->setCurrentIndex(index);
}

int FlexemuOptionsUi::GetTabIndex() const
{
    if (c_tabWidget == nullptr)
    {
        QMessageBox::critical(nullptr, PROGRAMNAME " Logic Error",
                              "setupUi(dialog) with a valid dialog instance "
                              "has to be called before.");
        return 0;
    }

    return c_tabWidget->currentIndex();
}

void FlexemuOptionsUi::OnFormatChanged(int index)
{
    bool isFreeDiskFormat = (index == 0);

    if (!isFreeDiskFormat)
    {
        auto trk_sec = flex_formats[index - 1];
        e_tracks->setValue(trk_sec.trk + 1);
        e_sectors->setValue(trk_sec.sec);
    }

    e_tracks->setEnabled(isFreeDiskFormat);
    e_sectors->setEnabled(isFreeDiskFormat);
}

void FlexemuOptionsUi::OnSectorChanged(int sectors)
{
    OnTrkSecChanged(e_tracks->value(), sectors);
}

void FlexemuOptionsUi::OnTrackChanged(int tracks)
{
    OnTrkSecChanged(tracks, e_sectors->value());
}

void FlexemuOptionsUi::OnTrkSecChanged(int tracks, int sectors)
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

    static const auto msg = tr("Uncommon FLEX disk format. "
                               "This may cause compatibility issues!");
    auto richText = QString("<p style=\"color:orange\">%1</p>").arg(msg);
    l_formatWarning->setText(isWarning ? richText : "");
}

void FlexemuOptionsUi::OnDirectoryDiskActiveChanged(bool isActive)
{
    cb_diskFormat->setEnabled(isActive);

    if (isActive)
    {
        OnFormatChanged(cb_diskFormat->currentIndex());
    }
    else
    {
        e_tracks->setEnabled(isActive);
        e_sectors->setEnabled(isActive);
    }
}

void FlexemuOptionsUi::OnTerminalTypeChanged(TerminalType type)
{
    switch (type)
    {
        case TerminalType::Scrolling:
            c_terminalIgnoreESC->setEnabled(true);
            break;

        case TerminalType::NCurses:
            c_terminalIgnoreESC->setEnabled(false);
            break;

        case TerminalType::Dummy:
            break;
    }
}

void FlexemuOptionsUi::InitializeUi()
{
    const auto openIcon = QIcon(":/resource/open_con.png");
    const auto openDirIcon = QIcon(":/resource/open_dir.png");
    const auto disk = QString("Select Disk Image");
    const auto diskDir = QString("Select Directory Disk");

    b_drive0->setIcon(openIcon);
    b_drive1->setIcon(openIcon);
    b_drive2->setIcon(openIcon);
    b_drive3->setIcon(openIcon);

    b_drive0->setToolTip(disk);
    b_drive1->setToolTip(disk);
    b_drive2->setToolTip(disk);
    b_drive3->setToolTip(disk);

    b_drive0Dir->setIcon(openDirIcon);
    b_drive1Dir->setIcon(openDirIcon);
    b_drive2Dir->setIcon(openDirIcon);
    b_drive3Dir->setIcon(openDirIcon);

    b_drive0Dir->setToolTip(diskDir);
    b_drive1Dir->setToolTip(diskDir);
    b_drive2Dir->setToolTip(diskDir);
    b_drive3Dir->setToolTip(diskDir);
}
