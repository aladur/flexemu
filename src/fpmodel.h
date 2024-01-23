/*
    fpmodel.h


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

#ifndef FPMODEL_INCLUDED
#define FPMODEL_INCLUDED

#include "misc1.h"
#include "fcinfo.h"
#include "efiletim.h"
#include "ffilebuf.h"
#include "sfpopts.h"
#include "warnoff.h"
#include <QPair>
#include <QString>
#include <QVector>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractTableModel>
#include "warnon.h"
#include <memory>
#include <vector>
#include <array>
#include <string>

class FlexDirEntry;
class FlexFileBuffer;
class FileContainerIf;
class FlexDirEntry;


class FlexplorerTableModel : public QAbstractTableModel
{
    struct sFileTypes
    {
        QString fileExt;
        QString fileType;
    };

public:
    FlexplorerTableModel() = delete;
    FlexplorerTableModel(const char *path, struct sFPOptions &options,
                         QObject *parent = Q_NULLPTR);
    virtual ~FlexplorerTableModel();

    // To correctly initialize the model Initialize() has to be called
    // right after object construction.
    // It has been separated to avoid virtual member function calls
    // in the object constructor.
    void Initialize();
    QString GetPath() const;
    QString GetUserFriendlyPath() const;
    bool IsWriteProtected() const;
    int GetContainerType() const;
    const FlexContainerInfo GetContainerInfo() const;

    QVector<QString> GetFilenames(const QModelIndexList &indexList) const;
    QVector<QString> GetFilenames() const;
    QVector<Byte> GetAttributes(const QModelIndexList &indexList) const;
    bool GetAttributes(const QModelIndex &index, Byte &attributes) const;
    bool GetAttributesString(const QModelIndex &index,
                             QString &attributes) const;
    void SetAttributesString(const QModelIndex &index,
                             const QString &attributes);
    static void CreateAttributesBitmasks(const QString &attributes,
                                         Byte &setMask, Byte &clearMask);
    QVector<int> FindFiles(const QString &pattern) const;
    QString AsText(const QModelIndexList &indexList,
                   const QString &mimeType) const;
    QString GetFilename(const QModelIndex &index) const;
    QModelIndex AddFile(const FlexFileBuffer &buffer);
    void DeleteFile(const QModelIndex &index);
    void RenameFile(const QModelIndex &index, const QString &newFilename);
    FlexFileBuffer CopyFile(const QModelIndex &index) const;
    std::string GetSupportedAttributes() const;

    // QAbstractTableModel overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int row, int count,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count,
                    const QModelIndex &parent = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order) override;

    static const int COL_ID{0};
    static const int COL_FILENAME{1};
    static const int COL_FILETYPE{2};
    static const int COL_RANDOM{3};
    static const int COL_SIZE{4};
    static const int COL_DATE{5};
    static const int COL_ATTRIBUTES{6};

public slots:
    void UpdateFileSizeHeaderName();
    void UpdateFileSizeColumn();

private:
    static const int COLUMNS{7};

    using RowType = std::array<QVariant, COLUMNS>;
    using IdsType = QVector<int>;

    void OpenContainer(const char *path, const FileTimeAccess &fileTimeAccess);
    QModelIndex AddRow(const FlexDirEntry &dirEntry, int role = Qt::EditRole);
    IdsType GetIds() const;
    void CalculateAndChangePersistentIndexList(const IdsType &oldIds);
    QString AsHtml(const QModelIndexList &indexList) const;
    static QString GetFileType(const FlexDirEntry &dirEntry);

    std::unique_ptr<FileContainerIf> container;
    QVector<RowType> rows;
    QString path;
    struct sFPOptions &options;

    static std::array<QString, COLUMNS> headerNames;
    static const QString headerNameFileSize;
    static const QString headerNameDataSize;
    static QVector<QPair<char, Byte> > attributeCharToFlag;
    static const QVector<sFileTypes> fileTypes;
};

#endif

