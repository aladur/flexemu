/*
    fpnewui.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
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


#ifndef FPNEWUI_INCLUDED
#define FPNEWUI_INCLUDED


#include "misc1.h"
#include "filecntb.h"
#include "warnoff.h"
#include "fpnew_ui.h"
#include <QObject>
#include <QString>
#include "warnon.h"

class QLineEdit;

class FlexplorerNewUi : public QObject, protected Ui_FlexplorerNew
{
    Q_OBJECT

public:

    FlexplorerNewUi();
    ~FlexplorerNewUi() override = default;
    void setupUi(QDialog &dialog);
    void TransferDataToDialog(int format = TYPE_DSK_DISKFILE,
                              int tracks = 80, int sectors = 36,
                              const QString &path = "");

    int GetTracks() const;
    int GetSectors() const;
    QString GetPath() const;
    int GetFormat() const;
    void SetDefaultPath(const QString &path);
    QString GetDefaultPath() const;

private slots:
    void OnDskFileFormat(bool value);
    void OnFlxFileFormat(bool value);
    void OnMdcrFileFormat(bool value);
    void OnFormatChanged(int index);
    void OnTrkSecChanged(int tracks, int sectors);
    void OnTrackChanged(int tracks);
    void OnSectorChanged(int sectors);
    void OnSelectPath();
    void OnAccepted();
    void OnRejected();

private:
    void InitializeWidgets();
    void ConnectSignalsWithSlots();
    void UpdateFormatTrkSecEnable(bool isMdcrFormat);
    void UpdateFilename();
    bool Validate();
    QString GetCurrentFileExtension() const;

    QDialog *dialog{nullptr};
    QString defaultPath;
    int format{TYPE_DSK_DISKFILE};
};

#endif

