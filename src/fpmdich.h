/*
    fpmdich.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2020-2025  W. Schwotzer

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

#ifndef FPMDICH_INCLUDED
#define FPMDICH_INCLUDED

#include "misc1.h"
#include "efiletim.h"
#include "warnoff.h"
#include <QPair>
#include <QPoint>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QTableView>
#include "warnon.h"
#include "filecntb.h"
#include "fpmodel.h"
#include "fpedit.h"
#include <memory>

class FlexDnDFiles;
class FlexDirEntry;
class QItemSelection;
class QItemSelectionModel;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QMouseEvent;
class QMimeData;


class FlexplorerMdiChild : public QTableView
{
    Q_OBJECT

public:
    FlexplorerMdiChild(const QString &path, struct sFPOptions &options);
    FlexplorerMdiChild() = delete;
    ~FlexplorerMdiChild() override = default;

    QString GetPath() const;
    QString GetUserFriendlyPath() const;
    bool IsWriteProtected() const;
    int GetSelectedFilesCount() const;
    unsigned GetSelectedFilesByteSize() const;
    DiskType GetFlexDiskType() const;
    void SelectAll();
    void DeselectAll();
    QVector<int>::size_type FindFiles(const QString &pattern);
    QVector<int>::size_type DeleteSelected();
    QVector<int>::size_type InjectFiles(const QStringList &filePaths);
    QVector<int>::size_type ExtractSelected(const QString &targetDirectory);
    QVector<int>::size_type ViewSelected();
    QStringList GetSelectedFilenames() const;
    QString GetSupportedAttributes() const;
    QVector<Byte> GetSelectedAttributes() const;
    int SetSelectedAttributes(Byte setMask, Byte clearMask);
    void Info();
#ifndef QT_NO_CLIPBOARD
    int CopySelectedToClipboard();
    int PasteFromClipboard();
#endif

signals:
    void SelectionHasChanged();

public slots:
    void OnFileTimeAccessChanged();
    void OnFileSizeTypeHasChanged();

private slots:
    void IsActivated(const QModelIndex &index);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void SetupModel(const QString &path);
    void SetupView();
    // Intentionally do not override QAbstractItemView::selectionChanged().
    // NOLINTNEXTLINE(bugprone-virtual-near-miss)
    void SelectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected);
    void BeginDrag();
    int PasteFrom(const QMimeData &mimeData);
    QMimeData *GetMimeDataForSelected(int *count = nullptr);
    QMimeData *GetHtmlMimeDataForSelected();
    void UpdateDateDelegate();
    void MultiSelect(const QVector<int> &rowIndices);
    void ResizeColumn(int column, const QString &text) const;
    void ResizeColumns() const;

    static const QString &GetMimeTypeFlexDiskImageFile();

    std::unique_ptr<FlexplorerTableModel> model;
    std::unique_ptr<FlexDateDelegate> dateDelegate;
    std::unique_ptr<FlexDateTimeDelegate> dateTimeDelegate;
    std::unique_ptr<FlexFilenameDelegate> filenameDelegate;
    std::unique_ptr<FlexAttributesDelegate> attributesDelegate;
    QPoint dragStartPosition;
    int selectedFilesCount;
    int selectedFilesByteSize;
    struct sFPOptions &options;
};

#endif

