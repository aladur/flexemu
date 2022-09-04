/*
    fsetupui.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2022  W. Schwotzer

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
#include <string>
#include <memory>
#include <stdexcept>
#include "warnoff.h"
#include <QtGlobal>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontMetrics>
#include "warnon.h"

#ifdef _MSC_VER
    #include <direct.h>
#endif

#include "fsetupui.h"


void FlexemuOptionsUi::ConnectSignalsWithSlots()
{
    QObject::connect(b_diskMonitorDir, &QAbstractButton::clicked,
            this, &FlexemuOptionsUi::OnSelectDiskMonitorDir);
    QObject::connect(b_monitorPgm, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_monitorPgm, FileType::HexBinaryFile); });

    QObject::connect(b_drive0, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_drive0, FileType::DiskContainerFile); });
    QObject::connect(b_drive1, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_drive1, FileType::DiskContainerFile); });
    QObject::connect(b_drive2, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_drive2, FileType::DiskContainerFile); });
    QObject::connect(b_drive3, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_drive3, FileType::DiskContainerFile); });

    QObject::connect(b_mdcrDrive0, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_mdcrDrive0, FileType::CassetteFile); });
    QObject::connect(b_mdcrDrive1, &QAbstractButton::clicked,
            [&](){ OnSelectFile(*e_mdcrDrive1, FileType::CassetteFile); });

    QObject::connect(r_ramExtNone, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateRamDependencies);
    QObject::connect(r_ramExt2x96, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateRamDependencies);
    QObject::connect(r_ramExt2x288, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateRamDependencies);

    QObject::connect(r_eurocom2v5, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateHardwareDependencies);
    QObject::connect(r_eurocom2v7, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::UpdateHardwareDependencies);

    QObject::connect(r_frequencyOriginal, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::OnFrequencyOriginal);
    QObject::connect(r_frequencyFast, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::OnFrequencyFast);
    QObject::connect(r_frequencySet, &QAbstractButton::toggled,
            this, &FlexemuOptionsUi::OnFrequencySet);

    QObject::connect(c_multiColorScheme, &QCheckBox::stateChanged,
            this, &FlexemuOptionsUi::OnMultiColorSchemeChanged);
    QObject::connect(cb_nColors,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
                     static_cast<void (QComboBox::*)(int)>(
                         &QComboBox::currentIndexChanged),
#else
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
#endif
            this, &FlexemuOptionsUi::OnNColorsChanged);

    QObject::connect(c_buttonBox, &QDialogButtonBox::accepted,
            this, &FlexemuOptionsUi::OnAccepted);
    QObject::connect(c_buttonBox, &QDialogButtonBox::rejected,
            this, &FlexemuOptionsUi::OnRejected);
}

FlexemuOptionsUi::FlexemuOptionsUi()
    : Ui_FlexemuSetup()
    , dialog(nullptr)
    , englishUS(QLocale::English, QLocale::UnitedStates)
{
}

FlexemuOptionsUi::~FlexemuOptionsUi()
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

    InitializeHardwareHyperlink(options.doc_dir.c_str());

    for (int x = 1; x <= MAX_PIXELSIZE; ++x)
    {
        const auto iconPath =
            QString::asprintf(":/resource/screen%u.png", x);
        const auto screenSizeIcon = QIcon(iconPath);

        auto text = QString::asprintf("x%u", x);

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
    static int ncolor_count[] = { 2, 8, 64 };
    for (auto nColors : ncolor_count)
    {
        auto text = QString::asprintf("%d", nColors);
        cb_nColors->addItem(text);

        if (options.nColors == nColors)
        {
            index = n;
        }
        ++n;
    }
    cb_nColors->setCurrentIndex(std::max(index, 0));

    index = -1;
    for (int i = 0; i < static_cast<int>(color_count); i++)
    {
        DWord colorRGBValue;

        auto colorName = colors[i].colorName;
        getColorForName(colors[i].colorName, &colorRGBValue);
        cb_color->addItem(tr(colorName));
        QPixmap pixmap(16,16);
        pixmap.fill(QColor(colors[i].colorName));
        QIcon colorIcon(pixmap);
        cb_color->setItemIcon(i, colorIcon);

        if (!stricmp(options.color.c_str(), colors[i].colorName))
        {
            index = i;
        }
    }
    cb_color->setMaxVisibleItems(cb_color->count());
    cb_color->setCurrentIndex(std::max(index, 0));

    bool isMultiColorSchemeChecked =
	    (0 == stricmp(options.color.c_str(), "default"));

    c_multiColorScheme->setChecked(isMultiColorSchemeChecked);

    c_isInverse->setChecked(options.isInverse != 0);

    c_undocumented->setChecked(options.use_undocumented);

    e_monitorPgm->setText(QString(options.hex_file.c_str()));
    e_diskMonitorDir->setText(QString(options.disk_dir.c_str()));

    e_drive0->setText(QString(options.drive[0].c_str()));
    e_drive1->setText(QString(options.drive[1].c_str()));
    e_drive2->setText(QString(options.drive[2].c_str()));
    e_drive3->setText(QString(options.drive[3].c_str()));

    e_mdcrDrive0->setText(QString(options.mdcrDrives[0].c_str()));
    e_mdcrDrive1->setText(QString(options.mdcrDrives[1].c_str()));

    c_canFormatDrive0->setChecked(options.canFormatDrive[0]);
    c_canFormatDrive1->setChecked(options.canFormatDrive[1]);
    c_canFormatDrive2->setChecked(options.canFormatDrive[2]);
    c_canFormatDrive3->setChecked(options.canFormatDrive[3]);

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
    if (options.frequency < 0.0f)
    {
        r_frequencyOriginal->setChecked(true);
    }
    else if (options.frequency == 0.0f)
    {
        r_frequencyFast->setChecked(true);
    }
    else
    {
        QString frequency_string = englishUS.toString(options.frequency);

        r_frequencySet->setChecked(true);
        e_frequency->setText(frequency_string);
    }

    auto setFileTime = (options.fileTimeAccess ==
           (FileTimeAccess::Get | FileTimeAccess::Set));
    c_fileTime->setChecked(setFileTime);

    c_isDisplaySmooth->setChecked(options.isSmooth != 0);

    c_terminalIgnoreNUL->setChecked(options.isTerminalIgnoreNUL);
    c_terminalIgnoreESC->setChecked(options.isTerminalIgnoreESC);

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
                break;

            case FlexemuOptionId::Drive1:
                e_drive1->setEnabled(false);
                b_drive1->setEnabled(false);
                break;

            case FlexemuOptionId::Drive2:
                e_drive2->setEnabled(false);
                b_drive2->setEnabled(false);
                break;

            case FlexemuOptionId::Drive3:
                e_drive3->setEnabled(false);
                b_drive3->setEnabled(false);
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

    do {
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

                case FlexemuOptionId::DiskDirectory:
                case FlexemuOptionId::HexFile:
                case FlexemuOptionId::IsEurocom2V5:
                case FlexemuOptionId::IsUseUndocumented:
                case FlexemuOptionId::Frequency:
                case FlexemuOptionId::IsInverse:
                case FlexemuOptionId::PixelSize:
                case FlexemuOptionId::FileTimeAccess:
                case FlexemuOptionId::IsDisplaySmooth:
                case FlexemuOptionId::IsTerminalIgnoreESC:
                case FlexemuOptionId::IsTerminalIgnoreNUL:
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
    AddFrequencyValidator(*e_frequency);
    ConnectSignalsWithSlots();
    // This dialog width calculation tries to make all available tabs
    // visible. It is just a rough estimation.
    QFontMetrics metrics(dialog->font());
    e_drive2->setMinimumWidth(metrics.boundingRect('x').width() * 52);
#ifdef _WIN32
    c_tabWidget->removeTab(4); // On Windows there is no terminal mode
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
        auto validator =
            static_cast<const QDoubleValidator *>(e_frequency->validator());

        auto message = QString::asprintf(
                              "CPU Frequency is not in the valid\n"
                              "range of %.1f ... %.1f MHz",
                              validator->bottom(), validator->top());
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
        auto n = cb_nColors->currentText().toUInt(&success);
        options.nColors = (success ? n : 2);

        if (c_multiColorScheme->isChecked() && cb_nColors->currentIndex() > 0)
        {
             options.color = "default";
        }
        else
        {
            options.color = cb_color->currentText().toStdString();

            for (size_t i = 0; i < color_count; i++)
            {
                auto colorName = tr(colors[i].colorName);

                if (colorName == cb_color->currentText())
                {
                    options.color = colors[i].colorName;
                }
            }
        }
    }

    if (!IsReadOnly(FlexemuOptionId::PixelSize))
    {
        options.isInverse = c_isInverse->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::IsUseUndocumented))
    {
        options.use_undocumented = c_undocumented->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::DiskDirectory))
    {
        options.disk_dir = e_diskMonitorDir->text().toStdString();
    }

    if (!IsReadOnly(FlexemuOptionId::DiskDirectory))
    {
        options.hex_file = e_monitorPgm->text().toStdString();
    }

    if (!IsReadOnly(FlexemuOptionId::Drive0))
    {
        options.drive[0] = e_drive0->text().toStdString();
    }
    if (!IsReadOnly(FlexemuOptionId::Drive1))
    {
        options.drive[1] = e_drive1->text().toStdString();
    }
    if (!IsReadOnly(FlexemuOptionId::Drive2))
    {
        options.drive[2] = e_drive2->text().toStdString();
    }
    if (!IsReadOnly(FlexemuOptionId::Drive3))
    {
        options.drive[3] = e_drive3->text().toStdString();
    }

    if (!IsReadOnly(FlexemuOptionId::CanFormatDrive0))
    {
        options.canFormatDrive[0] = c_canFormatDrive0->isChecked();
    }
    if (!IsReadOnly(FlexemuOptionId::CanFormatDrive1))
    {
        options.canFormatDrive[1] = c_canFormatDrive1->isChecked();
    }
    if (!IsReadOnly(FlexemuOptionId::CanFormatDrive2))
    {
        options.canFormatDrive[2] = c_canFormatDrive2->isChecked();
    }
    if (!IsReadOnly(FlexemuOptionId::CanFormatDrive3))
    {
        options.canFormatDrive[3] = c_canFormatDrive3->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::MdcrDrive0))
    {
        options.mdcrDrives[0] = e_mdcrDrive0->text().toStdString();
    }
    if (!IsReadOnly(FlexemuOptionId::MdcrDrive1))
    {
        options.mdcrDrives[1] = e_mdcrDrive1->text().toStdString();
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
                options.frequency = -1.0f;
        }
        else if (r_frequencyFast->isChecked())
        {
                options.frequency = 0.0f;
        }
        else if (r_frequencySet->isChecked())
        {
            // success == false should be prevented by Validate().
            auto frequency = englishUS.toFloat(e_frequency->text(), &success);
            options.frequency = (success ? frequency : -1.0f);
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

    if (!IsReadOnly(FlexemuOptionId::IsTerminalIgnoreNUL))
    {
        options.isTerminalIgnoreNUL = c_terminalIgnoreNUL->isChecked();
    }

    if (!IsReadOnly(FlexemuOptionId::IsTerminalIgnoreESC))
    {
        options.isTerminalIgnoreESC = c_terminalIgnoreESC->isChecked();
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

    if (fileInfo.isAbsolute() && QFile::exists(lineEdit.text()))
    {
        path = lineEdit.text();
    }

    switch (type)
    {
        case FileType::DiskContainerFile:
            filter = "*.dsk";
            path = QFileDialog::getOpenFileName(
                dialog, tr("Select a Disk file"), path,
                tr("FLEX file containers (*.dsk *.flx *.wta);;All files (*.*)"),
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
                tr("Intel HEX files (*.hex);;"
                    "Motorola S-Record files (*.s19 *.srec *.mot);;"
                    "FLEX binary files (*.cmd *.bin);;"
                    "All files (*.*)"),
                &filter);
            break;
    }

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

    e_mdcrDrive0->setEnabled(!r_eurocom2v7->isChecked());
    e_mdcrDrive1->setEnabled(!r_eurocom2v7->isChecked());
    b_mdcrDrive0->setEnabled(!r_eurocom2v7->isChecked());
    b_mdcrDrive1->setEnabled(!r_eurocom2v7->isChecked());

    c_canFormatDrive0->setEnabled(r_eurocom2v7->isChecked());
    c_canFormatDrive1->setEnabled(r_eurocom2v7->isChecked());
    c_canFormatDrive2->setEnabled(r_eurocom2v7->isChecked());
    c_canFormatDrive3->setEnabled(r_eurocom2v7->isChecked());

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
        auto text = QString(std::to_string(ORIGINAL_FREQUENCY).c_str());

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
    auto validator = new QDoubleValidator(0.1, 10000.0, 6, &lineEdit);

    validator->setLocale(englishUS);
    validator->setNotation(QDoubleValidator::StandardNotation);
    lineEdit.setValidator(validator);
}

QUrl FlexemuOptionsUi::CreateDocumentationUrl(
        const char *doc_dir, const char *html_file)
{
    auto path = QString(doc_dir) + PATHSEPARATORSTRING + html_file;

    return QUrl::fromLocalFile(path);
}

std::string FlexemuOptionsUi::CreateHref(const char *encoded_url,
                                         const char *description)
{
    return std::string("<a href=\"") + encoded_url +
           "\">" + description + "</a>";
}

void FlexemuOptionsUi::InitializeHardwareHyperlink(const char *doc_dir)
{
    auto url = CreateDocumentationUrl(doc_dir, "e2hwdesc.htm");
    auto href = CreateHref(url.toEncoded().data(),
                           "Eurocom II hardware description");

    l_hardwareLink->setOpenExternalLinks(true);
    l_hardwareLink->setTextFormat(Qt::RichText);
    l_hardwareLink->setText(QString(href.c_str()));
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

