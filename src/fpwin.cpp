/*
    fpwin.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2020-2024  W. Schwotzer

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
#include "about_ui.h"
#include "fversion.h"
#include "warnoff.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QSize>
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
#include <QTextBrowser>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "warnon.h"


FLEXplorer::FLEXplorer(sFPOptions &p_options)
    : mdiArea(new QMdiArea)
    , options(p_options)
{
    const QSize iconSize(options.iconSize, options.iconSize);

    injectDirectory = flx::getHomeDirectory().c_str();
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QImage image(":resource/background.png");
    mdiArea->setBackground(image);
    setCentralWidget(mdiArea);
    setAcceptDrops(true);
    mdiArea->setAcceptDrops(false);
    connect(mdiArea, &QMdiArea::subWindowActivated,
            this, &FLEXplorer::OnSubWindowActivated);

    CreateActions(iconSize);
    CreateStatusBar();
    UpdateMenus();
    RestoreRecentDisks();
    RestoreRecentDirectories();

    setWindowTitle(tr("FLEXplorer"));
    setUnifiedTitleAndToolBarOnMac(true);
    FlexDisk::onTrack0OnlyDirSectors = options.onTrack0OnlyDirSectors;
    SetIconSizeCheck(iconSize);

    resize(860, 720);
}

FLEXplorer::~FLEXplorer()
{
    DeleteRecentDirectoryActions();
    DeleteRecentDiskActions();
}

void FLEXplorer::OnNewFlexDisk()
{
    QDialog dialog;
    FlexplorerNewUi ui;
    ui.setupUi(dialog);
    ui.SetDefaultPath(QString(options.openDiskPath.c_str()));
    ui.TransferDataToDialog(DiskType::DSK, 80, 36);
    dialog.resize(newDialogSize);
    auto result = dialog.exec();
    newDialogSize = dialog.size();

    if (result == QDialog::Accepted)
    {
        options.openDiskPath = ui.GetDefaultPath().toStdString();
        try
        {
            if (ui.IsMDCRDiskActive())
            {
                QFile file(ui.GetPath());
                auto path = ui.GetPath().toStdString();

                if (file.exists())
                {
                    if (!file.remove())
                    {
                        throw FlexException(FERR_REMOVE, path);
                    }
                }

                MiniDcrTapePtr mdcr = MiniDcrTape::Create(path);
                // DCR containers can be created but not displayed in
                // FLEXplorer, so immediately return.
                return;
            }

            if (!ui.IsDiskTypeValid())
            {
                return;
            }

            auto path = ui.GetPath().toStdString();
            const char *bsFile = !options.bootSectorFile.empty() ?
                                 options.bootSectorFile.c_str() : nullptr;
            auto *container = FlexDisk::Create(
                                  path,
                                  options.ft_access,
                                  ui.GetTracks(),
                                  ui.GetSectors(),
                                  ui.GetDiskType(),
                                  bsFile);
            delete container;

            OpenFlexDiskForPath(ui.GetPath());
        }
        catch (FlexException &ex)
        {
            QMessageBox::critical(this, tr("FLEXplorer Error"), ex.what());
        }
    }
}

void FLEXplorer::OnOpenFlexDisk()
{
    const auto defaultDir = QString(options.openDiskPath.c_str());
    QStringList filePaths;
    QFileDialog dialog(this, tr("Select FLEX disk image files"), defaultDir,
                       "FLEX disk image files (*.dsk *.flx *.wta);;"
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

        if (!OpenFlexDiskForPath(path, isLast))
        {
            break;
        }
    }
}

void FLEXplorer::OnOpenDirectory()
{
    const auto defaultDir = QString(options.openDirectoryPath.c_str());
    QFileDialog dialog(this, tr("Open a FLEX directory disk"),
                       defaultDir);

    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if (dialog.exec() == QDialog::Accepted)
    {
        const auto directories = dialog.selectedFiles();

        auto path =
            QDir::toNativeSeparators(dialog.directory().absolutePath());
        options.openDirectoryPath = path.toStdString();

        if (!directories.empty())
        {
            path = QDir::toNativeSeparators(directories[0]);
            OpenFlexDiskForPath(path);
        }
    }
}

void FLEXplorer::OnOpenRecentDisk()
{
    auto *action = qobject_cast<QAction *>(sender());

    if (action != nullptr)
    {
        OpenFlexDiskForPath(action->data().toString());
    }
}

void FLEXplorer::OnOpenRecentDirectory()
{
    auto *action = qobject_cast<QAction *>(sender());

    if (action != nullptr)
    {
        OpenFlexDiskForPath(action->data().toString());
    }
}

void FLEXplorer::ProcessArguments(const QStringList &args)
{
    int i = 0;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    for (auto arg : args)
    {
        bool isLast = (i == args.size() - 1);

        if (i > 0)
        {
            arg = QDir::toNativeSeparators(arg);
            if (!OpenFlexDiskForPath(arg, isLast))
            {
                break;
            }
            QApplication::processEvents();
        }
        ++i;
    }

    QApplication::restoreOverrideCursor();
}

bool FLEXplorer::OpenFlexDiskForPath(QString path, bool isLast)
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
        auto fileInfo = QFileInfo(path);
        if (fileInfo.isFile())
        {
            UpdateForRecentDisk(path);
        }
        else if (fileInfo.isDir())
        {
            UpdateForRecentDirectory(path);
        }

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
        const std::function<void(FlexplorerMdiChild &child)>& action)
{
    auto *child = ActiveMdiChild();

    if (child != nullptr)
    {
        action(*child);
    }
}

#ifndef QT_NO_CLIPBOARD
void FLEXplorer::OnCopy()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto count = child.CopySelectedToClipboard();
        SetStatusMessage(tr("Copied %1 file(s) into clipboard").arg(count));
    });
}

void FLEXplorer::OnPaste()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto count = child.PasteFromClipboard();
        SetStatusMessage(tr("Pasted %1 file(s) from clipboard").arg(count));
    });
}
#endif

void FLEXplorer::OnSelectAll()
{
    ExecuteInChild([](FlexplorerMdiChild &child)
    {
        child.SelectAll();
    });
}

void FLEXplorer::OnDeselectAll()
{
    ExecuteInChild([](FlexplorerMdiChild &child)
    {
        child.DeselectAll();
    });
}

void FLEXplorer::OnFindFiles()
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

void FLEXplorer::OnDeleteSelected()
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

void FLEXplorer::OnInjectFiles()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        QFileDialog dialog(this, tr("Select file(s) to inject"));

        dialog.setFileMode(QFileDialog::ExistingFiles);
        dialog.setDirectory(options.openInjectFilePath.c_str());
        dialog.setViewMode(QFileDialog::Detail);

        if (dialog.exec() == QDialog::Accepted)
        {
            auto fileNames = dialog.selectedFiles();
            auto count = child.InjectFiles(fileNames);
            SetStatusMessage(tr("Injected %1 file(s)").arg(count));
            const auto path =
                QDir::toNativeSeparators(dialog.directory().absolutePath());
            options.openInjectFilePath = path.toStdString();
        }
    });
}

void FLEXplorer::OnExtractSelected()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto *subWindow = mdiArea->activeSubWindow();
        const auto defaultDir = QString(options.openInjectFilePath.c_str());
        QFileDialog dialog(this, tr("Target directory to extract file(s)"));

        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);
        dialog.setDirectory(defaultDir);

        auto result = dialog.exec();

        // As a result of opening a directory browser there maybe is no active
        // child any more => reactivate it.
        mdiArea->setActiveSubWindow(subWindow);

        if (result == QDialog::Accepted)
        {
            const auto directories = dialog.selectedFiles();

            auto path =
                QDir::toNativeSeparators(dialog.directory().absolutePath());
            options.openInjectFilePath = path.toStdString();

            if (!directories.empty())
            {
                const auto tgtPath = QDir::toNativeSeparators(directories[0]);
                auto count = child.ExtractSelected(tgtPath);
                SetStatusMessage(tr("Extracted %1 file(s)").arg(count));
            }
        }
    });
}

void FLEXplorer::OnViewSelected()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto count = child.ViewSelected();
        SetStatusMessage(tr("Opened %1 file(s) in editor").arg(count));
    });
}

void FLEXplorer::OnAttributesSelected()
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

void FLEXplorer::OnOptions()
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
        FlexDisk::SetBootSectorFile(options.bootSectorFile);
        FlexDisk::onTrack0OnlyDirSectors =
            options.onTrack0OnlyDirSectors;
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

void FLEXplorer::OnInfo()
{
    ExecuteInChild([](FlexplorerMdiChild &child)
    {
        child.Info();
    });
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void FLEXplorer::OnAbout()
{
    QDialog dialog;
    Ui_AboutDialog ui{};

    ui.setupUi(&dialog);
    const auto aboutIcon = QIcon(":/resource/about.png");
    dialog.setWindowIcon(aboutIcon);
    dialog.resize({0, 0});
    dialog.setModal(true);
    dialog.setSizeGripEnabled(true);
    auto title = tr("About FLEXplorer");
    dialog.setWindowTitle(title);
    auto *scene = new QGraphicsScene(ui.w_icon);
    scene->setSceneRect(0, 0, 32, 32);
    ui.w_icon->setScene(scene);
    scene->addPixmap(QPixmap(":/resource/flexplorer.png"))->setPos(0, 0);

    auto aboutText = tr("<b>FLEXplorer V%1</b><p>"
           "FLEXplorer is an explorer for FLEX disk images and "
           "directory disks.<p>FLEXplorer comes with "
           "ABSOLUTELY NO WARRANTY. This is free software, and You "
           "are welcome to redistribute it under certain conditions.<p>"
           "Please notice that this project was developed under the "
           "terms of the "
           "<a href=\"http://flexemu.neocities.org/copying.txt\">"
           "GNU GENERAL PUBLIC LICENCE V2</a>.<p><p>"
           "Have Fun!<p><p>"
           "Copyright (C) 1998-2024 "
           "<a href=\"mailto:wolfgang.schwotzer@gmx.net\">"
           "Wolfgang Schwotzer</a><p>"
           "<a href=\"http://flexemu.neocities.org\">"
           "http://flexemu.neocities.org</a>")
        .arg(VERSION);
    ui.e_about->setOpenExternalLinks(true);
    ui.e_about->setHtml(aboutText);

    auto versionsText = tr("<b>FLEXplorer V%2</b><p>compiled for " OSTYPE
            ", uses:")
        .arg(VERSION);
    versionsText.append("<table>");
    for (const auto &version : FlexemuVersions::GetVersions())
    {
        std::stringstream stream;

        stream << "<tr><td>&#x2022;</td><td>" << version.first <<
            "</td><td>" << version.second + "</td></tr>";
        versionsText.append(stream.str().c_str());
    }
    versionsText.append("</table>");
    ui.e_versions->setOpenExternalLinks(true);
    ui.e_versions->setHtml(versionsText);

    dialog.exec();
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

void FLEXplorer::OnUpdateWindowMenu()
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

void FLEXplorer::OnCloseActiveSubWindow()
{
    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto path = child.GetPath();
        mdiArea->closeActiveSubWindow();
        SetStatusMessage(tr("Closed %1").arg(path));
    });
}

void FLEXplorer::OnCloseAllSubWindows()
{
    mdiArea->closeAllSubWindows();
    SetStatusMessage(tr("Closed all windows"));
}

FlexplorerMdiChild *FLEXplorer::CreateMdiChild(const QString &path,
        struct sFPOptions &p_options)
{

    auto *child = new FlexplorerMdiChild(path, p_options);

    auto *subWindow = mdiArea->addSubWindow(child);
    QString iconResource =
        (child->GetFlexDiskType() == DiskType::Directory) ?
            ":resource/dir.png" :
            ":resource/flexplorer.png";
    subWindow->setWindowIcon(QIcon(iconResource));
    subWindow->setMinimumSize(400, 400);
    subWindow->resize(540, 560);

    child->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(child, &FlexplorerMdiChild::customContextMenuRequested,
            this, &FLEXplorer::OnContextMenuRequested);
    connect(this, &FLEXplorer::FileTimeAccessHasChanged,
            child, &FlexplorerMdiChild::OnFileTimeAccessChanged);
    connect(child, &FlexplorerMdiChild::SelectionHasChanged,
            this, &FLEXplorer::OnSelectionHasChanged);
    connect(this, &FLEXplorer::FileSizeTypeHasChanged,
            child, &FlexplorerMdiChild::OnFileSizeTypeHasChanged);
    connect(this, &FLEXplorer::FileSizeTypeHasChanged,
            this, &FLEXplorer::UpdateSelectedFiles);

    return child;
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
QToolBar *FLEXplorer::CreateToolBar(QWidget *parent,
                                    const QString &title,
                                    const QString &objectName,
                                    const QSize &iconSize)
{
    auto *newToolBar = new QToolBar(title, parent);
    newToolBar->setObjectName(objectName);
    newToolBar->setFloatable(false);
    newToolBar->setMovable(false);
    newToolBar->setIconSize(iconSize);

    return newToolBar;
}

void FLEXplorer::CreateActions(const QSize &iconSize)
{
    toolBar = CreateToolBar(this, tr("ToolBar"), QStringLiteral("toolBar"),
            iconSize);
    assert(toolBar != nullptr);
    addToolBar(toolBar);

    CreateFileActions(*toolBar);
    CreateEditActions(*toolBar);
    CreateViewActions(*toolBar);
    CreateFlexDiskActions(*toolBar);
    CreateExtrasActions(*toolBar);
    CreateWindowsActions(*toolBar);
    CreateHelpActions(*toolBar);
}

void FLEXplorer::CreateFileActions(QToolBar &p_toolBar)
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    const auto exitIcon = QIcon(":/resource/exit.png");
    QAction *exitAction = fileMenu->addAction(exitIcon, tr("E&xit"),
                          this, SLOT(OnExit()));
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    p_toolBar.addAction(exitAction);
    p_toolBar.addSeparator();

    const auto newIcon = QIcon(":/resource/new.png");
    newFlexDiskAction = new QAction(newIcon, tr("&New Disk image..."), this);
    newFlexDiskAction->setShortcuts(QKeySequence::New);
    newFlexDiskAction->setStatusTip(tr("Create a new FLEX disk image file"));
    connect(newFlexDiskAction, &QAction::triggered,
            this, &FLEXplorer::OnNewFlexDisk);
    fileMenu->addAction(newFlexDiskAction);
    p_toolBar.addAction(newFlexDiskAction);

    const auto openIcon = QIcon(":/resource/open_con.png");
    openFlexDiskAction = new QAction(openIcon, tr("&Open Disk image..."),
            this);
    openFlexDiskAction->setShortcuts(QKeySequence::Open);
    openFlexDiskAction->setStatusTip(tr("Open a FLEX disk image file"));
    connect(openFlexDiskAction, &QAction::triggered,
            this, &FLEXplorer::OnOpenFlexDisk);
    fileMenu->addAction(openFlexDiskAction);
    p_toolBar.addAction(openFlexDiskAction);

    const auto openDirIcon = QIcon(":/resource/open_dir.png");
    openDirectoryAction = new QAction(openDirIcon, tr("Open &Directory..."),
                                      this);
    openDirectoryAction->setShortcut(QKeySequence(tr("Ctrl+D")));
    openDirectoryAction->setStatusTip(
            tr("Open a directory as FLEX disk image"));
    connect(openDirectoryAction, &QAction::triggered,
            this, &FLEXplorer::OnOpenDirectory);
    fileMenu->addAction(openDirectoryAction);
    p_toolBar.addAction(openDirectoryAction);

    recentDisksMenu = fileMenu->addMenu(tr("&Recent Disks"));
    CreateRecentDiskActionsFor(recentDisksMenu);
    recentDirectoriesMenu = fileMenu->addMenu(tr("R&ecent Directories"));
    CreateRecentDirectoryActionsFor(recentDirectoriesMenu);

    fileMenu->addSeparator();

    fileMenu->addAction(exitAction);
}

void FLEXplorer::CreateEditActions(QToolBar &p_toolBar)
{
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    const auto viewIcon = QIcon(":/resource/view.png");
    viewAction = new QAction(viewIcon, tr("&View..."), this);
    viewAction->setStatusTip(tr("View selected files"));
    auto font = viewAction->font();
    font.setBold(true);
    viewAction->setFont(font);
    connect(viewAction, &QAction::triggered, this, &FLEXplorer::OnViewSelected);
    editMenu->addAction(viewAction);
    p_toolBar.addSeparator();
    p_toolBar.addAction(viewAction);

    const auto deleteIcon = QIcon(":/resource/delete.png");
    deleteAction = new QAction(deleteIcon, tr("&Delete..."), this);
    deleteAction->setStatusTip(tr("Delete selected files"));
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered,
            this, &FLEXplorer::OnDeleteSelected);
    editMenu->addAction(deleteAction);
    p_toolBar.addAction(deleteAction);

    const auto attributesIcon = QIcon(":/resource/attributes.png");
    attributesAction = new QAction(attributesIcon, tr("&Attributes..."), this);
    attributesAction->setStatusTip(tr("Display and modify attributes of "
                                      "selected files"));
    connect(attributesAction, &QAction::triggered,
            this, &FLEXplorer::OnAttributesSelected);
    editMenu->addAction(attributesAction);
    editMenu->addSeparator();
    p_toolBar.addAction(attributesAction);
    p_toolBar.addSeparator();

    const auto injectIcon = QIcon(":/resource/inject.png");
    injectAction = new QAction(injectIcon, tr("&Inject..."), this);
    injectAction->setStatusTip(tr("Inject selected files"));
    injectAction->setShortcut(QKeySequence(tr("Ctrl+I")));
    connect(injectAction, &QAction::triggered, this,
            &FLEXplorer::OnInjectFiles);
    editMenu->addAction(injectAction);
    p_toolBar.addAction(injectAction);

    const auto extractIcon = QIcon(":/resource/extract.png");
    extractAction = new QAction(extractIcon, tr("E&xtract..."), this);
    extractAction->setStatusTip(tr("Extract selected files"));
    extractAction->setShortcut(QKeySequence(tr("Ctrl+X")));
    connect(extractAction, &QAction::triggered, this,
            &FLEXplorer::OnExtractSelected);
    editMenu->addAction(extractAction);
    editMenu->addSeparator();
    p_toolBar.addAction(extractAction);
    p_toolBar.addSeparator();

    const auto selectAllIcon = QIcon(":/resource/selectall.png");
    selectAllAction = new QAction(selectAllIcon, tr("Select &All"), this);
    selectAllAction->setStatusTip(tr("Select all files"));
    selectAllAction->setShortcuts(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this,
            &FLEXplorer::OnSelectAll);
    editMenu->addAction(selectAllAction);
    p_toolBar.addAction(selectAllAction);

    const auto deselectAllIcon = QIcon(":/resource/deselectall.png");
    deselectAllAction = new QAction(deselectAllIcon, tr("D&elect All"), this);
    deselectAllAction->setStatusTip(tr("Deselect all files"));
    deselectAllAction->setShortcut(QKeySequence(tr("Ctrl+E")));
    connect(deselectAllAction, &QAction::triggered, this,
            &FLEXplorer::OnDeselectAll);
    editMenu->addAction(deselectAllAction);
    p_toolBar.addAction(deselectAllAction);

    const auto findFilesIcon = QIcon(":/resource/find.png");
    findFilesAction = new QAction(findFilesIcon, tr("&Find Files..."), this);
    findFilesAction->setShortcut(QKeySequence::Find);
    findFilesAction->setStatusTip(tr("Find files by filename pattern"));
    connect(findFilesAction, &QAction::triggered, this,
            &FLEXplorer::OnFindFiles);
    editMenu->addAction(findFilesAction);
    p_toolBar.addAction(findFilesAction);
    p_toolBar.addSeparator();

#ifndef QT_NO_CLIPBOARD
    editMenu->addSeparator();
    const auto copyIcon = QIcon(":/resource/copy.png");
    copyAction = new QAction(copyIcon, tr("&Copy"), this);
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setStatusTip(tr("Copy selected files to the clipboard"));
    connect(copyAction, &QAction::triggered, this, &FLEXplorer::OnCopy);
    editMenu->addAction(copyAction);
    p_toolBar.addAction(copyAction);

    const auto pasteIcon = QIcon(":/resource/paste.png");
    pasteAction = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAction->setShortcuts(QKeySequence::Paste);
    pasteAction->setStatusTip(tr("Paste files from the clipboard"));
    connect(pasteAction, &QAction::triggered, this, &FLEXplorer::OnPaste);
    editMenu->addAction(pasteAction);
    p_toolBar.addAction(pasteAction);
    p_toolBar.addSeparator();
#endif
}

void FLEXplorer::CreateViewActions(QToolBar &/* p_toolBar */)
{
    auto *viewMenu = menuBar()->addMenu(tr("&View"));

    auto *iconSizeMenu = viewMenu->addMenu(tr("&Icon Size"));

    for (uint16_t index = 0U; index < ICON_SIZES; ++index)
    {
        iconSizeAction[index] = CreateIconSizeAction(*iconSizeMenu, index);
        connect(iconSizeAction[index], &QAction::triggered,
            this, [&,index](){ OnIconSize(index); });
    }
}

void FLEXplorer::CreateFlexDiskActions(QToolBar &p_toolBar)
{
    QMenu *containerMenu = menuBar()->addMenu(tr("&Disk"));

    const auto infoIcon = QIcon(":/resource/info.png");
    infoAction = new QAction(infoIcon, tr("&Info..."), this);
    infoAction->setShortcut(QKeySequence(tr("Ctrl+Shift+I")));
    infoAction->setStatusTip(tr("Show disk image properties"));
    connect(infoAction, &QAction::triggered, this, &FLEXplorer::OnInfo);
    containerMenu->addAction(infoAction);
    p_toolBar.addAction(infoAction);
}

void FLEXplorer::CreateExtrasActions(QToolBar &p_toolBar)
{
    QMenu *extrasMenu = menuBar()->addMenu(tr("&Extras"));

    const auto optionsIcon = QIcon(":/resource/options.png");
    optionsAction = new QAction(optionsIcon, tr("&Options..."), this);
    optionsAction->setStatusTip(tr("Display and modify application's options"));
    connect(optionsAction, &QAction::triggered, this, &FLEXplorer::OnOptions);
    extrasMenu->addAction(optionsAction);
    p_toolBar.addSeparator();
    p_toolBar.addAction(optionsAction);
}

void FLEXplorer::CreateWindowsActions(QToolBar &p_toolBar)
{
    windowMenu = menuBar()->addMenu(tr("&Window"));

    connect(windowMenu, &QMenu::aboutToShow,
            this, &FLEXplorer::OnUpdateWindowMenu);

    const auto closeIcon = QIcon(":/resource/window-close.png");
    closeAction = new QAction(closeIcon, tr("Cl&ose"), this);
    closeAction->setStatusTip(tr("Close the active window"));
    connect(closeAction, &QAction::triggered,
            this, &FLEXplorer::OnCloseActiveSubWindow);
    p_toolBar.addSeparator();
    p_toolBar.addAction(closeAction);

    closeAllAction = new QAction(tr("Close &All"), this);
    closeAllAction->setStatusTip(tr("Close all the windows"));
    connect(closeAllAction, &QAction::triggered,
            this, &FLEXplorer::OnCloseAllSubWindows);

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

    OnUpdateWindowMenu();
}

void FLEXplorer::CreateHelpActions(QToolBar &p_toolBar)
{
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    const auto aboutIcon = QIcon(":/resource/about.png");
    aboutAction = helpMenu->addAction(aboutIcon, tr("&About"),
                                      this, SLOT(OnAbout()));
    aboutAction->setStatusTip(tr("Show the application's About box"));
    p_toolBar.addSeparator();
    p_toolBar.addAction(aboutAction);

    const auto aboutQtIcon = QIcon(":/resource/qt.png");
    aboutQtAction = helpMenu->addAction(aboutQtIcon, tr("&About Qt"),
                                      qApp, SLOT(aboutQt()));
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    p_toolBar.addAction(aboutQtAction);
}

void FLEXplorer::OnContextMenuRequested(QPoint pos)
{
    assert(selectAllAction);

    ExecuteInChild([&](FlexplorerMdiChild &child)
    {
        auto *contextMenu = new QMenu(this);

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

void FLEXplorer::OnExit()
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

void FLEXplorer::OnSubWindowActivated(QMdiSubWindow *window)
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
        const auto format = (selectedFilesCount == 1) ?
                tr("%1 File selected") : tr("%1 Files selected");

        auto text = format.arg(selectedFilesCount);
        l_selectedFilesCount->setText(text);
        text = tr("%1 Byte").arg(selectedFilesByteSize);
        l_selectedFilesByteSize->setText(text);
    });
}

void FLEXplorer::OnSelectionHasChanged()
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
        const auto fileExtension = QString(flx::getFileExtension(
                        path.toStdString()).c_str()).toLower();

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
            if (!OpenFlexDiskForPath(path, isLast))
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

    for (auto i = 0; i < sFPOptions::maxRecentFiles; ++i)
    {
        // Create all actions, de/activate them by setVisible() false/true.
        action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this,
                &FLEXplorer::OnOpenRecentDisk);
        recentDiskActions.append(action);
        menu->addAction(action);
    }

    recentDisksClearAllAction = new QAction(tr("Clear List"), this);
    recentDisksClearAllAction->setVisible(false);
    connect(recentDisksClearAllAction, &QAction::triggered, this,
            &FLEXplorer::OnClearAllRecentDiskEntries);
    menu->addSeparator();
    menu->addAction(recentDisksClearAllAction);

    UpdateRecentDiskActions();
}

// Always call this function when recentDiskPaths has changed or the
// recentDiskActions have been initialized.
void FLEXplorer::UpdateRecentDiskActions() const
{
    QStringList::size_type idx = 0;
    for (const auto &path : recentDiskPaths)
    {
        auto idxString = QString("%1. ").arg(idx + 1);
        const QIcon icon(":/resource/open_con.png");
        const auto strippedPath = StripPath(path);

        recentDiskActions.at(idx)->setText(idxString + strippedPath);
        recentDiskActions.at(idx)->setData(path);
        recentDiskActions.at(idx)->setVisible(true);
        recentDiskActions.at(idx)->setIcon(icon);
        ++idx;
    }

    for (idx = recentDiskPaths.size(); idx < sFPOptions::maxRecentFiles; ++idx)
    {
        recentDiskActions.at(idx)->setVisible(false);
    }

    recentDisksClearAllAction->setVisible(!recentDiskPaths.isEmpty());
}

void FLEXplorer::OnClearAllRecentDiskEntries()
{
    recentDiskPaths.clear();
    options.recentDiskPaths.clear();

    UpdateRecentDiskActions();
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
    while (recentDiskPaths.size() > sFPOptions::maxRecentFiles)
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

        if (fileInfo.exists() && fileInfo.isFile())
        {
            recentDiskPaths.push_back(path.c_str());
        }
    }

    UpdateRecentDiskActions();
}

void FLEXplorer::CreateRecentDirectoryActionsFor(QMenu *menu)
{
    QAction *action = nullptr;

    for (auto i = 0; i < sFPOptions::maxRecentDirectories; ++i)
    {
        // Create all actions, de/activate them by setVisible() false/true.
        action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this,
                &FLEXplorer::OnOpenRecentDirectory);
        recentDirectoryActions.append(action);
        menu->addAction(action);
    }

    recentDirectoriesClearAllAction = new QAction(tr("Clear List"), this);
    recentDirectoriesClearAllAction->setVisible(false);
    connect(recentDirectoriesClearAllAction, &QAction::triggered, this,
            &FLEXplorer::OnClearAllRecentDirectoryEntries);
    menu->addSeparator();
    menu->addAction(recentDirectoriesClearAllAction);

    UpdateRecentDirectoryActions();
}

// Always call this function when recentDirectoryPaths has changed or the
// recentDirectoryActions have been initialized.
void FLEXplorer::UpdateRecentDirectoryActions() const
{
    QStringList::size_type idx = 0;
    for (const auto &path : recentDirectoryPaths)
    {
        const auto idxString = QString("%1. ").arg(idx + 1);
        const QIcon icon(":/resource/open_dir.png");
        const auto strippedPath = StripPath(path);

        recentDirectoryActions.at(idx)->setText(idxString + strippedPath);
        recentDirectoryActions.at(idx)->setData(path);
        recentDirectoryActions.at(idx)->setVisible(true);
        recentDirectoryActions.at(idx)->setIcon(icon);
        ++idx;
    }

    for (idx = recentDirectoryPaths.size();
            idx < sFPOptions::maxRecentDirectories;
            ++idx)
    {
        recentDirectoryActions.at(idx)->setVisible(false);
    }

    auto isEmpty = recentDirectoryPaths.isEmpty();
    recentDirectoriesClearAllAction->setVisible(!isEmpty);
}

void FLEXplorer::OnClearAllRecentDirectoryEntries()
{
    recentDirectoryPaths.clear();
    options.recentDirectoryPaths.clear();

    UpdateRecentDirectoryActions();
}

void FLEXplorer::DeleteRecentDirectoryActions()
{
    while (!recentDirectoryActions.isEmpty())
    {
        delete recentDirectoryActions.takeLast();
    }
}

// Call this when a directory has been successfully "loaded".
void FLEXplorer::UpdateForRecentDirectory(const QString &path)
{
    recentDirectoryPaths.removeAll(path);
    recentDirectoryPaths.prepend(path);
    while (recentDirectoryPaths.size() > sFPOptions::maxRecentDirectories)
    {
        recentDirectoryPaths.removeLast();
    }
    options.recentDirectoryPaths.clear();
    for (const auto &directoryPath : recentDirectoryPaths)
    {
        options.recentDirectoryPaths.push_back(directoryPath.toStdString());
    }

    UpdateRecentDirectoryActions();
}

void FLEXplorer::RestoreRecentDirectories()
{
    recentDirectoryPaths.clear();
    for (const auto &path : options.recentDirectoryPaths)
    {
        const QFileInfo fileInfo(path.c_str());

        if (fileInfo.exists() && fileInfo.isDir())
        {
            recentDirectoryPaths.push_back(path.c_str());
        }
    }

    UpdateRecentDirectoryActions();
}

void FLEXplorer::OnIconSize(int index)
{
    index = std::max(index, 0);
    index = std::min(index, 2);

    int size = 16 + 8 * index;
    const QSize iconSize({size, size});

    SetIconSize(iconSize);
    SetIconSizeCheck(iconSize);

    options.iconSize = size;
}

void FLEXplorer::SetIconSize(const QSize &iconSize)
{
    toolBar->setIconSize(iconSize);
}

void FLEXplorer::SetIconSizeCheck(const QSize &iconSize)
{
    const int sizeIndex = IconSizeToIndex(iconSize);

    for (int index = 0; index < ICON_SIZES; ++index)
    {
        auto *action = iconSizeAction[index];
        assert(action != nullptr);
        action->setChecked(index == sizeIndex);
    }
}

