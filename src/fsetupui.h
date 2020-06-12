/*
    fsetupui.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2020  W. Schwotzer

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
#include "fsetup_ui.h"
#include <string>
#include <QObject>
#include <QString>
#include <QUrl>


class QLineEdit;
struct sGuiOptions;
struct sOptions;

class FlexemuOptionsUi : public QObject, protected Ui_FlexemuSetup
{
    Q_OBJECT

public:

    FlexemuOptionsUi();
    virtual ~FlexemuOptionsUi();
    void setupUi(QDialog *dialog);
    void TransferDataToDialog(
            const sGuiOptions &guiOptions,
            const sOptions &options);
    void TransferDataFromDialog(sGuiOptions &guiOptions, sOptions &options);

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

private:
    enum class FileType
    {
        DiskContainerFile,
        CassetteFile,
        HexBinaryFile,
    };

    void ConnectSignalsWithSlots();
    void AddFrequencyValidator(QLineEdit &lineEdit);
    void InitializeHardwareHyperlink(const char *doc_dir);
    bool Validate();

    void OnSelectFile(QLineEdit &lineEdit, FileType type);

    static QUrl CreateDocumentationUrl(const char *doc_dir,
                                       const char *html_file);
    static std::string CreateHref(const char *encoded_url,
                                  const char *description);
    static QString GetRelativePath(
            const QString &diskDir,
            const QString &filePath);

    QDialog *dialog;
};

#endif

