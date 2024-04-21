/*
    fpedit.h


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

#ifndef FPEDIT_INCLUDED
#define FPEDIT_INCLUDED

#include "efiletim.h"
#include "warnoff.h"
#include <QStringList>
#include <QStyledItemDelegate>
#include <QRegularExpressionValidator>
#include "warnon.h"

class QWidget;
class QModelIndex;
class QAbstractItemModel;
class QStyledOptionViewItem;


class FlexRegularExpressionValidator : public QRegularExpressionValidator
{
public:
    FlexRegularExpressionValidator() = delete;
    explicit FlexRegularExpressionValidator(const QRegularExpression &regex,
                                   QObject *parent = Q_NULLPTR,
                                   const QVector<QString> &filenames = { });
    QValidator::State validate(QString &input, int &pos) const override;

private:
    QVector<QString> filenames;
};

// Delegate to edit date and time.
class FlexDateDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    const FileTimeAccess &fileTimeAccess;
    QWidget *parentWidget;

public:
    FlexDateDelegate(const FileTimeAccess &fileTimeAccess,
                     QWidget *parentWidget);

    QString displayText(const QVariant &value, const QLocale &locale)
        const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

// Delegate to edit date.
class FlexDateTimeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    const FileTimeAccess &fileTimeAccess;
    QWidget *parentWidget;

public:
    FlexDateTimeDelegate(const FileTimeAccess &fileTimeAccess,
                         QWidget *parentWidget);

    QString displayText(const QVariant &value, const QLocale &locale)
        const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

class FlexFilenameDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    QWidget *parentWidget;

public:
    explicit FlexFilenameDelegate(QWidget *parentWidget);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

class FlexAttributesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    QString supportedAttributes;
    QWidget *parentWidget;

public:
    explicit FlexAttributesDelegate(const QString &p_supportedAttributes,
                                    QWidget *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    static QStringList GetAllCombinations(const QString &attributes);

private:
    static void GetCombinations(const QString &chars, int length, int start,
                                QString &current, QStringList &result);
};

#endif
