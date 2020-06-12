/*
    fpedit.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2020  W. Schwotzer

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

#include <QStyledItemDelegate>
#include <QRegularExpressionValidator>

class QWidget;
class QStringList;
class QModelIndex;
class QAbstractItemModel;
class QStyledOptionViewItem;


class FlexRegularExpressionValidator : public QRegularExpressionValidator
{
public:
    FlexRegularExpressionValidator() = delete;
    FlexRegularExpressionValidator(const QRegularExpression &regex,
                                   QObject *parent = Q_NULLPTR,
                                   const QVector<QString> &filenames = { });
    QValidator::State validate(QString &input, int &pos) const override;

private:
    QVector<QString> filenames;
};

class FlexDateDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    QWidget *parentWidget;

public:
    explicit FlexDateDelegate(QWidget *parentWidget);

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

    static QStringList GetAllCombinations(const QString &supportedAttributes);

private:
    static void GetCombinations(const QString &chars, int length, int start,
                                QString &current, QStringList &result);
};

#endif
