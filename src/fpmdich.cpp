/*
    fpmdich.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
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
#include "fcinfo.h"
#include "fdirent.h"
#include "filecntb.h"
#include "flexerr.h"
#include "cvtwchar.h"
#include "bdir.h"
#include "bprocess.h"
#include "warnoff.h"
#include <QFileInfo>
#include <QFile>
#include <QDate>
#include <QDrag>
#include <QMouseEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QClipboard>
#include <QApplication>
#include <QDir>
#include <QProgressDialog>
#include "warnon.h"
#include <cassert>
#include <string>
#include "fcopyman.h"
#include "fpdnd.h"
#include "fpmodel.h"
#include "fpedit.h"
#include "fpmdich.h"
#include "sfpopts.h"
#include "fpcnvui.h"
#include "qtfree.h"

const QString FlexplorerMdiChild::mimeTypeFlexDiskImageFile =
                                      "application/x-flexdiskimagefile";

FlexplorerMdiChild::FlexplorerMdiChild(const QString &path,
                                       struct sFPOptions &p_options) :
    dragStartPosition(0,0),
    selectedFilesCount(0), selectedFilesByteSize(0),
    options(p_options)
{
    SetupModel(path);
    SetupView();

    auto title = GetUserFriendlyPath();
    title += (IsWriteProtected() ? tr(" [read-only]") : "");
    setWindowTitle(title);

    // Set column for unique Id hidden.
    setColumnHidden(FlexplorerTableModel::COL_ID, true);

    dateDelegate.reset(new FlexDateDelegate(options.ft_access, this));
    dateTimeDelegate.reset(new FlexDateTimeDelegate(options.ft_access, this));
    UpdateDateDelegate();
    filenameDelegate.reset(new FlexFilenameDelegate(this));
    setItemDelegateForColumn(FlexplorerTableModel::COL_FILENAME,
                             filenameDelegate.get());
    attributesDelegate.reset(
            new FlexAttributesDelegate(GetSupportedAttributes(), this));
    setItemDelegateForColumn(FlexplorerTableModel::COL_ATTRIBUTES,
                             attributesDelegate.get());
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::DoubleClicked |
                    QAbstractItemView::EditKeyPressed);

    // Although sorting not yet enabled this initiates a first sorting.
    horizontalHeader()->setSortIndicator(FlexplorerTableModel::COL_FILENAME,
                                         Qt::AscendingOrder);
    setSortingEnabled(true);
    model->UpdateFileSizeHeaderName();
}

FlexplorerMdiChild::~FlexplorerMdiChild()
{
}

QString FlexplorerMdiChild::GetPath() const
{
    assert(model);
    return model->GetPath();
}

QString FlexplorerMdiChild::GetUserFriendlyPath() const
{
    assert(model);
    return model->GetUserFriendlyPath();
}

bool FlexplorerMdiChild::IsWriteProtected() const
{
    assert(model);
    return model->IsWriteProtected();
}

int FlexplorerMdiChild::GetContainerType() const
{
    assert(model);
    return model->GetContainerType();
}

QString FlexplorerMdiChild::GetSupportedAttributes() const
{
    assert(model);
    return QString(model->GetSupportedAttributes().c_str());
}

int FlexplorerMdiChild::GetSelectedFilesCount() const
{
    return selectedFilesCount;
}

int FlexplorerMdiChild::GetSelectedFilesByteSize() const
{
    return selectedFilesByteSize;
}

void FlexplorerMdiChild::SelectAll()
{
    selectAll();
}

void FlexplorerMdiChild::DeselectAll()
{
    clearSelection();
    selectedFilesCount = 0;
    selectedFilesByteSize = 0;
}

QVector<int>::size_type FlexplorerMdiChild::FindFiles(const QString &pattern)
{
    assert(model);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    auto rowIndices = model->FindFiles(pattern);
    MultiSelect(rowIndices);
    QApplication::restoreOverrideCursor();

    return rowIndices.count();
}

QVector<int>::size_type FlexplorerMdiChild::DeleteSelected()
{
    auto selectedRows = selectionModel()->selectedRows();
    QProgressDialog progress(tr("Delete files ..."), tr("&Cancel"), 0,
                             cast_from_qsizetype(selectedRows.size() - 1), this);
    int count = 0;

    progress.show();
    std::reverse(selectedRows.begin(), selectedRows.end());

    for (auto &index : selectedRows)
    {
        QApplication::processEvents();
        progress.setValue(count);
        if (progress.wasCanceled())
        {
            break;
        }

        try
        {
            model->DeleteFile(index);
            ++count;
        }
        catch (FlexException &ex)
        {
            auto buttons = QMessageBox::Ok | QMessageBox::Cancel;
            auto result = QMessageBox::critical(this, tr("FLEXPlorer Error"),
                                                ex.what(), buttons,
                                                QMessageBox::Ok);

            if (result != QMessageBox::Ok)
            {
                break;
            }

            continue;
        }
    }

    return count;
}

QVector<int>::size_type FlexplorerMdiChild::InjectFiles(
                        const QStringList &filePaths)
{
    QProgressDialog progress(tr("Inject files ..."), tr("&Cancel"), 0,
                             cast_from_qsizetype(filePaths.size() - 1), this);
    QVector<int> rowIndices;
    auto index = 0;

    progress.show();
    for (const auto &path : filePaths)
    {
        FlexFileBuffer buffer;

        QApplication::processEvents();
        progress.setValue(index++);
        if (progress.wasCanceled())
        {
            break;
        }

        const auto filePath = QDir::toNativeSeparators(path);
        if (!buffer.ReadFromFile(filePath.toUtf8().data()))
        {
            auto msg = tr("Error reading from\n%1\nInjection aborted.");
            msg = msg.arg(filePath);
            QMessageBox::warning(this, tr("FLEXplorer warning"), msg);
            continue;
        }

        if (buffer.IsTextFile())
        {
            if (options.injectTextFileAskUser)
            {
                QDialog dialog(this);
                FlexplorerConvertUi ui;

                ui.setupUi(dialog);
                ui.TransferDataToDialog(tr("Inject text file"),
                                        filePath,
                                        tr("Convert to FLEX text file"),
                                        options.injectTextFileConvert,
                                        options.injectTextFileAskUser);
                dialog.adjustSize();
                auto result = dialog.exec();

                if (result == QDialog::Accepted)
                {
                    options.injectTextFileConvert = ui.GetConvert();
                    options.injectTextFileAskUser = ui.GetAskUser();
                }
                else
                {
                    break;
                }
            }

            if (options.injectTextFileConvert)
            {
                buffer.ConvertToFlexTextFile();
            }
        }

        try
        {
            auto rowIndicesFound = model->FindFiles(buffer.GetFilename());
            if (!rowIndicesFound.isEmpty())
            {
                auto msg = tr("%1\nalready exists. Overwrite?");

                msg = msg.arg(buffer.GetFilename());
                auto answer = QMessageBox::question(this, "FLEXplorer", msg,
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes);

                if (answer == QMessageBox::Yes)
                {
                    auto modelIndex = model->index(rowIndicesFound[0], 0);
                    model->DeleteFile(modelIndex);
                }
                else
                {
                    continue;
                }
            }

            rowIndices.append(model->AddFile(buffer).row());
        }
        catch (FlexException &ex)
        {
            auto msg = tr("%1\nInjection aborted.\nContinue?");

            msg = msg.arg(ex.what());
            auto buttons = QMessageBox::Yes | QMessageBox::No;
            auto answer = QMessageBox::critical(this, tr("FLEXPlorer Error"),
                                                msg, buttons,
                                                QMessageBox::Yes);

            if (answer == QMessageBox::No)
            {
                break;
            }
        }
    }

    MultiSelect(rowIndices);

    return rowIndices.count();
}

QVector<int>::size_type FlexplorerMdiChild::ExtractSelected(
                        const QString &targetDirectory)
{
    auto selectedRows = selectionModel()->selectedRows();
    QProgressDialog progress(tr("Extract files ..."), tr("&Cancel"), 0,
                             cast_from_qsizetype(selectedRows.size() - 1), this);
    QVector<int>::size_type count = 0;

    progress.show();
    for (auto &index : selectedRows)
    {
        QApplication::processEvents();
        progress.setValue(cast_from_qsizetype(count));
        if (progress.wasCanceled())
        {
            break;
        }

        try
        {
            QString filename = model->GetFilename(index);
            auto buffer = model->CopyFile(index);

            if (buffer.IsFlexTextFile())
            {
                if (options.extractTextFileAskUser)
                {
                    QDialog dialog(this);
                    FlexplorerConvertUi ui;

                    ui.setupUi(dialog);
                    ui.TransferDataToDialog(tr("Extract text file"),
                                            filename,
                                            tr("Convert to host text file"),
                                            options.extractTextFileConvert,
                                            options.extractTextFileAskUser);
                    dialog.adjustSize();
                    auto result = dialog.exec();

                    if (result == QDialog::Accepted)
                    {
                        options.extractTextFileConvert = ui.GetConvert();
                        options.extractTextFileAskUser = ui.GetAskUser();
                    }
                    else
                    {
                        break;
                    }
                }

                if (options.extractTextFileConvert)
                {
                    buffer.ConvertToTextFile();
                }
            }

            auto targetFilename = filename.toLower();
            auto targetPath = targetDirectory + PATHSEPARATORSTRING +
                              targetFilename;

            if (QFile::exists(targetPath))
            {
                auto msg = tr("%1\nalready exists. Overwrite it?");

                msg = msg.arg(targetPath);
                auto answer = QMessageBox::question(this, "FLEXplorer", msg,
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes);
                if (answer != QMessageBox::Yes)
                {
                    continue;
                }
                if (!QFile::remove(targetPath))
                {
                    msg = tr("Cannot remove\n%1\nExtraction aborted.");
                    msg = msg.arg(targetPath);
                    QMessageBox::warning(this, tr("FLEXplorer warning"), msg);
                    continue;
                }
            }

            if (!buffer.WriteToFile(targetPath.toUtf8().data()))
            {
                throw FlexException(FERR_UNABLE_TO_CREATE,
                                    targetFilename.toUtf8().data());
            }

            ++count;
        }
        catch (FlexException &ex)
        {
            auto msg = tr("%1\nExtraction aborted.\nContinue?");

            msg = msg.arg(ex.what());
            auto buttons = QMessageBox::Yes | QMessageBox::No;
            auto answer = QMessageBox::critical(this, tr("FLEXPlorer Error"),
                                                msg, buttons,
                                                QMessageBox::Yes);

            if (answer == QMessageBox::No)
            {
                break;
            }
        }
    }

    return count;
}

QVector<int>::size_type FlexplorerMdiChild::ViewSelected()
{
    auto selectedRows = selectionModel()->selectedRows();
    QVector<int>::size_type count = 0;

    for (auto &index : selectedRows)
    {
        auto filename = model->GetFilename(index).toStdString();
        strlower(filename);

        try
        {
            auto buffer = model->CopyFile(index);

            if (buffer.IsFlexTextFile())
            {
                buffer.ConvertToTextFile();
            }
            else
            {
                buffer.ConvertToDumpFile(16);
            }

#ifdef _WIN32
            // Windows ShellExtensions works best with a
            // well known file extension.
            if (getFileExtension(filename.c_str()) != ".txt")
            {
                filename += ".txt";
            }
#endif

            auto tempPath = getTempPath() + PATHSEPARATORSTRING "flexplorer";

            if (!BDirectory::Exists(tempPath))
            {
                if (!BDirectory::Create(tempPath))
                {
                    throw FlexException(FERR_UNABLE_TO_CREATE, tempPath);
                }
            }

            tempPath += PATHSEPARATORSTRING +
                        getFileName(GetPath().toUtf8().data());
            if (!BDirectory::Exists(tempPath))
            {
                if (!BDirectory::Create(tempPath))
                {
                    throw FlexException(FERR_UNABLE_TO_CREATE, tempPath);
                }
            }

            std::string tempFile(tempPath);
            tempFile.append(PATHSEPARATORSTRING).append(filename);

            if (buffer.WriteToFile(tempFile.c_str()))
            {
#ifdef _WIN32
                SHELLEXECUTEINFO execInfo;
                std::wstring wTempFile = ConvertToUtf16String(tempFile);

                memset(&execInfo, 0, sizeof(execInfo));
                execInfo.cbSize = sizeof(execInfo);
                execInfo.lpVerb = L"open";
                execInfo.lpFile = wTempFile.c_str();
                execInfo.lpDirectory = L".";
                execInfo.nShow = SW_SHOWNORMAL;

                if (ShellExecuteEx(&execInfo))
                {
                    ++count;
                }
                else
                {
                    throw FlexException(GetLastError(),
                        std::string("In function ViewSelectedItems()."));
                }
#else
                // On Unix/Linux the mime type is used depending
                // on the file contents. It can have any file extension.
                BProcess process("xdg-open", ".");

                process.AddArgument(tempFile);

                if (process.Start())
                {
                    ++count;
                }
                else
                {
                    throw FlexException(FERR_CREATE_PROCESS,
                                        "xdg-open", tempFile);
                }
#endif
            }
            else
            {
                throw FlexException(FERR_UNABLE_TO_CREATE, tempFile);
            }
        }
        catch (FlexException &ex)
        {
            QMessageBox::critical(this, tr("FLEXplorer Error"), ex.what());
        }
    }

    return count;
}

QVector<Byte> FlexplorerMdiChild::GetSelectedAttributes() const
{
    return model->GetAttributes(selectionModel()->selectedRows());
}

QVector<QString> FlexplorerMdiChild::GetSelectedFilenames() const
{
    return model->GetFilenames(selectionModel()->selectedRows());
}

int FlexplorerMdiChild::SetSelectedAttributes(Byte setMask, Byte clearMask)
{
    int count = 0;

    for (auto &index : selectionModel()->selectedRows())
    {
        Byte oldAttributes;
        auto filename = model->GetFilename(index);

        if (model->GetAttributes(index, oldAttributes))
        {
            FlexDirEntry dirEntry;
            dirEntry.SetAttributes(oldAttributes);
            dirEntry.SetAttributes(setMask, clearMask);
            auto newAttributes = dirEntry.GetAttributes();

            if (oldAttributes == newAttributes)
            {
                continue;
            }

            QString attributesString =
                dirEntry.GetAttributesString().c_str();
            auto attribIndex = model->index(index.row(),
                                      FlexplorerTableModel::COL_ATTRIBUTES);
            try
            {
                model->SetAttributesString(attribIndex, attributesString);
                ++count;
            }
            catch (FlexException &ex)
            {
                auto message = QString(ex.what());

                message += ".\nContinue?";
                auto result = QMessageBox::critical(this,
                                  tr("FLEXPlorer Error"), message,
                                  QMessageBox::Yes | QMessageBox::No);

                if (result == QMessageBox::No)
                {
                    break;
                }
            }
        }
    }

    return count;
}

void FlexplorerMdiChild::Info()
{
    FlexContainerInfo info;

    try
    {
        info = model->GetContainerInfo();
    }
    catch (FlexException &ex)
    {
        QMessageBox::critical(this, tr("FLEXPlorer Error"), ex.what());
        return;
    }

    int tracks;
    int sectors;
    info.GetTrackSector(tracks, sectors);

    auto title = tr("Disk image %1 #%2")
        .arg(info.GetName().c_str())
        .arg(info.GetNumber());

    QString str = tr("Path: ");
    str += QString(info.GetPath().c_str()).append("\n");
    str += tr("Type: ");
    str += QString(info.GetTypeString().c_str()).append("\n");
    str += tr("Date: ");
    const auto &date = info.GetDate();
    QDate qdate(date.GetYear(), date.GetMonth(), date.GetDay());
    str += qdate.toString().append("\n");
    if (tracks != 0 && sectors != 0)
    {
        str += tr("Tracks: ");
        str += QString::number(tracks).append("\n");
        str += tr("Sectors: ");
        str += QString::number(sectors).append("\n");
    }
    str += tr("Size: ");
    str += QString::number(info.GetTotalSize() / 1024);
    str += tr(" KByte").append("\n");
    str += tr("Free: ");
    str += QString::number(info.GetFree() / 1024);
    str += tr(" KByte").append("\n");

    if (info.GetAttributes() & FLX_READONLY)
    {
        str += tr("Attributes: read-only").append("\n");
    }
    if (info.GetType() & TYPE_DSK_CONTAINER)
    {
        auto header = info.GetJvcFileHeader();

        str += tr("JVC header: ");
        if (header.empty())
        {
            str += tr("none");
        }
        else
        {
            bool isAppend = false;
            for (const auto value : header)
            {
                str += (isAppend ? "," : "");
                str += QString::number((Word)value);
                isAppend = true;
            }
        }
    }

    QMessageBox::information(this, title, str);
}

void FlexplorerMdiChild::SetupModel(const QString &path)
{
    model.reset(
        new FlexplorerTableModel(path.toUtf8().data(), options, this));
    model->Initialize();
    setModel(model.get());
}

void FlexplorerMdiChild::SetupView()
{
    assert(model);
    setSelectionModel(new QItemSelectionModel(model.get()));
    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &FlexplorerMdiChild::SelectionChanged);
    connect(this, &FlexplorerMdiChild::activated,
            this, &FlexplorerMdiChild::IsActivated);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    // QHeaderView::ResizeToContents would always optimize the column size
    // based on it's contents but for large tables it is much too slow.
    // Instead the column width is set by a simple heuristic and also can be
    // adapted by the user.
    ResizeColumns();
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // For vertical headers the minimum size (=height) is sufficient.
    auto size = verticalHeader()->minimumSectionSize();
    verticalHeader()->setDefaultSectionSize(size);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    setAcceptDrops(true);
}

// Intentionally do not override QAbstractItemView::selectionChanged().
// NOLINTNEXTLINE(bugprone-virtual-near-miss)
void FlexplorerMdiChild::SelectionChanged(
        const QItemSelection &selected,
        const QItemSelection &deselected)
{
    auto updateSelectedFilesFct = [&](const QModelIndex &index, int pmOne)
    {
        if (index.column() == FlexplorerTableModel::COL_SIZE)
        {
            selectedFilesCount += pmOne;
            auto variant = model->data(index, Qt::DisplayRole);
            selectedFilesByteSize += pmOne * variant.toInt();
        }
    };

    for (auto &index : selected.indexes())
    {
        updateSelectedFilesFct(index, +1);
    }

    for (auto &index : deselected.indexes())
    {
        updateSelectedFilesFct(index, -1);
    }

    emit SelectionHasChanged();
}

void FlexplorerMdiChild::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragStartPosition = event->pos();
    }

    QTableView::mousePressEvent(event);
}

void FlexplorerMdiChild::mouseMoveEvent(QMouseEvent *event)
{
    auto distance = (event->pos() - dragStartPosition).manhattanLength();

    if ((event->buttons() & Qt::LeftButton) &&
        distance >= QApplication::startDragDistance())
    {
        // Starting a mouse drag.
        BeginDrag();
    }
}

void FlexplorerMdiChild::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(mimeTypeFlexDiskImageFile) &&
       !IsWriteProtected())
    {
        if (event->source() != this)
        {
            event->acceptProposedAction();
        }
        return;
    }

    event->ignore();
}

void FlexplorerMdiChild::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(mimeTypeFlexDiskImageFile) &&
       !IsWriteProtected())
    {
        if (event->source() != this)
        {
            event->acceptProposedAction();
        }
        return;
    }

    event->ignore();
}

void FlexplorerMdiChild::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(mimeTypeFlexDiskImageFile) &&
       !IsWriteProtected())
    {
        if (event->source() != this)
        {
            PasteFrom(*event->mimeData());
            event->acceptProposedAction();
        }
        return;
    }

    event->ignore();
}

QMimeData *FlexplorerMdiChild::GetMimeDataForSelected(int *count)
{
    if (count != nullptr)
    {
        *count = 0;
    }

    auto selectedRows = selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
    {
        return nullptr;
    }

    FlexDnDFiles files(GetPath().toUtf8().data(), getHostName());

    for (const auto &index : selectedRows)
    {
        try
        {
            files.Add(model->CopyFile(index));
            if (count != nullptr)
            {
                ++(*count);
            }
        }
        catch (FlexException &ex)
        {
            auto message = QString(ex.what());

            message += ".\nContinue?";
            auto result = QMessageBox::critical(this, tr("FLEXPlorer Error"),
                              message, QMessageBox::Yes | QMessageBox::No);

            if (result == QMessageBox::No)
            {
                return nullptr;
            }
        }
    }

    QMimeData *mimeData = new QMimeData;

    if (files.GetFileCount() != 0)
    {
        QByteArray itemData;
        itemData.resize(files.GetFileSize());
        files.WriteDataTo(reinterpret_cast<Byte *>(itemData.data()));
        mimeData->setData(mimeTypeFlexDiskImageFile, itemData);
    }

    QStringList mimeTypes { "text/plain", "text/csv", "text/html" };
    for (const auto &mimeType : mimeTypes)
    {
        QString text = model->AsText(selectedRows, mimeType);
        mimeData->setData(mimeType, text.toUtf8().data());
    }

    return mimeData;
}

void FlexplorerMdiChild::BeginDrag()
{
    auto *mimeData = GetMimeDataForSelected();

    if (mimeData != nullptr)
    {
        auto iconSize = QSize(64, 64);
        auto icon = QIcon(":/resource/diskimagefiles.png");
        auto pixmap = icon.pixmap(iconSize, QIcon::Normal, QIcon::On);
        auto *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(0, 0));
        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction, Qt::CopyAction);
    }
}

#ifndef QT_NO_CLIPBOARD
int FlexplorerMdiChild::CopySelectedToClipboard()
{
    int count = 0;
    auto *clipboard = QApplication::clipboard();

    if (clipboard != nullptr)
    {
        auto *mimeData = GetMimeDataForSelected(&count);

        if (mimeData != nullptr)
        {
            clipboard->setMimeData(mimeData);
        }
    }

    return count;
}
#endif

int FlexplorerMdiChild::PasteFrom(const QMimeData &mimeData)
{
    int count = 0;
    FlexDnDFiles files;

    if (!mimeData.hasFormat(mimeTypeFlexDiskImageFile))
    {
        return count;
    }

    QByteArray itemData = mimeData.data(mimeTypeFlexDiskImageFile);

    files.ReadDataFrom(reinterpret_cast<Byte *>(itemData.data()));

    if (getHostName() == files.GetDnsHostName() &&
            isPathsEqual(GetPath().toUtf8().data(), files.GetPath()))
    {
        return count;
    }

    setSortingEnabled(false);
    for (auto fileIndex = 0U; fileIndex < files.GetFileCount(); ++fileIndex)
    {
        try
        {
            const auto &buffer = files.GetBufferAt(fileIndex);
            auto index = model->AddFile(buffer);
            if (index.isValid())
            {
                ++count;
            }
        }
        catch (FlexException &ex)
        {
            if (ex.GetErrorCode() == FERR_DISK_FULL_WRITING ||
                ex.GetErrorCode() == FERR_DIRECTORY_FULL)
            {
                QMessageBox::critical(this, tr("FLEXPlorer Error"),
                                      ex.what(), QMessageBox::Ok);
                setSortingEnabled(true);

                return count;
            }

            QMessageBox::warning(this, tr("FLEXPlorer Error"),
                                 ex.what(), QMessageBox::Ok);
        }
    }
    setSortingEnabled(true);

    return count;
}

#ifndef QT_NO_CLIPBOARD
int FlexplorerMdiChild::PasteFromClipboard()
{
    auto *clipboard = QApplication::clipboard();

    if (clipboard != nullptr)
    {
        const auto *mimeData = clipboard->mimeData();

        if (mimeData != nullptr &&
            mimeData->hasFormat(mimeTypeFlexDiskImageFile))
        {
            return PasteFrom(*mimeData);
        }
    }

    return 0;
}
#endif

void FlexplorerMdiChild::IsActivated(const QModelIndex &
        /* [[maybe_unused]] const QModelIndex &index */)
{
    ViewSelected();
}

void FlexplorerMdiChild::OnFileTimeAccessChanged()
{
    UpdateDateDelegate();
    for (int row = 0; row < model->rowCount(); ++row)
    {
        update(model->index(row, FlexplorerTableModel::COL_DATE));
    }
    ResizeColumn(FlexplorerTableModel::COL_DATE,
                 model->GetColumnMaxStrings()[FlexplorerTableModel::COL_DATE]);
}

void FlexplorerMdiChild::UpdateDateDelegate()
{
    if (options.ft_access == FileTimeAccess::NONE)
    {
        setItemDelegateForColumn(FlexplorerTableModel::COL_DATE,
                                 dateDelegate.get());
    }
    else
    {
        setItemDelegateForColumn(FlexplorerTableModel::COL_DATE,
                                 dateTimeDelegate.get());
    }
}

void FlexplorerMdiChild::OnFileSizeTypeHasChanged()
{
    static const int ssm4 = SECTOR_SIZE - 4;

    model->UpdateFileSizeColumn();
    model->UpdateFileSizeHeaderName();

    if (options.fileSizeType == FileSizeType::FileSize)
    {
        selectedFilesByteSize = selectedFilesByteSize / ssm4 * SECTOR_SIZE;
    }
    else if (options.fileSizeType == FileSizeType::DataSize)
    {
        selectedFilesByteSize = selectedFilesByteSize / SECTOR_SIZE * ssm4;
    }
}

void FlexplorerMdiChild::MultiSelect(const QVector<int> &rowIndices)
{
    // Multi selection with iterating selectRow(rowIndex) is much too slow.
    // Instead using QItemSelection works.
    QItemSelection allSelections;
    clearSelection();
    auto selection = selectionMode();
    setSelectionMode(QAbstractItemView::MultiSelection);

    for (auto rowIndex : rowIndices)
    {
        auto from = model->index(rowIndex, FlexplorerTableModel::COL_ID);
        auto to = model->index(rowIndex, FlexplorerTableModel::COL_ATTRIBUTES);
        QItemSelection singleRowSelection(from, to);
        allSelections.merge(singleRowSelection, QItemSelectionModel::Select);
    }

    selectionModel()->select(allSelections, QItemSelectionModel::Select);

    setSelectionMode(selection);
}

void FlexplorerMdiChild::ResizeColumn(int column, const QString &text) const
{
    QStyleOptionViewItem style;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    initViewItemOption(&style);
    auto fontMetrics = QFontMetrics(style.font);
#else
    style = viewOptions();
    auto fontMetrics = QFontMetrics(style.font);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int size = fontMetrics.horizontalAdvance(text);
#else
    int size = fontMetrics.width(text);
#endif
    horizontalHeader()->resizeSection(column, size);
}

void FlexplorerMdiChild::ResizeColumns() const
{
    int column = 0;

    for (const auto &maxString : model->GetColumnMaxStrings())
    {
        ResizeColumn(column++, maxString);
    }
}

