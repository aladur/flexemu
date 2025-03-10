/*
    fsetupui.h


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


#ifndef FSETUPUI_INCLUDED
#define FSETUPUI_INCLUDED


#include "misc1.h"
#include "brcfile.h"
#include "bregistr.h"
#include "soptions.h"
#include "termimpf.h"
#include <string>
#include <vector>
#include "warnoff.h"
#ifdef USE_CMAKE
#include "ui_fsetup.h"
#else
#include "fsetup_ui.h"
#endif
#include <QObject>
#include <QString>
#include <QUrl>
#include <QLocale>
#include "warnon.h"
#include <filesystem>

namespace fs = std::filesystem;


class QLineEdit;

class FlexemuOptionsUi : public QObject, protected Ui_FlexemuSetup
{
    Q_OBJECT

public:

    FlexemuOptionsUi();
    ~FlexemuOptionsUi() override = default;
    void setupUi(QDialog *dialog);
    void TransferDataToDialog(const sOptions &options);
    void TransferDataFromDialog(sOptions &options);
    void SetTabIndex(int index);
    int GetTabIndex() const;

private slots:
    void OnFrequencyOriginal(bool value);
    void OnFrequencyFast(bool value);
    void OnFrequencySet(bool value);
    void UpdateHardwareDependencies();
    void UpdateRamDependencies();
    void UpdateColorDependencies();
    void OnMultiColorSchemeChanged(int state);
    void OnNColorsChanged(int index);
    void OnSelectDiskMonitorDir();
    void OnAccepted();
    void OnRejected();
    void OnFormatChanged(int index);
    void OnTrackChanged(int tracks);
    void OnSectorChanged(int sectors);
    void OnTrkSecChanged(int tracks, int sectors);
    void OnDirectoryDiskActiveChanged(bool isActive);
    void OnTerminalTypeChanged(TerminalType type);

private:
    enum class FileType : uint8_t
    {
        FlexDiskFile,
        CassetteFile,
        HexBinaryFile,
    };

    void ConnectSignalsWithSlots();
    static void AddFrequencyValidator(QLineEdit &lineEdit);
    void InitializeHardwareHyperlink(const fs::path &doc_dir);
    bool Validate();
    bool IsReadOnly(FlexemuOptionId optionId);
    void SetOptionsReadOnly(const std::vector<FlexemuOptionId> &readOnlyOptions);
    static std::vector<FlexemuOptionId> AddDependentReadOnlyOptions(
            const std::vector<FlexemuOptionId> &readOnlyOptions);

    void OnSelectFile(QLineEdit &lineEdit, FileType type);

    static QUrl CreateDocumentationUrl(const fs::path &doc_dir,
                                       const fs::path &html_file);
    static std::string CreateHref(const char *encoded_url,
                                  const char *description);
    static QString GetRelativePath(
            const QString &directory,
            const QString &filePath);

    QDialog *dialog{nullptr};
    std::vector<FlexemuOptionId> readOnlyOptions;
};

#endif

