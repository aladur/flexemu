/*
    fpwin.h


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

#ifndef FPWIN_INCLUDED
#define FPWIN_INCLUDED

#include "e2.h"
#include "efiletim.h"
#include "warnoff.h"
#include <QPoint>
#include <QSize>
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QList>
#include "warnon.h"
#include "sfpopts.h" // After qt include to avoid automoc issue
#include <functional>


class FlexplorerMdiChild;
class QAction;
class QEvent;
class QWidget;
class QToolBar;
class QLabel;
class QMenu;
class QMdiArea;
class QMimeData;
class QMdiSubWindow;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

/*------------------------------------------------------
 FLEXplorer
 The main window class
 An explorer for any FLEX file or disk container
--------------------------------------------------------*/
class FLEXplorer : public QMainWindow
{
    Q_OBJECT

public:
    FLEXplorer() = delete;
    explicit FLEXplorer(struct sFPOptions &options);
    ~FLEXplorer() override;
    FLEXplorer(const FLEXplorer &src) = delete;
    FLEXplorer(FLEXplorer &&src) = delete;
    FLEXplorer &operator=(const FLEXplorer &src) = delete;
    FLEXplorer &operator=(FLEXplorer &&src) = delete;

    void ProcessArguments(const QStringList &args);

signals:
    void FileTimeAccessHasChanged();
    void FileSizeTypeHasChanged();

private slots:
    void OnNewFlexDisk();
    void OnOpenFlexDisk();
    void OnOpenDirectory();
    void OnOpenRecentDisk();
    void OnOpenRecentDirectory();
    void OnClearAllRecentDiskEntries();
    void OnClearAllRecentDirectoryEntries();
#ifndef QT_NO_CLIPBOARD
    void OnCopy();
    void OnPaste();
#endif
    void OnSelectAll();
    void OnDeselectAll();
    void OnFindFiles();
    void OnDeleteSelected();
    void OnInjectFiles();
    void OnExtractSelected();
    void OnViewSelected();
    void OnAttributesSelected();
    void OnInfo();
    void OnOptions();
    void OnSubWindowActivated(QMdiSubWindow *window);
    void OnContextMenuRequested(QPoint pos);
    void OnUpdateWindowMenu();
    void OnCloseActiveSubWindow();
    void OnCloseAllSubWindows();
    void OnAbout();
    void OnSelectionHasChanged();
    void OnIconSize(int index);

    static void OnExit();

private:
    void ExecuteInChild(
            const std::function<void(FlexplorerMdiChild &child)>&action);
    QToolBar *CreateToolBar(QWidget *parent, const QString &title,
                            const QString &objectName, const QSize &iconSize);
    void CreateActions(const QSize &iconSize);
    void CreateFileActions(QToolBar &p_toolBar);
    void CreateEditActions(QToolBar &p_toolBar);
    void CreateViewActions(QToolBar &p_toolBar);
    void CreateFlexDiskActions(QToolBar &p_toolBar);
    void CreateExtrasActions(QToolBar &p_toolBar);
    void CreateWindowsActions(QToolBar &p_toolBar);
    void CreateHelpActions(QToolBar &p_toolBar);
    void CreateStatusBar();
    FlexplorerMdiChild *CreateMdiChild(const QString &path,
                                       struct sFPOptions &options);
    FlexplorerMdiChild *ActiveMdiChild() const;
    QMdiSubWindow *FindMdiChild(const QString &path) const;
    void changeEvent(QEvent *event) override;
    void UpdateSelectedFiles();
    void UpdateMenus();
    static QStringList GetSupportedFiles(const QMimeData *mimeData);
    void SetFileTimeAccess(FileTimeAccess fileTimeAccess);
    void CreateRecentDiskActionsFor(QMenu *menu);
    void UpdateRecentDiskActions() const;
    void DeleteRecentDiskActions();
    void UpdateForRecentDisk(const QString &path);
    void RestoreRecentDisks();
    void CreateRecentDirectoryActionsFor(QMenu *menu);
    void UpdateRecentDirectoryActions() const;
    void DeleteRecentDirectoryActions();
    void UpdateForRecentDirectory(const QString &path);
    void RestoreRecentDirectories();
    bool OpenFlexDiskForPath(QString path, bool isLast = true);
    void SetStatusMessage(const QString &message);
    void SetIconSize(const QSize &iconSize);
    void SetIconSizeCheck(const QSize &iconSize);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool event(QEvent *event) override;


    QMdiArea *mdiArea{};
    QMenu *windowMenu{};
    QMenu *recentDisksMenu{};
    QList<QAction *> recentDiskActions;
    QAction *recentDisksClearAllAction{};
    QMenu *recentDirectoriesMenu{};
    QAction *recentDirectoriesClearAllAction{};
    QList<QAction *> recentDirectoryActions;
    QStatusBar *l_statusMessage{};
    QLabel *l_selectedFilesCount{};
    QLabel *l_selectedFilesByteSize{};
    QToolBar *toolBar{};
    QAction *newFlexDiskAction{};
    QAction *openFlexDiskAction{};
    QAction *openDirectoryAction{};
    QAction *injectAction{};
    QAction *extractAction{};
    QAction *selectAllAction{};
    QAction *deselectAllAction{};
    QAction *findFilesAction{};
#ifndef QT_NO_CLIPBOARD
    QAction *copyAction{};
    QAction *pasteAction{};
#endif
    QAction *deleteAction{};
    QAction *viewAction{};
    QAction *attributesAction{};
    QAction *infoAction{};
    QAction *optionsAction{};
    QAction *closeAction{};
    QAction *closeAllAction{};
    QAction *tileAction{};
    QAction *cascadeAction{};
    QAction *nextAction{};
    QAction *previousAction{};
    QAction *windowMenuSeparatorAction{};
    QAction *aboutAction{};
    QAction *aboutQtAction{};
    std::array<QAction *, ICON_SIZES> iconSizeAction{};
    QSize newDialogSize;
    QSize optionsDialogSize;
    QSize attributesDialogSize;
    QString findPattern{"*.*"};
    QString injectDirectory;
    QString extractDirectory;
    QStringList recentDiskPaths;
    QStringList recentDirectoryPaths;
    sFPOptions &options;
};

class ProcessArgumentsFtor
{
    // Intentionally use a reference.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    FLEXplorer &window;
    QStringList args;

public:
    ProcessArgumentsFtor() = delete;
    /* Parameter comes from main(). */
    /* NOLINTNEXTLINE(modernize-avoid-c-arrays) */
    ProcessArgumentsFtor(FLEXplorer &win, int p_argc, char *p_argv[])
    : window(win)
    {
        for (int i = 0; i < p_argc; ++i)
        {
            args.push_back(p_argv[i]);
        }
    }
    ProcessArgumentsFtor(const ProcessArgumentsFtor &src) = default;
    ProcessArgumentsFtor(ProcessArgumentsFtor &&src) noexcept
        : window(src.window)
        , args(std::move(src.args))
    {
    }
    ProcessArgumentsFtor &operator=(const ProcessArgumentsFtor &src) = delete;
    ProcessArgumentsFtor &operator=(ProcessArgumentsFtor &&src) = delete;

    void operator() ()
    {
        window.ProcessArguments(args);
    }
};

#endif
