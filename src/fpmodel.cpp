/*
    fpmodel.cpp


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
#include "fdirent.h"
#include "flexerr.h"
#include "fattrib.h"
#include "ffilecnt.h"
#include "dircont.h"
#include "fcopyman.h"
#include "ifilecnt.h"
#include "ffilebuf.h"
#include "filecont.h"
#include "qtfree.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "warnoff.h"
#include <QtGlobal>
#include <QObject>
#include <QLocale>
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
#include <unordered_map>
#include <utility>
#include <memory>


HeaderNames_t &FlexplorerTableModel::GetHeaderNames()
{
    static HeaderNames_t headerNames =
    {
        tr("Id"), tr("Filename"), tr("Filetype"), tr("Random"), tr("Filesize"),
        tr("Date"), tr("Attributes"),
    };

    return headerNames;
}

const FlexplorerTableModel::FileTypes_t &FlexplorerTableModel::GetFileTypes()
{
    static const FileTypes_t fileTypes
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

    return fileTypes;
}

FlexplorerTableModel::FlexplorerTableModel(const char *p_path,
                                           struct sFPOptions &p_options,
                                           QObject *parent)
    : QAbstractTableModel(parent)
    , options(p_options)
{
    OpenFlexDisk(p_path, options.ft_access);
    // For each column set a string with aproximate maximum size.
    // Date column width depends on options. See GetColumnMaxStrings()
    // for details.
    maxStrings = QStringList({
        "*88888*", "*MMMMMMMM.MMM*", "*Executable file*", "*Random*",
        "*Filesize*", "", "*Attributes*"
    });

}

QString FlexplorerTableModel::GetFileType(const FlexDirEntry &dirEntry)
{
    QString fileExtension(dirEntry.GetFileExt().c_str());

    const auto &fileTypes = GetFileTypes();
    for (const auto &fileType : fileTypes)
    {
        if (fileType.fileExt == fileExtension)
        {
            return fileType.fileType;
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

    for (const auto &item : GetAttributeCharToFlag())
    {
        if (attributes.contains(item.first))
        {
            setMask |= static_cast<Byte>(item.second);
        }
        else
        {
            clearMask |= static_cast<Byte>(item.second);
        }
    }
}

QModelIndex FlexplorerTableModel::SetRow(const FlexDirEntry &dirEntry, int row,
                                         int role)
{
    static const int ssm4 = SECTOR_SIZE - 4;
    int column = 0;

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
    FlexDiskIterator iter;
    int columnIndex = 0;

    for (iter = container->begin(); iter != container->end(); ++iter)
    {
        ++columnIndex;
    }

    insertRows(0, columnIndex);
    columnIndex = 0;
    for (iter = container->begin(); iter != container->end(); ++iter)
    {
        SetRow(*iter, columnIndex, Qt::DisplayRole);
        ++columnIndex;
    }
}

QString FlexplorerTableModel::GetPath() const
{
    return path;
}

QString FlexplorerTableModel::GetUserFriendlyPath() const
{
    return flx::getFileName(path.toStdString()).c_str();
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
    const auto &headerText = GetHeaderNameForFileSize(options.fileSizeType);
    auto variant = QVariant(headerText);

    setHeaderData(FlexplorerTableModel::COL_SIZE, Qt::Horizontal, variant,
                  Qt::DisplayRole);
}

void FlexplorerTableModel::UpdateFileSizeColumn()
{
    static const int column = COL_SIZE;
    static const int ssm4 = SECTOR_SIZE - 4;

    for (auto &row : rows)
    {
        auto value = row.at(column).toUInt();
        if (options.fileSizeType == FileSizeType::FileSize)
        {
            value = value / ssm4 * SECTOR_SIZE;
        }
        else if (options.fileSizeType == FileSizeType::DataSize)
        {
            value = value / SECTOR_SIZE * ssm4;
        }
        QVariant variant(value);
        row.at(column) = variant;
    }
    QVector<int> roles { Qt::DisplayRole };

    emit dataChanged(index(0, column), index(rowCount(), column), roles);
}

unsigned FlexplorerTableModel::GetFlexDiskType() const
{
    return container->GetFlexDiskType();
}

FlexDiskAttributes FlexplorerTableModel::GetFlexDiskAttributes() const
{
    FlexDiskAttributes diskAttributes;

    container->GetDiskAttributes(diskAttributes);

    return diskAttributes;
}

QStringList FlexplorerTableModel::GetFilenames(
        const QModelIndexList &indexList) const
{
    QStringList filenames;

    filenames.reserve(indexList.size());
    for (const auto &index : indexList)
    {
        if (index.isValid())
        {
            filenames.append(rows[index.row()][COL_FILENAME].toString());
        }
    }

    return filenames;
}

QStringList FlexplorerTableModel::GetFilenames() const
{
    QStringList filenames;

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
    // clang-tidy: auto *srcIter is not compatible with Qt6.
    // NOLINTNEXTLINE(readability-qualified-auto)
    for (auto srcIter = vFilenames.begin(); srcIter != vFilenames.end(); )
    {
        filenames.insert(*(srcIter++));
    }

    FlexDiskIterator iter;
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
        FlexDiskIterator iter(filename.toStdString());

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
        FlexDiskIterator iter(filename.toStdString());

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
    container->WriteFromBuffer(buffer);

    auto dirEntry = buffer.GetDirEntry();
    if (!dirEntry.IsEmpty())
    {
        insertRows(rowCount(), 1);
        return SetRow(dirEntry, rowCount() - 1, Qt::DisplayRole);
    }

    return {};
}

void FlexplorerTableModel::DeleteFile(const QModelIndex &index)
{
    if (index.isValid())
    {
        auto filename = GetFilename(index);
        container->DeleteFile(filename.toStdString());
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
        dirEntry.SetTotalFileName(oldFilename.toStdString());
        auto oldFileType = GetFileType(dirEntry);

        try
        {
            container->RenameFile(oldFilename.toStdString(),
                                  newFilename.toStdString());
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
        dirEntry.SetTotalFileName(newFilename.toStdString());
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
        return container->ReadToBuffer(filename.toStdString());
    }

    return {};
}

void FlexplorerTableModel::SetAttributesString(const QModelIndex &index,
                                               const QString &attributes)
{
    if (index.isValid())
    {
        auto oldAttributes = rows[index.row()][COL_ATTRIBUTES].toString();
        auto filename = GetFilename(index);

        if (oldAttributes != attributes)
        {
            Byte setMask;
            Byte clearMask;

            CreateAttributesBitmasks(attributes, setMask, clearMask);
            auto attribIndex =
                QAbstractTableModel::index(index.row(), COL_ATTRIBUTES);

            try
            {
                container->SetAttributes(filename.toStdString(),
                                         setMask, clearMask);
            }
            catch (FlexException &ex)
            {
                // Set attributes failed: Restore the old file attributes.
                setData(attribIndex, oldAttributes);
                throw ex;
            }

            setData(attribIndex, attributes);
        }
    }
}

int FlexplorerTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    // This is an override method with a defined return type.
    return cast_from_qsizetype(rows.size());
}

int FlexplorerTableModel::columnCount(const QModelIndex & /*parent*/) const
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

    return {};
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
            section < static_cast<int>(GetHeaderNames().size()))
        {
            return GetHeaderNames().at(section);
        }

        if (orientation == Qt::Vertical)
        {
            return QString("%1").arg(section + 1);
        }
    }

    return {};
}

bool FlexplorerTableModel::setHeaderData(
        int section,
        Qt::Orientation orientation,
        const QVariant &value,
        int role)
{
    auto &headerNames = GetHeaderNames();
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal &&
        section >= 0 &&
        section < static_cast<int>(headerNames.size()))
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
            return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        }

        return QAbstractTableModel::flags(index);
    }

    return Qt::ItemIsEnabled;
}

bool FlexplorerTableModel::insertRows(
        int row,
        int count,
        const QModelIndex & /*parent*/)
{
    if (count >= 0 && row == rows.size())
    {
        if (count > 0)
        {
            RowType newRow;
            auto max = row + count;
            beginInsertRows(QModelIndex(), row, max - 1);
            for (auto i = row; i < max; ++i)
            {
                rows.append(newRow);
            }
            endInsertRows();
        }

        return true;
    }

    return false;
}

bool FlexplorerTableModel::removeRows(
        int row,
        int count,
        const QModelIndex & /*parent*/)
{
    if (count >= 0 && row + count <= rows.size())
    {
        if (count > 0)
        {
            beginRemoveRows(QModelIndex(), row, row + count - 1);
            // clang-tidy: auto *const iter is not compatible with Qt6.
            // NOLINTNEXTLINE(readability-qualified-auto)
            const auto iter = rows.begin() + row;
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
        ids.push_back(row[COL_ID].toInt());
    }

    return ids;
}

void FlexplorerTableModel::CalculateAndChangePersistentIndexList(
        const IdsType &oldIds)
{
    auto newIds = GetIds();
    QModelIndexList fromList;
    QModelIndexList toList;
    std::unordered_map<int, int> idToNewRow;
    int newRow = 0;

    idToNewRow.reserve(oldIds.size());
    for (int newId : newIds)
    {
        idToNewRow.emplace(std::pair<int, int>(newId, newRow));
        ++newRow;
    }

    fromList.reserve(oldIds.size() * COLUMNS);
    toList.reserve(oldIds.size() * COLUMNS);
    for (int oldRow = 0; oldRow < oldIds.size(); ++oldRow)
    {
        if (oldIds[oldRow] != newIds[oldRow])
        {
            newRow = idToNewRow[oldIds[oldRow]];

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
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif
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
                switch (lhs[column].typeId())
                {
                    case QMetaType::QString:
                        return lhs[column].toString() < rhs[column].toString();

                    case QMetaType::QDateTime:
                        return lhs[column].toDateTime() <
                            rhs[column].toDateTime();

                    case QMetaType::Int:
                        return lhs[column].toInt() < rhs[column].toInt();

                    case QMetaType::UInt:
                        return lhs[column].toUInt() < rhs[column].toUInt();

                    default:
                    {
                        auto msg = std::string(
                            "FlexplorerTableModel::sort(): Unexpected type '");
                        msg += std::string(lhs[column].typeName()) + "'";

                        throw std::logic_error(msg);
                    }
                }
#else
                switch (lhs[column].type())
                {
                    case QVariant::String:
                        return lhs[column].toString() < rhs[column].toString();

                    case QVariant::DateTime:
                        return lhs[column].toDateTime() <
                            rhs[column].toDateTime();

                    case QVariant::Int:
                        return lhs[column].toInt() < rhs[column].toInt();

                    case QVariant::UInt:
                        return lhs[column].toUInt() < rhs[column].toUInt();

                    default:
                    {
                        auto msg = std::string(
                            "FlexplorerTableModel::sort(): Unexpected type '");
                        msg += std::string(lhs[column].typeName()) + "'";

                        throw std::logic_error(msg);
                    }
                }
#endif
            };
            break;

        case Qt::DescendingOrder:
            compare = [&column](const RowType &lhs, const RowType &rhs){
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                switch (lhs[column].typeId())
                {
                    case QMetaType::QString:
                        return lhs[column].toString() > rhs[column].toString();

                    case QMetaType::QDateTime:
                        return lhs[column].toDateTime() >
                            rhs[column].toDateTime();

                    case QMetaType::Int:
                        return lhs[column].toInt() > rhs[column].toInt();

                    case QMetaType::UInt:
                        return lhs[column].toUInt() > rhs[column].toUInt();

                    default:
                    {
                        auto msg = std::string(
                            "FlexplorerTableModel::sort(): Unexpected type '");
                        msg += std::string(lhs[column].typeName()) + "'";

                        throw std::logic_error(msg);
                    }
                }
#else
                switch (lhs[column].type())
                {
                    case QVariant::String:
                        return lhs[column].toString() > rhs[column].toString();

                    case QVariant::DateTime:
                        return lhs[column].toDateTime() >
                            rhs[column].toDateTime();

                    case QVariant::Int:
                        return lhs[column].toInt() > rhs[column].toInt();

                    case QVariant::UInt:
                        return lhs[column].toUInt() > rhs[column].toUInt();

                    default:
                    {
                        auto msg = std::string(
                            "FlexplorerTableModel::sort(): Unexpected type '");
                        msg += std::string(lhs[column].typeName()) + "'";

                        throw std::logic_error(msg);
                    }
                }
#endif
            };
            break;
    }

    emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(),
                                QAbstractItemModel::VerticalSortHint);

    std::stable_sort(rows.begin(), rows.end(), compare);
    CalculateAndChangePersistentIndexList(oldIds);

    emit layoutChanged();
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}

QString FlexplorerTableModel::VariantToString(const QVariant &variant) const
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if (variant.typeId() == QMetaType::QDateTime)
#else
    if (variant.type() == QVariant::DateTime)
#endif
    {
        const auto locale = QLocale::system();

        if (options.ft_access == FileTimeAccess::NONE)
        {
            const auto d = variant.toDate();
            return locale.toString(d, QLocale::ShortFormat);
        }

        const auto dateTime = variant.toDateTime();
        return locale.toString(dateTime, QLocale::ShortFormat);
    }

    return variant.toString();
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
            const auto modelIndex = index(rowIndex.row(), column);
            const auto variant = data(modelIndex, Qt::DisplayRole);
            stream <<
                "<td>" <<
                VariantToString(variant) <<
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
        static QVector<int> widths { 13, 16, 7, 9, 12, 11 };
        widths[COL_DATE - 1] = (options.ft_access == FileTimeAccess::NONE) ?
            12 : 17;
        fieldWidthFct = [&](int col){
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
            stream << VariantToString(data(modelIndex, Qt::DisplayRole));
            separatorFct(column);
        }
        stream.setFieldWidth(0);
        stream << "\n";
    }

    return textString;
}

void FlexplorerTableModel::OpenFlexDisk(const char *p_path,
                                         const FileTimeAccess &fileTimeAccess)
{
    struct stat sbuf{};

    // path can either be a directory or a file container.
    if (p_path == nullptr)
    {
        throw FlexException(FERR_INVALID_NULL_POINTER, "p_path");
    }

    if (stat(p_path, &sbuf))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, p_path);
    }

    path = p_path;

    if (S_ISDIR(sbuf.st_mode))
    {
        std::string directory(p_path);

        if (flx::endsWithPathSeparator(directory))
        {
            directory = directory.substr(0, directory.size()-1);
        }

        container =
            std::make_unique<FlexDirectoryDiskByFile>(directory, fileTimeAccess);
    }
    else
    {
        auto mode = std::ios::in | std::ios::out | std::ios::binary;
        try
        {
            // 1st try opening read-write.
            container = std::make_unique<FlexDisk>(
                            p_path, mode, fileTimeAccess);
        }
        catch (FlexException &)
        {
            // 2nd try opening read-only.
            mode &= ~std::ios::out;
            container = std::make_unique<FlexDisk>(
                            p_path, mode, fileTimeAccess);
        }
    }
    auto *container_s =
        dynamic_cast<IFlexDiskBySector *>(container.get());
    if (container_s != nullptr && !container_s->IsFlexFormat())
    {
        container.reset();
        throw FlexException(FERR_CONTAINER_UNFORMATTED, p_path);
    }
}

QStringList FlexplorerTableModel::GetColumnMaxStrings()
{
    if (options.ft_access == FileTimeAccess::NONE)
    {
        maxStrings[COL_DATE] =  "*88/88/8888*";
    }
    else
    {
        maxStrings[COL_DATE] =  "88/88/88 88:88 MM";
    }

    return maxStrings;
}

const QString &FlexplorerTableModel::GetHeaderNameForFileSize(FileSizeType type)
{
    static const QString headerNameFileSize(tr("Filesize"));
    static const QString headerNameDataSize(tr("Datasize"));

    return (type == FileSizeType::FileSize) ?
        headerNameFileSize : headerNameDataSize;
}

