/*
    fpmodel.cpp


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
#include "fdirent.h"
#include "flexerr.h"
#include "ffilecnt.h"
#include "dircont.h"
#include "fcopyman.h"
#include "ifilecnt.h"
#include "ffilebuf.h"
#include "filecont.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "warnoff.h"
#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QDate>
#include <QVector>
#include <QSet>
#include <QModelIndex>
#include <QModelIndexList>
#include <QTextStream>
#include <QIODevice>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include "warnon.h"
#include "fpmodel.h"


const QString FlexplorerTableModel::headerNameFileSize(tr("Filesize"));
const QString FlexplorerTableModel::headerNameDataSize(tr("Datasize"));

std::array<QString, FlexplorerTableModel::COLUMNS>
    FlexplorerTableModel::headerNames =
{
    tr("Id"), tr("Filename"), tr("Filetype"), tr("Random"), tr("Filesize"),
    tr("Date"), tr("Attributes"),
};

QVector<QPair<char, Byte> > FlexplorerTableModel::attributeCharToFlag
{
    { 'W', WRITE_PROTECT },
    { 'D', DELETE_PROTECT },
    { 'R', READ_PROTECT },
    { 'C', CATALOG_PROTECT },
};

const QVector<FlexplorerTableModel::sFileTypes>
    FlexplorerTableModel::fileTypes
{
    { "BIN", "Binary file" },
    { "TXT", "Text file" },
    { "CMD", "Executable file" },
    { "BAS", "Basic file" },
    { "SYS", "System file" },
    { "BAK", "Backup file" },
    { "BAC", "Backup file" },
    { "DAT", "Data file" },
};

FlexplorerTableModel::FlexplorerTableModel(const char *p_path,
                                           struct sFPOptions &p_options,
                                           QObject *parent)
    : QAbstractTableModel(parent)
    , options(p_options)
{
    OpenContainer(p_path, options.ft_access);
    Initialize();
}

FlexplorerTableModel::~FlexplorerTableModel()
{
}

QString FlexplorerTableModel::GetFileType(const FlexDirEntry &dirEntry)
{
    QString fileExtension(dirEntry.GetFileExt().c_str());

    for (const auto &fileType : fileTypes)
    {
        if (fileType.fileExt == fileExtension)
        {
            return tr(fileType.fileType.toUtf8().data());
        }
    }

    return tr("%1 file").arg(fileExtension);
}

void FlexplorerTableModel::CreateAttributesBitmasks(const QString &attributes,
                                                    Byte &setMask,
                                                    Byte &clearMask)
{
    setMask = 0;
    clearMask = 0;

    for (const auto &item : attributeCharToFlag)
    {
        if (attributes.contains(item.first))
        {
            setMask |= item.second;
        }
        else
        {
            clearMask |= item.second;
        }
    }
}

QModelIndex FlexplorerTableModel::AddRow(const FlexDirEntry &dirEntry, int role)
{
    static const int ssm4 = SECTOR_SIZE - 4;
    int column = 0;
    auto row = rowCount();

    insertRows(row, 1);
    setData(index(row, column++), row, role);
    QString fileName(dirEntry.GetTotalFileName().c_str());
    setData(index(row, column++), fileName, role);
    auto fileType = GetFileType(dirEntry);
    setData(index(row, column++), fileType, role);
    QString random(dirEntry.IsRandom() ? "Yes" : "");
    setData(index(row, column++), random, role);
    auto filesize = dirEntry.GetFileSize();
    if (options.fileSizeType == FileSizeType::DataSize)
    {
        filesize = filesize / SECTOR_SIZE * ssm4;
    }
    setData(index(row, column++), filesize, role);
    const auto &date = dirEntry.GetDate();
    QDate qdate(date.GetYear(), date.GetMonth(), date.GetDay());
    const auto &time = dirEntry.GetTime();
    QTime qtime(time.GetHour(), time.GetMinute());
    QDateTime qDateTime(qdate, qtime);
    setData(index(row, column++), qDateTime, role);
    QString attributes(dirEntry.GetAttributesString().c_str());
    setData(index(row, column), attributes, role);

    return index(row, 0);
}

void FlexplorerTableModel::Initialize()
{
    FileContainerIterator iter;

    for (iter = container->begin(); iter != container->end(); ++iter)
    {
        AddRow(*iter);
    }
}

QString FlexplorerTableModel::GetPath() const
{
    return path;
}

QString FlexplorerTableModel::GetUserFriendlyPath() const
{
    return getFileName(path.toUtf8().data()).c_str();
}

bool FlexplorerTableModel::IsWriteProtected() const
{
    return container->IsWriteProtected();
}

std::string FlexplorerTableModel::GetSupportedAttributes() const
{
    return container->GetSupportedAttributes();
}

void FlexplorerTableModel::UpdateFileSizeHeaderName()
{
    auto headerText = (options.fileSizeType == FileSizeType::FileSize) ?
                       headerNameFileSize : headerNameDataSize;
    auto variant = QVariant(headerText);

    setHeaderData(FlexplorerTableModel::COL_SIZE, Qt::Horizontal, variant,
                  Qt::DisplayRole);
}

void FlexplorerTableModel::UpdateFileSizeColumn()
{
    static const int column = COL_SIZE;
    static const int ssm4 = SECTOR_SIZE - 4;

    for (int row = 0; row < rows.size(); ++row)
    {
        auto value = rows[row].at(column).toInt();
        if (options.fileSizeType == FileSizeType::FileSize)
        {
            value = value / ssm4 * SECTOR_SIZE;
        }
        else if (options.fileSizeType == FileSizeType::DataSize)
        {
            value = value / SECTOR_SIZE * ssm4;
        }
        QVariant variant(value);
        rows[row].at(column) = variant;
    }
    QVector<int> roles { Qt::DisplayRole };

    emit dataChanged(index(0, column), index(rowCount(), column), roles);
}

int FlexplorerTableModel::GetContainerType() const                                
{
    return container->GetContainerType();
}

const FlexContainerInfo FlexplorerTableModel::GetContainerInfo() const                                
{
    FlexContainerInfo info;

    container->GetInfo(info);

    return info;
}

QVector<QString> FlexplorerTableModel::GetFilenames(
        const QModelIndexList &indexList) const
{
    QVector<QString> filenames;

    filenames.reserve(indexList.size());
    for (auto &index : indexList)
    {
        if (index.isValid())
        {
            filenames.append(rows[index.row()][COL_FILENAME].toString());
        }
    }

    return filenames;
}

QVector<QString> FlexplorerTableModel::GetFilenames() const
{
    QVector<QString> filenames;

    for (int row = 0; row < rowCount(); ++row)
    {
        filenames.append(rows[row][COL_FILENAME].toString());
    }

    return filenames;
}

QString FlexplorerTableModel::GetFilename(const QModelIndex &index) const
{
    if (index.row() >= 0 && index.row() < rowCount())
    {
        return rows[index.row()][COL_FILENAME].toString();
    }

    return "";
}

QVector<Byte> FlexplorerTableModel::GetAttributes(
        const QModelIndexList &indexList) const
{
    auto vFilenames = GetFilenames(indexList);
    QSet<QString> filenames;

    filenames.reserve(vFilenames.size());
    for (auto srcIter = vFilenames.begin(); srcIter != vFilenames.end(); )
    {
        filenames.insert(*(srcIter++));
    }

    FileContainerIterator iter;
    QVector<Byte> attributes;
    attributes.reserve(filenames.size());
    for (iter = container->begin(); iter != container->end(); ++iter)
    {
        const auto &dirEntry = *iter;

        if (filenames.contains(QString(dirEntry.GetTotalFileName().c_str())))
        {
            attributes.append(dirEntry.GetAttributes());
        }
    }

    return attributes;
}

bool FlexplorerTableModel::GetAttributesString(const QModelIndex &index,
                                              QString &attributesString) const
{
    if (index.isValid())
    {
        auto filename = GetFilename(index);
        FileContainerIterator iter(filename.toUtf8().data());

        iter = container->begin();
        if (iter != container->end())
        {
            attributesString = (*iter).GetAttributesString().c_str();
            return true;
        }
    }

    return false;
}

bool FlexplorerTableModel::GetAttributes(const QModelIndex &index,
                                         Byte &attributes) const
{
    if (index.isValid())
    {
        const auto filename = GetFilename(index);
        FileContainerIterator iter(filename.toUtf8().data());

        iter = container->begin();
        if (iter != container->end())
        {
            attributes = (*iter).GetAttributes();
            return true;
        }
    }

    return false;
}

QVector<int> FlexplorerTableModel::FindFiles(const QString &pattern) const
{
    QVector<int> rowIndices;
    int rowIndex = 0;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QString regexPattern(pattern);

    // Convert wildcard pattern into a regex pattern
    regexPattern.replace(QString(".").data(), 1, QString("\\.").data(), 2);
    regexPattern.replace(QString("*").data(), 1, QString(".*").data(), 2);
    regexPattern.replace('?', '.');
    QRegularExpression regex(regexPattern,
        QRegularExpression::CaseInsensitiveOption);
    auto doesMatch = [&](const QString& subject) -> bool {
        return regex.match(subject).hasMatch();
    };
#else
    QRegExp regex(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
    auto doesMatch = [&](const QString& subject) -> bool {
        return regex.exactMatch(subject);
    };
#endif

    rowIndices.reserve(rows.size() / 2);
    for (const auto &row : rows)
    {
        if (doesMatch(row[COL_FILENAME].toString()))
        {
            rowIndices.push_back(rowIndex);
        }
        ++rowIndex;
    }

    return rowIndices;
}

QModelIndex FlexplorerTableModel::AddFile(const FlexFileBuffer &buffer)
{
    FlexDirEntry dirEntry;
    auto filename = buffer.GetFilename();

    container->WriteFromBuffer(buffer);
    if (container->FindFile(filename, dirEntry))
    {
        return AddRow(dirEntry, Qt::DisplayRole);
    }

    return QModelIndex();
}

void FlexplorerTableModel::DeleteFile(const QModelIndex &index)
{
    if (index.isValid())
    {
        auto filename = GetFilename(index);
        container->DeleteFile(filename.toUtf8().data());
        removeRows(index.row(), 1);
    }
}

void FlexplorerTableModel::RenameFile(const QModelIndex &index,
                                      const QString &newFilename)
{
    if (index.isValid())
    {
        FlexDirEntry dirEntry;
        auto oldFilename = GetFilename(index);
        dirEntry.SetTotalFileName(oldFilename.toUtf8().data());
        auto oldFileType = GetFileType(dirEntry);

        try
        {
            container->RenameFile(oldFilename.toUtf8().data(),
                                  newFilename.toUtf8().data());
        }
        catch (FlexException &ex)
        {
            // Rename failed: Restore the old file name.
            setData(index, oldFilename, Qt::DisplayRole);
            throw ex;
        }

        // Rename in container was successful.
        // 1. Update file name column.
        setData(index, newFilename);
        // 2. Update the file type column if changed.
        dirEntry.SetTotalFileName(newFilename.toUtf8().data());
        auto newFileType = GetFileType(dirEntry);
        if (newFileType != oldFileType)
        {
            auto fileTypeIndex =
                QAbstractTableModel::index(index.row(), COL_FILETYPE);
            setData(fileTypeIndex, newFileType);
        }
    }
}

FlexFileBuffer FlexplorerTableModel::CopyFile(const QModelIndex &index) const
{
    if (index.isValid())
    {
        auto filename = GetFilename(index);
        return container->ReadToBuffer(filename.toUtf8().data());
    }

    return FlexFileBuffer();
}

void FlexplorerTableModel::SetAttributesString(const QModelIndex &index,
                                               const QString &newAttribString)
{
    if (index.isValid())
    {
        auto oldAttribString = rows[index.row()][COL_ATTRIBUTES].toString();
        auto filename = GetFilename(index);

        if (oldAttribString != newAttribString)
        {
            Byte setMask, clearMask;

            CreateAttributesBitmasks(newAttribString, setMask, clearMask);
            auto attribIndex =
                QAbstractTableModel::index(index.row(), COL_ATTRIBUTES);

            try
            {
                container->SetAttributes(filename.toUtf8().data(),
                                         setMask, clearMask);
            }
            catch (FlexException &ex)
            {
                // Set attributes failed: Restore the old file attributes.
                setData(attribIndex, oldAttribString);
                throw ex;
            }

            setData(attribIndex, newAttribString);
        }
    }
}

int FlexplorerTableModel::rowCount(const QModelIndex &) const
{
    // This is an override method with a defined return type.
    return static_cast<int>(rows.size());
}

int FlexplorerTableModel::columnCount(const QModelIndex &) const
{
    return COLUMNS;
}

QVariant FlexplorerTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() &&
        (role == Qt::DisplayRole || role == Qt::EditRole))
    {
        if (index.row() < rows.size() && index.column() < COLUMNS)
        {
            return rows.at(index.row()).at(index.column());
        }
    }

    return QVariant();
}

bool FlexplorerTableModel::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    if (index.isValid() &&
        (role == Qt::DisplayRole || role == Qt::EditRole))
    {
        if (index.row() < rows.size() && index.column() < COLUMNS)
        {
            QVector<int> roles { role };
            if (rows.at(index.row()).at(index.column()) != value)
            {
                rows[index.row()].at(index.column()) = value;
                emit dataChanged(index, index, roles);
                return true;
            }
        }
    }

    return false;
}

QVariant FlexplorerTableModel::headerData(
        int section,
        Qt::Orientation orientation,
        int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal &&
            section >= 0 &&
            section < static_cast<int>(headerNames.size()))
        {
            return headerNames.at(section);
        }
        else if (orientation == Qt::Vertical)
        {
            return QString("%1").arg(section + 1);
        }
    }

    return QVariant();
}

bool FlexplorerTableModel::setHeaderData(
        int section,
        Qt::Orientation orientation,
        const QVariant &value,
        int role)
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal &&
        section >= 0 &&
        section < static_cast<int>(headerNames.size()) &&
        role == Qt::DisplayRole)
    {
        bool isChanged = (headerNames[section] != value.toString());

        if (isChanged)
        {
            headerNames[section] = value.toString();
            emit headerDataChanged(orientation, section, section);
        }

        return true;
    }

    return false;
}

Qt::ItemFlags FlexplorerTableModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
    {
        static QVector<int> editableRows{ 1, 6 };
        int col = index.column();
        auto isEditable = !container->IsWriteProtected() &&
                          editableRows.contains(col);

        if (isEditable)
        {
            return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        }
        else
        {
            return QAbstractItemModel::flags(index);
        }
    }
    else
    {
        return Qt::ItemIsEnabled;
    }
}

bool FlexplorerTableModel::insertRows(
        int row,
        int count,
        const QModelIndex &)
{
    if (count >= 0 && row <= rows.size() + 1)
    {
        if (count > 0)
        {
            RowType newRow;
            beginInsertRows(QModelIndex(), row, row + count - 1);
            auto iter = rows.begin() + row;
            rows.insert(iter, newRow);
            endInsertRows();
        }

        return true;
    }

    return false;
}

bool FlexplorerTableModel::removeRows(
        int row,
        int count,
        const QModelIndex &)
{
    if (count >= 0 && row + count <= rows.size())
    {
        if (count > 0)
        {
            beginRemoveRows(QModelIndex(), row, row + count - 1);
            auto iter = rows.begin() + row;
            rows.erase(iter, iter + count);
            endRemoveRows();
        }

        return true;
    }

    return false;
}

FlexplorerTableModel::IdsType FlexplorerTableModel::GetIds() const
{
    IdsType ids;

    ids.reserve(rowCount());
    for (const auto &row : rows)
    {
        ids.push_back(row[COL_ID].toString());
    }

    return ids;
}

void FlexplorerTableModel::CalculateAndChangePersistentIndexList(
        const IdsType &oldIds)
{
    auto newIds = GetIds();
    QModelIndexList fromList;
    QModelIndexList toList;

    for (int oldRow = 0; oldRow < oldIds.size(); ++oldRow)
    {
        if (oldIds[oldRow] != newIds[oldRow])
        {
            auto pos = std::find(newIds.begin(), newIds.end(), oldIds[oldRow]);
            int newRow = static_cast<int>(pos - newIds.begin());

            for (int col = 0; col < COLUMNS; ++col)
            {
                fromList.push_back(index(oldRow, col));
                toList.push_back(index(newRow, col));
            }
        }
    }

    changePersistentIndexList(fromList, toList);
}

void FlexplorerTableModel::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= rows.size())
    {
        return;
    }

    auto oldIds = GetIds();
    std::function<bool(const RowType &lhs, const RowType &rhs)> compare;

    switch (order)
    {
        case Qt::AscendingOrder:
            compare = [&column](const RowType &lhs, const RowType &rhs){
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                if (lhs[column].typeId() == QMetaType::QString)
                {
                    return lhs[column].toString() < rhs[column].toString();
                }
                else if (lhs[column].typeId() == QMetaType::QDateTime)
                {
                    return lhs[column].toDateTime() < rhs[column].toDateTime();
                }
                else if (lhs[column].typeId() == QMetaType::Int)
                {
                    return lhs[column].toInt() < rhs[column].toInt();
                }
                else
#else
                if (lhs[column].type() == QVariant::String)
                {
                    return lhs[column].toString() < rhs[column].toString();
                }
                else if (lhs[column].type() == QVariant::DateTime)
                {
                    return lhs[column].toDateTime() < rhs[column].toDateTime();
                }
                else if (lhs[column].type() == QVariant::Int)
                {
                    return lhs[column].toInt() < rhs[column].toInt();
                }
                else
#endif
                {
                    auto msg = std::string(
                        "FlexplorerTableModel::sort(): Unexpected type '");
                    msg += std::string(lhs[column].typeName()) + "'";

                    throw std::logic_error(msg);
                }
            };
            break;

        case Qt::DescendingOrder:
            compare = [&column](const RowType &lhs, const RowType &rhs){
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                if (lhs[column].typeId() == QMetaType::QString)
                {
                    return lhs[column].toString() > rhs[column].toString();
                }
                else if (lhs[column].typeId() == QMetaType::QDateTime)
                {
                    return lhs[column].toDateTime() > rhs[column].toDateTime();
                }
                else if (lhs[column].typeId() == QMetaType::Int)
                {
                    return lhs[column].toInt() > rhs[column].toInt();
                }
                else
#else
                if (lhs[column].type() == QVariant::String)
                {
                    return lhs[column].toString() > rhs[column].toString();
                }
                else if (lhs[column].type() == QVariant::DateTime)
                {
                    return lhs[column].toDateTime() > rhs[column].toDateTime();
                }
                else if (lhs[column].type() == QVariant::Int)
                {
                    return lhs[column].toInt() > rhs[column].toInt();
                }
                else
#endif
                {
                    auto msg = std::string(
                        "FlexplorerTableModel::sort(): Unexpected type '");
                    msg += std::string(lhs[column].typeName()) + "'";

                    throw std::logic_error(msg);
                }
            };
            break;
    }

    emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(),
                                QAbstractItemModel::VerticalSortHint);

    std::stable_sort(rows.begin(), rows.end(), compare);
    CalculateAndChangePersistentIndexList(oldIds);

    emit layoutChanged();
}

QString FlexplorerTableModel::AsHtml(const QModelIndexList &indexList) const
{
    int column;
    QString htmlString;
    QTextStream stream(&htmlString, QIODevice::ReadOnly);

    stream <<
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n" <<
        "<html>\n" <<
        "<head>\n" <<
        "<meta http-equiv=\"content-type\""
            "content=\"text/html; charset=utf-8\">\n" <<
        "<meta name=\"generator\" "
        "content=\"Flexplorer " VERSION " (" OSTYPE ")\">\n" <<
        "<title></title>\n" <<
        "<style type=\"text/css\">\n" <<
        "body,table,tr,th,td { font-family:\"Courier\"; font-size:x-small }\n"<<
        "table,th,td { border: 1px solid black; border-spacing: 1px }\n" <<
        "</style>\n" <<
        "</head>\n" <<
        "<body>\n" <<
        "<table>\n";

    stream << "<tr>";
    for (column = 1; column < COLUMNS; ++column)
    {
        stream <<
            "<th>" <<
            headerData(column, Qt::Horizontal, Qt::DisplayRole).toString() <<
            "</th>";
    }
    stream << "</tr>\n";

    for (const auto &rowIndex : indexList)
    {
        stream << "<tr>";
        for (column = 1; column < COLUMNS; ++column)
        {
            auto modelIndex = index(rowIndex.row(), column);
            stream <<
                "<td>" <<
                data(modelIndex, Qt::DisplayRole).toString() <<
                "</td>";
        }
        stream << "</tr>\n";
    }

    stream <<
        "</table>\n" <<
        "</body>\n" <<
        "</html>\n";

    return htmlString;
}

QString FlexplorerTableModel::AsText(const QModelIndexList &indexList,
                                     const QString &mimeType) const
{
    if (mimeType == "text/html")
    {
        return AsHtml(indexList);
    }

    int column;
    QString textString;
    QTextStream stream(&textString, QIODevice::ReadOnly);
    std::function<void(int)> fieldWidthFct = [](int){ };
    std::function<void(int)> separatorFct = [](int){ };

    if (mimeType == "text/plain")
    {
        fieldWidthFct = [&](int col){
            static QVector<int> widths { 13, 17, 7, 7, 12, 11 };
            stream.setFieldWidth(widths[col - 1]);
        };
    }
    else if (mimeType == "text/csv")
    {
        separatorFct = [&](int col){
            if (col < COLUMNS - 1)
            {
                stream << ",";
            }
        };
    }
    else
    {
        return "";
    }

    for (column = 1; column < COLUMNS; ++column)
    {
        fieldWidthFct(column);
        stream << 
            headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
        separatorFct(column);
    }
    stream.setFieldWidth(0);
    stream << "\n";

    for (const auto &rowIndex : indexList)
    {
        for (column = 1; column < COLUMNS; ++column)
        {
            fieldWidthFct(column);
            auto modelIndex = index(rowIndex.row(), column);
            stream << data(modelIndex, Qt::DisplayRole).toString();
            separatorFct(column);
        }
        stream.setFieldWidth(0);
        stream << "\n";
    }

    return textString;
}

void FlexplorerTableModel::OpenContainer(const char *p_path,
                                         const FileTimeAccess &fileTimeAccess)
{
    struct stat sbuf;
    
    // path can either be a directory or a file container.
    if (p_path == nullptr || *p_path == '\0' || stat(p_path, &sbuf))
    {   
        throw FlexException(FERR_INVALID_NULL_POINTER, "p_path");
    }
    else if (stat(p_path, &sbuf))
    {   
        throw FlexException(FERR_UNABLE_TO_OPEN, p_path);
    }
    
    path = p_path;
    
    if (S_ISDIR(sbuf.st_mode))
    {
        std::string directory(p_path);
        
        if (endsWithPathSeparator(directory))
        {   
            directory = directory.substr(0, directory.size()-1);
        }
        
        container.reset(
            new DirectoryContainer(directory.c_str(), fileTimeAccess));
    }
    else
    {   
        try 
        {   
            // 1st try opening read-write.
            container.reset(new FlexFileContainer(p_path, "rb+",
                            fileTimeAccess));
        }
        catch (FlexException &)
        {   
            // 2nd try opening read-only.
            container.reset(new FlexFileContainer(p_path, "rb",
                            fileTimeAccess));
        }
    }
    auto container_s =
        dynamic_cast<FileContainerIfSector *>(container.get());
    if (container_s != nullptr && !container_s->IsFlexFormat())
    {
        container.reset();
        throw FlexException(FERR_CONTAINER_UNFORMATTED, p_path);
    }
}

