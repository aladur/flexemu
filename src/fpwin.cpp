/*
    fpwin.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2020-2023  W. Schwotzer

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
#include "mdcrtape.h"
#include "fpwin.h"
#include "fpnewui.h"
#include "fpoptui.h"
#include "fpattrui.h"
#include "fpmdich.h"
#include "filecntb.h"
#include "bregistr.h"
#include "brcfile.h"
#include "flexerr.h"
#include "ffilecnt.h"
#include "fcopyman.h"
#include "qtfree.h"
#include "warnoff.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMdiSubWindow>
#include <QMdiArea>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QEvent>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include "warnon.h"


FLEXplorer::FLEXplorer(sFPOptions &p_options) : mdiArea(new QMdiArea),
    windowMenu(nullptr),
    l_selectedFilesCount(nullptr), l_selectedFilesByteSize(nullptr),
    fileToolBar(nullptr), editToolBar(nullptr),
    containerToolBar(nullptr),
    newContainerAction(nullptr), openContainerAction(nullptr),
    openDirectoryAction(nullptr), 
    injectAction(nullptr), extractAction(nullptr),
    selectAllAction(nullptr), deselectAllAction(nullptr),
    findFilesAction(nullptr),
#ifndef QT_NO_CLIPBOARD
    copyAction(nullptr), pasteAction(nullptr),
#endif
    deleteAction(nullptr), viewAction(nullptr),
    attributesAction(nullptr),
    infoAction(nullptr),
    optionsAction(nullptr),
    closeAction(nullptr), closeAllAction(nullptr),
    tileAction(nullptr), cascadeAction(nullptr),
    nextAction(nullptr), previousAction(nullptr),
    windowMenuSeparatorAction(nullptr),
    aboutAction(nullptr),
    aboutQtAction(nullptr),
    newDialogSize{0, 0}, optionsDialogSize{0, 0},
    attributesDialogSize{0, 0},
    findPattern("*.*"),
    options(p_options)
{
    injectDirectory = getHomeDirectory().c_str();
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QImage image(":resource/background.png");
    mdiArea->setBackground(image);
    setCentralWidget(mdiArea);
    setAcceptDrops(true);
    mdiArea->setAcceptDrops(false);
    connect(mdiArea, &QMdiArea::subWindowActivated,
            this, &FLEXplorer::SubWindowActivated);

    CreateActions();
    CreateStatusBar();
    UpdateMenus();
    RestoreRecentDisks();

    setWindowTitle(tr("FLEXplorer"));
    setUnifiedTitleAndToolBarOnMac(true);

    resize(860, 720);
}

FLEXplorer::~FLEXplorer()
{
    DeleteRecentDiskActions();
}

void FLEXplorer::NewContainer()
{
    QDialog dialog;
    FlexplorerNewUi ui;
    ui.setupUi(dialog);
    ui.SetDefaultPath(QString(options.openDiskPath.c_str()));
    ui.TransferDataToDialog(TYPE_DSK_CONTAINER, 80, 36);
    dialog.resize(newDialogSize);
    auto result = dialog.exec();
    newDialogSize = dialog.size();

    if (result == QDialog::Accepted)
    {
        options.openDiskPath = ui.GetDefaultPath().toStdString();
        try
        {
            if (ui.GetFormat() == TYPE_MDCR_CONTAINER)
            {
                MiniDcrTapePtr mdcr = MiniDcrTape::Create(
                                      ui.GetPath().toUtf8().data());
                // DCR containers can be created but not displayed in
                // FLEXplorer, so immediately return.
                return;
            }

            auto filename = getFileName(ui.GetPath().toStdString());
            auto directory = getParentPath(ui.GetPath().toStdString());
            const char *bsFile = !options.bootSectorFile.empty() ?
                                 options.bootSectorFile.c_str() : nullptr;
            auto *container = FlexFileContainer::Create(
                                  directory.c_str(),
                                  filename.c_str(),
                                  ui.GetTracks(), ui.GetSectors(),
                                  options.ft_access,
                                  ui.GetFormat(),
                                  bsFile);
            delete container;

            OpenContainerForPath(ui.GetPath());
        }
        catch (FlexException &ex)
        {
            QMessageBox::critical(this, tr("FLEXplorer Error"), ex.what());
        }
    }
}

void FLEXplorer::OpenContainer()
{
    const auto defaultDir = QString(options.openDiskPath.c_str());
    QStringList filePaths;
    QFileDialog dialog(this, tr("Select FLEX file containers"), defaultDir,
                       "FLEX file containers (*.dsk *.flx *.wta);;"
                       "All files (*.*)");

    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
    {
        const auto path =
            QDir::toNativeSeparators(dialog.directory().absolutePath());
        options.openDiskPath = path.toStdString();
        filePaths = dialog.selectedFiles();
    }

    for (const auto &filePath : filePaths)
    {
        bool isLast = (!QString::compare(filePath, filePaths.back()));
        const auto path = QDir::toNativeSeparators(filePath);

        if (!OpenContainerForPath(path, isLast))
        {
            break;
        }
    }
}

void FLEXplorer::OpenDirectory()
{
    const auto defaultDir = QString(options.openDirectoryPath.c_str());
    QFileDialog dialog(this, tr("Open a FLEX directory container"),
                       defaultDir);

    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dialog.exec() == QDialog::Accepted)
    {
        const auto directories = dialog.selectedFiles();

        auto path =
            QDir::toNativeSeparators(dialog.directory().absolutePath());
        options.openDirectoryPath = path.toStdString();

        if (directories.size() >= 1)
        {
            path = QDir::toNativeSeparators(directories[0]);
            OpenContainerForPath(path);
        }
    }
}

void FLEXplorer::OpenRecentDisk()
{
    auto *action = qobject_cast<QAction *>(sender());

    if (action != nullptr)
    {
        OpenContainerForPath(action->data().toString());
    }
}

bool FLEXplorer::OpenContainerForPath(QString path, bool isLast)
{
    // If path ends with a path separator character it will be cut off.
    // This can happen when calling FLEXplorer on the command line with
    // a path entered with command line completion.
    if (path.size() > 1 && path.right(1) == QString(PATHSEPARATORSTRING))
    {
        path.resize(path.size() - 1);
    }

    try
    {
        auto *child = CreateMdiChild(path, options);

        child->show();

        SetStatusMessage(tr("Loaded %1").arg(path));
        UpdateForRecentDisk(path);

        return true;
    }
    catch (FlexException &ex)
    {
        auto message = QString(ex.what());
        QMessageBox::StandardButtons buttons =
            isLast ? QMessageBox::Ok : QMessageBox::Yes | QMessageBox::No;

        if (!isLast)
        {
            message += "\nContinue?";
        }
        auto result = QMessageBox::critical(
                this, tr("FLEXplorer Error"),
                message, buttons);

        return isLast || (result == QMessageBox::Yes);
    }
}

void FLEXplorer::ExecuteInChild(
        std::function<void(FlexplorerMdiChild &child)> action)
{
    auto *child = ActiveMdiChild();

    if (child != nullptr)
    {
        action(*child);
    }
}

#ifndef QT_NO_CLIPBOARD
void FLEXplorer::Copy()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto count = child.CopySelectedToClipboard();
        SetStatusMessage(tr("Copied %1 file(s) into clipboard").arg(count));
    });
}

void FLEXplorer::Paste()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto count = child.PasteFromClipboard();
        SetStatusMessage(tr("Pasted %1 file(s) from clipboard").arg(count));
    });
}
#endif

void FLEXplorer::SelectAll()
{
    ExecuteInChild([](FlexplorerMdiChild &child)
    {
        child.SelectAll();
    });
}

void FLEXplorer::DeselectAll()
{
    ExecuteInChild([](FlexplorerMdiChild &child)
    {
        child.DeselectAll();
    });
}

void FLEXplorer::FindFiles()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        bool success = false;
        auto result =
            QInputDialog::getText(this, tr("FLEXplorer Find Files"),
                                  tr("Enter a pattern for files to look for.\n"
                                     "Wildcards ? and * are allowed"),
                                  QLineEdit::Normal, findPattern, &success);

        if (success && !result.isEmpty())
        {
            findPattern = result;
            auto count = child.FindFiles(findPattern);
            auto message = tr("Found %1 file(s) with pattern %2");
            message = message.arg(count).arg(findPattern);
            SetStatusMessage(message);
        }
    });
}

void FLEXplorer::DeleteSelected()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto filenames = child.GetSelectedFilenames();
        auto message =
            (filenames.size() == 1 ?
                tr("Delete %1?").arg(filenames.at(0)) :
                tr("Delete all %1 selected files?").arg(filenames.size()));
        auto buttons = QMessageBox::Yes | QMessageBox::No;
        auto result = QMessageBox::question(this, tr("FLEXplorer"),
                                            message,
                                            buttons,
                                            QMessageBox::Yes);
        if (result == QMessageBox::Yes)
        {
            auto count = child.DeleteSelected();
            SetStatusMessage(tr("Deleted %1 file(s)").arg(count));
        }
    });
}

void FLEXplorer::InjectFiles()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        QFileDialog dialog(this, tr("Select file(s) to inject"),
                           injectDirectory);

        dialog.setFileMode(QFileDialog::ExistingFiles);
        dialog.setDirectory(injectDirectory);
        dialog.setViewMode(QFileDialog::Detail);

        if (dialog.exec())
        {
            auto fileNames = dialog.selectedFiles();
            auto count = child.InjectFiles(fileNames);
            SetStatusMessage(tr("Injected %1 file(s)").arg(count));
            injectDirectory = dialog.directory().absolutePath();
        }
    });
}

void FLEXplorer::ExtractSelected()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto *subWindow = mdiArea->activeSubWindow();
        auto targetDirectory = QFileDialog::getExistingDirectory(this,
                tr("Target Directory to extract file(s)"), extractDirectory,
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        // As a result of opening a directory browser there maybe is no active
        // child any more => reactivate it.
        mdiArea->setActiveSubWindow(subWindow);

        if (!targetDirectory.isEmpty())
        {
            auto count = child.ExtractSelected(targetDirectory);
            SetStatusMessage(tr("Extracted %1 file(s)").arg(count));
            extractDirectory = targetDirectory;
        }
    });
}

void FLEXplorer::ViewSelected()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto count = child.ViewSelected();
        SetStatusMessage(tr("Opened %1 file(s) in editor").arg(count));
    });
}

void FLEXplorer::AttributesSelected()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        QDialog dialog;
        FileAttributesUi ui;
        ui.setupUi(dialog);
        auto supportedAttributes = child.GetSupportedAttributes();
        auto filenames = child.GetSelectedFilenames();
        auto attributes = child.GetSelectedAttributes();
        bool isWriteProtected = child.IsWriteProtected();

        if (attributes.isEmpty())
        {
            return;
        }

        ui.TransferDataToDialog(filenames, attributes, supportedAttributes,
                                isWriteProtected);
        dialog.resize(attributesDialogSize);
        auto result = dialog.exec();
        attributesDialogSize = dialog.size();

        if (result == QDialog::Accepted)
        {
            auto count = child.SetSelectedAttributes(
                                   ui.GetSetMask(), ui.GetClearMask());
            SetStatusMessage(tr("Changed attributes of %1 file(s)").arg(count));
        }
    });
}

void FLEXplorer::Options()
{
    QDialog dialog;
    FlexplorerOptionsUi ui;
    ui.setupUi(dialog);
    ui.TransferDataToDialog(options);
    dialog.resize(optionsDialogSize);
    auto result = dialog.exec();
    optionsDialogSize = dialog.size();

    if (result == QDialog::Accepted)
    {
        auto oldFileTimeAccess = options.ft_access;
        auto oldFileSizeType = options.fileSizeType;

        ui.TransferDataFromDialog(options);
        FlexFileContainer::bootSectorFile = options.bootSectorFile;
        if (oldFileTimeAccess != options.ft_access)
        {
            emit FileTimeAccessHasChanged();
        }

        if (oldFileSizeType != options.fileSizeType)
        {
            emit FileSizeTypeHasChanged();
        }
    }
}

void FLEXplorer::Info()
{
    ExecuteInChild([](FlexplorerMdiChild &child)
    {
        child.Info();
    });
}

void FLEXplorer::About()
{
    auto version = QString(VERSION);

    QMessageBox::about(this, tr("About FLEXplorer"),
        tr("<b>FLEXplorer V%1</b><p>"
           "FLEXplorer is an explorer for FLEX disk and "
           "directory containers.<p>FLEXplorer comes with "
           "ABSOLUTELY NO WARRANTY. This is free software, and You "
           "are welcome to redistribute it under certain conditions.<p>"
           "Please notice that this project was developed under the "
           "terms of the "
           "<a href=\"http://flexemu.neocities.org/copying.txt\">"
           "GNU GENERAL PUBLIC LICENCE V2</a>.<p><p>"
           "Have Fun!<p><p>"
           "Copyright (C) 1998-2023 "
           "<a href=\"mailto:wolfgang.schwotzer@gmx.net\">"
           "Wolfgang Schwotzer</a><p>"
           "<a href=\"http://flexemu.neocities.org\">"
           "http://flexemu.neocities.org</a>")
        .arg(version));
}

void FLEXplorer::UpdateMenus()
{
    auto *child = ActiveMdiChild();
    bool hasMdiChild = (child != nullptr);
    bool isWriteProtected = (hasMdiChild && child->IsWriteProtected());
    bool hasSelection = (hasMdiChild && child->GetSelectedFilesCount() > 0);

    injectAction->setEnabled(hasMdiChild && !isWriteProtected);
    extractAction->setEnabled(hasMdiChild && hasSelection);
    selectAllAction->setEnabled(hasMdiChild);
    deselectAllAction->setEnabled(hasMdiChild);
    findFilesAction->setEnabled(hasMdiChild);
#ifndef QT_NO_CLIPBOARD
    copyAction->setEnabled(hasMdiChild && hasSelection);
    pasteAction->setEnabled(hasMdiChild && !isWriteProtected);
#endif
    deleteAction->setEnabled(!isWriteProtected && hasSelection);
    viewAction->setEnabled(hasSelection);
    attributesAction->setEnabled(hasSelection);

    infoAction->setEnabled(hasMdiChild);

    closeAction->setEnabled(hasMdiChild);
    closeAllAction->setEnabled(hasMdiChild);
    tileAction->setEnabled(hasMdiChild);
    cascadeAction->setEnabled(hasMdiChild);
    nextAction->setEnabled(hasMdiChild);
    previousAction->setEnabled(hasMdiChild);
    windowMenuSeparatorAction->setVisible(hasMdiChild);
}

void FLEXplorer::UpdateWindowMenu()
{
    assert(windowMenu);

    windowMenu->clear();
    windowMenu->addAction(closeAction);
    windowMenu->addAction(closeAllAction);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAction);
    windowMenu->addAction(cascadeAction);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAction);
    windowMenu->addAction(previousAction);
    windowMenu->addAction(windowMenuSeparatorAction);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    windowMenuSeparatorAction->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i)
    {
        QMdiSubWindow *mdiSubWindow = windows.at(i);
        auto *child =
            qobject_cast<FlexplorerMdiChild *>(mdiSubWindow->widget());

        QString acceleratorPrefix = (i < 9) ? "&" : "";
        QString text = tr("%1%2 %3").arg(acceleratorPrefix)
                                .arg(i + 1)
                                .arg(child->GetUserFriendlyPath());
        auto *action = windowMenu->addAction(text);
        connect(action, &QAction::triggered,
                [this, mdiSubWindow]() {
                    mdiArea->setActiveSubWindow(mdiSubWindow);
                });
        action->setCheckable(true);
        action->setChecked(child == ActiveMdiChild());
        auto statusTip =
            tr("Activate window for %1").arg(child->GetUserFriendlyPath());
        action->setStatusTip(statusTip);
    }
}

void FLEXplorer::CloseActiveSubWindow()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto path = child.GetPath();
        mdiArea->closeActiveSubWindow();
        SetStatusMessage(tr("Closed %1").arg(path));
    });
}

void FLEXplorer::CloseAllSubWindows()
{
    mdiArea->closeAllSubWindows();
    SetStatusMessage(tr("Closed all windows"));
}

FlexplorerMdiChild *FLEXplorer::CreateMdiChild(const QString &path,
        struct sFPOptions &p_options)
{
    auto *child = new FlexplorerMdiChild(path, p_options);

    auto subWindow = mdiArea->addSubWindow(child);
    QString iconResource =
        (child->GetContainerType() & TYPE_DIRECTORY) ?
            ":resource/dir.png" :
            ":resource/flexplorer.png";
    subWindow->setWindowIcon(QIcon(iconResource));
    subWindow->setMinimumSize(200, 200);
    subWindow->resize(640, 560);
    child->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(child, &FlexplorerMdiChild::customContextMenuRequested,
            this, &FLEXplorer::ContextMenuRequested);
    connect(this, &FLEXplorer::FileTimeAccessHasChanged,
            child, &FlexplorerMdiChild::OnFileTimeAccessChanged);
    connect(child, &FlexplorerMdiChild::SelectionHasChanged,
            this, &FLEXplorer::SelectionHasChanged);
    connect(this, &FLEXplorer::FileSizeTypeHasChanged,
            child, &FlexplorerMdiChild::OnFileSizeTypeHasChanged);
    connect(this, &FLEXplorer::FileSizeTypeHasChanged,
            this, &FLEXplorer::UpdateSelectedFiles);

    return child;
}

QToolBar *FLEXplorer::CreateToolBar(QWidget *parent,
                                    const QString &title,
                                    const QString &objectName)
{
    QToolBar *toolBar = new QToolBar(title, parent);
    toolBar->setObjectName(objectName);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize({32, 32});
    addToolBar(toolBar);

    return toolBar;
}

void FLEXplorer::CreateActions()
{
    CreateFileActions();
    CreateEditActions();
    CreateContainerActions();
    CreateExtrasActions();
    CreateWindowsActions();
    CreateHelpActions();
}

void FLEXplorer::CreateFileActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileToolBar = CreateToolBar(this, tr("File"),
                                QStringLiteral("fileToolBar"));

    const auto exitIcon = QIcon(":/resource/exit.png");
    QAction *exitAction = fileMenu->addAction(exitIcon, tr("E&xit"),
                          this, SLOT(Exit()));
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    fileToolBar->addAction(exitAction);
    fileToolBar->addSeparator();

    const auto newIcon = QIcon(":/resource/new.png");
    newContainerAction = new QAction(newIcon, tr("&New Container..."), this);
    newContainerAction->setShortcuts(QKeySequence::New);
    newContainerAction->setStatusTip(tr("Create a new FLEX file container"));
    connect(newContainerAction, &QAction::triggered,
            this, &FLEXplorer::NewContainer);
    fileMenu->addAction(newContainerAction);
    fileToolBar->addAction(newContainerAction);

    const auto openIcon = QIcon(":/resource/open_con.png");
    openContainerAction = new QAction(openIcon, tr("&Open Container..."), this);
    openContainerAction->setShortcuts(QKeySequence::Open);
    openContainerAction->setStatusTip(
            tr("Open a FLEX file container"));
    connect(openContainerAction, &QAction::triggered,
            this, &FLEXplorer::OpenContainer);
    fileMenu->addAction(openContainerAction);
    fileToolBar->addAction(openContainerAction);

    const auto openDirIcon = QIcon(":/resource/open_dir.png");
    openDirectoryAction = new QAction(openDirIcon, tr("Open &Directory..."),
                                      this);
    openDirectoryAction->setShortcut(QKeySequence(tr("Ctrl+D")));
    openDirectoryAction->setStatusTip(
            tr("Open a directory as FLEX file container"));
    connect(openDirectoryAction, &QAction::triggered,
            this, &FLEXplorer::OpenDirectory);
    fileMenu->addAction(openDirectoryAction);
    fileToolBar->addAction(openDirectoryAction);

    recentDisksMenu = fileMenu->addMenu(tr("&Recent Disks"));
    CreateRecentDiskActionsFor(recentDisksMenu);

    fileMenu->addSeparator();

    fileMenu->addAction(exitAction);
}

void FLEXplorer::CreateEditActions()
{
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editToolBar = CreateToolBar(this, tr("Edit"),
                                QStringLiteral("editToolBar"));

    const auto viewIcon = QIcon(":/resource/view.png");
    viewAction = new QAction(viewIcon, tr("&View..."), this);
    viewAction->setStatusTip(tr("View selected files"));
    auto font = viewAction->font();
    font.setBold(true);
    viewAction->setFont(font);
    connect(viewAction, &QAction::triggered, this, &FLEXplorer::ViewSelected);
    editMenu->addAction(viewAction);
    editToolBar->addAction(viewAction);

    const auto deleteIcon = QIcon(":/resource/delete.png");
    deleteAction = new QAction(deleteIcon, tr("&Delete..."), this);
    deleteAction->setStatusTip(tr("Delete selected files"));
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered,
            this, &FLEXplorer::DeleteSelected);
    editMenu->addAction(deleteAction);
    editToolBar->addAction(deleteAction);

    const auto attributesIcon = QIcon(":/resource/attributes.png");
    attributesAction = new QAction(attributesIcon, tr("&Attributes..."), this);
    attributesAction->setStatusTip(tr("Display and modify attributes of "
                                      "selected files"));
    connect(attributesAction, &QAction::triggered,
            this, &FLEXplorer::AttributesSelected);
    editMenu->addAction(attributesAction);
    editMenu->addSeparator();
    editToolBar->addAction(attributesAction);
    editToolBar->addSeparator();

    const auto injectIcon = QIcon(":/resource/inject.png");
    injectAction = new QAction(injectIcon, tr("&Inject..."), this);
    injectAction->setStatusTip(tr("Inject selected files"));
    injectAction->setShortcut(QKeySequence(tr("Ctrl+I")));
    connect(injectAction, &QAction::triggered, this,
            &FLEXplorer::InjectFiles);
    editMenu->addAction(injectAction);
    editToolBar->addAction(injectAction);

    const auto extractIcon = QIcon(":/resource/extract.png");
    extractAction = new QAction(extractIcon, tr("E&xtract..."), this);
    extractAction->setStatusTip(tr("Extract selected files"));
    extractAction->setShortcut(QKeySequence(tr("Ctrl+X")));
    connect(extractAction, &QAction::triggered, this,
            &FLEXplorer::ExtractSelected);
    editMenu->addAction(extractAction);
    editMenu->addSeparator();
    editToolBar->addAction(extractAction);
    editToolBar->addSeparator();

    const auto selectAllIcon = QIcon(":/resource/selectall.png");
    selectAllAction = new QAction(selectAllIcon, tr("Select &All"), this);
    selectAllAction->setStatusTip(tr("Select all files"));
    selectAllAction->setShortcuts(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, &FLEXplorer::SelectAll);
    editMenu->addAction(selectAllAction);
    editToolBar->addAction(selectAllAction);

    const auto deselectAllIcon = QIcon(":/resource/deselectall.png");
    deselectAllAction = new QAction(deselectAllIcon, tr("D&elect All"), this);
    deselectAllAction->setStatusTip(tr("Deselect all files"));
    deselectAllAction->setShortcut(QKeySequence(tr("Ctrl+E")));
    connect(deselectAllAction, &QAction::triggered, this,
            &FLEXplorer::DeselectAll);
    editMenu->addAction(deselectAllAction);
    editToolBar->addAction(deselectAllAction);

    const auto findFilesIcon = QIcon(":/resource/find.png");
    findFilesAction = new QAction(findFilesIcon, tr("&Find Files..."), this);
    findFilesAction->setShortcut(QKeySequence::Find);
    findFilesAction->setStatusTip(tr("Find files by filename pattern"));
    connect(findFilesAction, &QAction::triggered, this, &FLEXplorer::FindFiles);
    editMenu->addAction(findFilesAction);
    editToolBar->addAction(findFilesAction);
    editToolBar->addSeparator();

#ifndef QT_NO_CLIPBOARD
    editMenu->addSeparator();
    const auto copyIcon = QIcon(":/resource/copy.png");
    copyAction = new QAction(copyIcon, tr("&Copy"), this);
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setStatusTip(tr("Copy selected files to the clipboard"));
    connect(copyAction, &QAction::triggered, this, &FLEXplorer::Copy);
    editMenu->addAction(copyAction);
    editToolBar->addAction(copyAction);

    const auto pasteIcon = QIcon(":/resource/paste.png");
    pasteAction = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAction->setShortcuts(QKeySequence::Paste);
    pasteAction->setStatusTip(tr("Paste files from the clipboard"));
    connect(pasteAction, &QAction::triggered, this, &FLEXplorer::Paste);
    editMenu->addAction(pasteAction);
    editToolBar->addAction(pasteAction);
    editToolBar->addSeparator();
#endif
}

void FLEXplorer::CreateContainerActions()
{
    QMenu *containerMenu = menuBar()->addMenu(tr("&Container"));
    containerToolBar = CreateToolBar(this, tr("Edit"),
                                     QStringLiteral("containerToolBar"));

    const auto infoIcon = QIcon(":/resource/info.png");
    infoAction = new QAction(infoIcon, tr("&Info..."), this);
    infoAction->setShortcut(QKeySequence(tr("Ctrl+I")));
    infoAction->setStatusTip(tr("Show container properties"));
    connect(infoAction, &QAction::triggered, this, &FLEXplorer::Info);
    containerMenu->addAction(infoAction);
    containerToolBar->addAction(infoAction);
}

void FLEXplorer::CreateExtrasActions()
{
    QMenu *extrasMenu = menuBar()->addMenu(tr("&Extras"));
    auto *extrasToolBar = CreateToolBar(this, tr("Extras"),
                                        QStringLiteral("extrasToolBar"));

    const auto optionsIcon = QIcon(":/resource/options.png");
    optionsAction = new QAction(optionsIcon, tr("&Options..."), this);
    optionsAction->setStatusTip(tr("Display and modify application's options"));
    connect(optionsAction, &QAction::triggered, this, &FLEXplorer::Options);
    extrasMenu->addAction(optionsAction);
    extrasToolBar->addAction(optionsAction);
}

void FLEXplorer::CreateWindowsActions()
{
    windowMenu = menuBar()->addMenu(tr("&Window"));
    auto *windowToolBar = CreateToolBar(this, tr("Window"),
                                        QStringLiteral("windowToolBar"));
    connect(windowMenu, &QMenu::aboutToShow,
            this, &FLEXplorer::UpdateWindowMenu);

    const auto closeIcon = QIcon(":/resource/window-close.png");
    closeAction = new QAction(closeIcon, tr("Cl&ose"), this);
    closeAction->setStatusTip(tr("Close the active window"));
    connect(closeAction, &QAction::triggered,
            this, &FLEXplorer::CloseActiveSubWindow);
    windowToolBar->addAction(closeAction);

    closeAllAction = new QAction(tr("Close &All"), this);
    closeAllAction->setStatusTip(tr("Close all the windows"));
    connect(closeAllAction, &QAction::triggered,
            this, &FLEXplorer::CloseAllSubWindows);

    tileAction = new QAction(tr("&Tile"), this);
    tileAction->setStatusTip(tr("Tile the windows"));
    connect(tileAction, &QAction::triggered, mdiArea,
            &QMdiArea::tileSubWindows);

    cascadeAction = new QAction(tr("&Cascade"), this);
    cascadeAction->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAction, &QAction::triggered, mdiArea,
            &QMdiArea::cascadeSubWindows);

    nextAction = new QAction(tr("Ne&xt"), this);
    nextAction->setShortcuts(QKeySequence::NextChild);
    nextAction->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAction, &QAction::triggered, mdiArea,
            &QMdiArea::activateNextSubWindow);

    previousAction = new QAction(tr("Pre&vious"), this);
    previousAction->setShortcuts(QKeySequence::PreviousChild);
    previousAction->setStatusTip(tr("Move the focus to the previous window"));
    connect(previousAction, &QAction::triggered, mdiArea,
            &QMdiArea::activatePreviousSubWindow);

    windowMenuSeparatorAction = new QAction(this);
    windowMenuSeparatorAction->setSeparator(true);
    menuBar()->addSeparator();

    UpdateWindowMenu();
}

void FLEXplorer::CreateHelpActions()
{
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    auto *helpToolBar = CreateToolBar(this, tr("Help"),
                                      QStringLiteral("helpToolBar"));

    const auto aboutIcon = QIcon(":/resource/about.png");
    aboutAction = helpMenu->addAction(aboutIcon, tr("&About"),
                                      this, SLOT(About()));
    aboutAction->setStatusTip(tr("Show the application's About box"));
    helpToolBar->addAction(aboutAction);

    const auto aboutQtIcon = QIcon(":/resource/qt.png");
    aboutQtAction = helpMenu->addAction(aboutQtIcon, tr("&About Qt"),
                                      qApp, SLOT(aboutQt()));
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    helpToolBar->addAction(aboutQtAction);
}

void FLEXplorer::ContextMenuRequested(QPoint pos)
{
    assert(selectAllAction);

    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        QMenu *contextMenu = new QMenu(this);

        contextMenu->addAction(viewAction);
        contextMenu->addAction(deleteAction);
        contextMenu->addAction(attributesAction);
        contextMenu->addSeparator();
        contextMenu->addAction(injectAction);
        contextMenu->addAction(extractAction);
        contextMenu->addSeparator();
        contextMenu->addAction(selectAllAction);
        contextMenu->addAction(deselectAllAction);
        contextMenu->addAction(findFilesAction);
#ifndef QT_NO_CLIPBOARD
        contextMenu->addSeparator();
        contextMenu->addAction(copyAction);
        contextMenu->addAction(pasteAction);
#endif
        contextMenu->popup(child.viewport()->mapToGlobal(pos));
    });
}

void FLEXplorer::CreateStatusBar()
{
    auto *leftSpace = new QLabel(this);
    l_selectedFilesCount = new QLabel(this);
    l_selectedFilesByteSize = new QLabel(this);
    l_selectedFilesCount->setContentsMargins(10, 0, 10, 0);

    statusBar()->setStyleSheet(
        "QStatusBar::item { border: 1px solid grey; border-radius: 2px; } ");
    statusBar()->addWidget(leftSpace, 1);
    statusBar()->addWidget(l_selectedFilesCount);
    statusBar()->addWidget(l_selectedFilesByteSize);
    SetStatusMessage(tr("Ready"));
}

FlexplorerMdiChild *FLEXplorer::ActiveMdiChild() const
{
    QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow();

    if (activeSubWindow != nullptr)
    {
        return qobject_cast<FlexplorerMdiChild *>(activeSubWindow->widget());
    }

    return nullptr;

}

QMdiSubWindow *FLEXplorer::FindMdiChild(const QString &path) const
{
    foreach (QMdiSubWindow *window, mdiArea->subWindowList())
    {
        auto *mdiChild = qobject_cast<FlexplorerMdiChild *>(window->widget());
        if (mdiChild->GetPath() == path)
        {
            return window;
        }
    }

    return nullptr;
}

void FLEXplorer::Exit()
{
    qApp->closeAllWindows();
    QCoreApplication::quit();
}

void FLEXplorer::changeEvent(QEvent *event)
{
    if (
        event->type() == QEvent::FontChange
        || event->type() == QEvent::StyleChange
        || event->type() == QEvent::ThemeChange
       )
    {
        newDialogSize = QSize(0, 0);
        optionsDialogSize = QSize(0, 0);
        attributesDialogSize = QSize(0, 0);
    }
}

void FLEXplorer::SubWindowActivated(QMdiSubWindow *window)
{
    UpdateMenus();

    if (window == nullptr)
    {
        l_selectedFilesCount->setText("");
        l_selectedFilesByteSize->setText("");
    }
    else
    {
        UpdateSelectedFiles();
    }
}

void FLEXplorer::UpdateSelectedFiles()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto selectedFilesCount = child.GetSelectedFilesCount();
        auto selectedFilesByteSize = child.GetSelectedFilesByteSize();
        const char *format =
            (selectedFilesCount == 1) ?
                "%d File selected" : "%d Files selected";

        auto text = QString::asprintf(format, selectedFilesCount);
        l_selectedFilesCount->setText(text);
        text = QString::asprintf("%d Byte", selectedFilesByteSize);
        l_selectedFilesByteSize->setText(text);
    });
}

void FLEXplorer::SelectionHasChanged()
{
    UpdateMenus();
    UpdateSelectedFiles();
}

void FLEXplorer::SetStatusMessage(const QString &message)
{
    statusBar()->showMessage(message, 4000);
}

QStringList FLEXplorer::GetSupportedFiles(const QMimeData *mimeData)
{
    static const QStringList supportedExtensions{ ".dsk", ".flx", ".wta" };
    QStringList supportedFiles;

    for (const auto &url : mimeData->urls())
    {
        const auto path = QDir::toNativeSeparators(url.toLocalFile());
        const auto fileExtension =
            QString(getFileExtension(path.toStdString()).c_str()).toLower();

        if (supportedExtensions.contains(fileExtension))
        {
            supportedFiles.append(path);
        }
    }

    return supportedFiles;
}

void FLEXplorer::dragEnterEvent(QDragEnterEvent *event)
{
    if (!GetSupportedFiles(event->mimeData()).isEmpty())
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void FLEXplorer::dragMoveEvent(QDragMoveEvent *event)
{
    if (!GetSupportedFiles(event->mimeData()).isEmpty())
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void FLEXplorer::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void FLEXplorer::dropEvent(QDropEvent *event)                           
{
    const auto supportedFiles = GetSupportedFiles(event->mimeData());
    if (!supportedFiles.isEmpty())
    {
        for (int i = 0; i < supportedFiles.size(); ++i)
        {
            bool isLast = (i + 1 == supportedFiles.size());
            const auto &path = supportedFiles.at(i);
            if (!OpenContainerForPath(path.toUtf8().data(), isLast))
            {
                break;
            }
        }
    }
}

void FLEXplorer::SetFileTimeAccess(FileTimeAccess fileTimeAccess)
{
    auto hasChanged = (options.ft_access != fileTimeAccess);

    options.ft_access = fileTimeAccess;
    if (hasChanged)
    {
        emit FileTimeAccessHasChanged();
    }
}

void FLEXplorer::CreateRecentDiskActionsFor(QMenu *menu)
{
    QAction *action = nullptr;

    for (auto i = 0; i < options.maxRecentFiles; ++i)
    {
        // Create all actions, de/activate them by setVisible() false/true.
        action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &FLEXplorer::OpenRecentDisk);
        recentDiskActions.append(action);
        menu->addAction(action);
    }

    UpdateRecentDiskActions();
}

// Always call this function when recentDiskPaths has changed or the
// recentDiskActions have been initialized.
void FLEXplorer::UpdateRecentDiskActions() const
{
    QStringList::size_type idx = 0;
    for (const auto &path : recentDiskPaths)
    {
        auto fileInfo = QFileInfo(path);
        auto idxString = QString("%1. ").arg(idx + 1);
        const auto strippedPath = StripPath(path);

        recentDiskActions.at(idx)->setText(idxString + strippedPath);
        recentDiskActions.at(idx)->setData(path);
        recentDiskActions.at(idx)->setVisible(true);
        if (fileInfo.isFile())
        {
            const QIcon icon(":/resource/open_con.png");
            recentDiskActions.at(idx)->setIcon(icon);
        }
        else if (fileInfo.isDir())
        {
            const QIcon icon(":/resource/open_dir.png");
            recentDiskActions.at(idx)->setIcon(icon);
        }
        ++idx;
    }

    for (idx = recentDiskPaths.size(); idx < options.maxRecentFiles; ++idx)
    {
        recentDiskActions.at(idx)->setVisible(false);
    }
}

void FLEXplorer::DeleteRecentDiskActions()
{
    while (!recentDiskActions.isEmpty())
    {
        delete recentDiskActions.takeLast();
    }
}

// Call this when a disk has been successfully loaded.
void FLEXplorer::UpdateForRecentDisk(const QString &path)
{
    recentDiskPaths.removeAll(path);
    recentDiskPaths.prepend(path);
    while (recentDiskPaths.size() > options.maxRecentFiles)
    {
        recentDiskPaths.removeLast();
    }
    options.recentDiskPaths.clear();
    for (const auto &diskPath : recentDiskPaths)
    {
        options.recentDiskPaths.push_back(diskPath.toStdString());
    }

    UpdateRecentDiskActions();
}

void FLEXplorer::RestoreRecentDisks()
{
    recentDiskPaths.clear();
    for (const auto &path : options.recentDiskPaths)
    {
        const QFileInfo fileInfo(path.c_str());

        if (fileInfo.exists())
        {
            recentDiskPaths.push_back(path.c_str());
        }
    }

    UpdateRecentDiskActions();
}

