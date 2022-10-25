/*
    fpwin.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2020-2022  W. Schwotzer

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

#include "misc1.h"
#include "efiletim.h"
#include "warnoff.h"
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include "warnon.h"
#include <functional>
#include "sfpopts.h"


class FlexplorerMdiChild;
class QAction;
class QLabel;
class QMenu;
class QMdiArea;
class QMimeData;
class QMdiSubWindow;

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
    FLEXplorer(struct sFPOptions &options);

    bool OpenContainerForPath(QString path, bool isLast = true);

signals:
    void FileTimeAccessHasChanged();

private slots:
    void NewContainer();
    void OpenContainer();
    void OpenDirectory();
    void Exit();
#ifndef QT_NO_CLIPBOARD
    void Copy();
    void Paste();
#endif
    void SelectAll();
    void DeselectAll();
    void FindFiles();
    void DeleteSelected();
    void InjectFiles();
    void ExtractSelected();
    void ViewSelected();
    void AttributesSelected();
    void Info();
    void Options();
    void SubWindowActivated(QMdiSubWindow *window);
    void ContextMenuRequested(QPoint pos);
    void UpdateWindowMenu();
    void CloseActiveSubWindow();
    void CloseAllSubWindows();
    void About();

public slots:
    void SelectionHasChanged();
    void SetStatusMessage(const QString &message);

private:
    void ExecuteInChild(std::function<void(FlexplorerMdiChild &child)>);
    QToolBar *CreateToolBar(QWidget *parent, const QString &title,
                            const QString &objectName);
    void CreateActions();
    void CreateFileActions();
    void CreateEditActions();
    void CreateContainerActions();
    void CreateExtrasActions();
    void CreateWindowsActions();
    void CreateHelpActions();
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

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;


    QMdiArea *mdiArea;
    QMenu *windowMenu;
    QLabel *l_selectedFilesCount;
    QLabel *l_selectedFilesByteSize;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *containerToolBar;
    QAction *newContainerAction;
    QAction *openContainerAction;
    QAction *openDirectoryAction;
    QAction *injectAction;
    QAction *extractAction;
    QAction *selectAllAction;
    QAction *deselectAllAction;
    QAction *findFilesAction;
#ifndef QT_NO_CLIPBOARD
    QAction *copyAction;
    QAction *pasteAction;
#endif
    QAction *deleteAction;
    QAction *viewAction;
    QAction *attributesAction;
    QAction *infoAction;
    QAction *optionsAction;
    QAction *closeAction;
    QAction *closeAllAction;
    QAction *tileAction;
    QAction *cascadeAction;
    QAction *nextAction;
    QAction *previousAction;
    QAction *windowMenuSeparatorAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    QSize newDialogSize;
    QSize optionsDialogSize;
    QSize attributesDialogSize;
    QString findPattern;
    QString injectDirectory;
    QString extractDirectory;
    sFPOptions &options;
};

#endif
