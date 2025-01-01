/*
    fpedit.cpp


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


#include "flexerr.h"
#include "qtfree.h"
#include "warnoff.h"
#include <QWidget>
#include <QDateEdit>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QCompleter>
#include <QAbstractItemModel>
#include <QStyleOptionViewItem>
#include <QMessageBox>
#include "warnon.h"
#include <algorithm>
#include <cassert>
#include "fpmodel.h"
#include "fpedit.h"


FlexRegularExpressionValidator::FlexRegularExpressionValidator(
        const QRegularExpression &regex, QObject *parent,
        QStringList p_filenames) :
    QRegularExpressionValidator(regex, parent),
        filenames(std::move(p_filenames))
{
}

QValidator::State FlexRegularExpressionValidator::validate(QString &input,
                                                           int &pos) const
{
    auto result = QRegularExpressionValidator::validate(input, pos);
    if (filenames.contains(input.toUpper()))
    {
        result = QValidator::Intermediate;
    }

    return result;
}

FlexDateDelegate::FlexDateDelegate(const FileTimeAccess &p_fileTimeAccess,
        QWidget *p_parentWidget) :
    fileTimeAccess(p_fileTimeAccess),
    parentWidget(p_parentWidget)
{
}

QString FlexDateDelegate::displayText(const QVariant &value,
        const QLocale &locale) const
{
    return locale.toString(value.toDate(), QLocale::ShortFormat);
}

QWidget *FlexDateDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem & /*option*/,
        const QModelIndex & /*index*/) const
{
    auto *editor = new QDateEdit(parent);

    editor->setFrame(false);
    // Limit the date range to valid FLEX dates.
    editor->setDateRange(QDate(1975, 1, 1), QDate(2074, 12, 31));
    editor->setSelectedSection(QDateTimeEdit::YearSection);
    editor->setCalendarPopup(true);

    return editor;
}

void FlexDateDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::EditRole).toDate();
    auto *dateEditor = dynamic_cast<QDateEdit *>(editor);
    assert(dateEditor != nullptr);
    dateEditor->setDate(value);
}

void FlexDateDelegate::setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    auto *dateEditor = dynamic_cast<QDateEdit *>(editor);
    assert(dateEditor != nullptr);
    auto value = dateEditor->date();
    model->setData(index, value, Qt::EditRole);
    //TODO: Set date in model
    QMessageBox::warning(parentWidget, tr("FLEXPlorer Error"),
        "Changing date not implemented yet", QMessageBox::Ok);
}

void FlexDateDelegate::updateEditorGeometry(
        QWidget *editor,
        const QStyleOptionViewItem &option,
        const QModelIndex & /*index*/) const
{
    editor->setGeometry(option.rect);
}

FlexDateTimeDelegate::FlexDateTimeDelegate(
        const FileTimeAccess &p_fileTimeAccess,
        QWidget *p_parentWidget) :
    fileTimeAccess(p_fileTimeAccess),
    parentWidget(p_parentWidget)
{
}

QString FlexDateTimeDelegate::displayText(const QVariant &value,
        const QLocale &locale) const
{
    return locale.toString(value.toDateTime(), QLocale::ShortFormat);
}

QWidget *FlexDateTimeDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem & /*option*/,
        const QModelIndex & /*index*/) const
{
    auto *editor = new QDateTimeEdit(parent);

    editor->setFrame(false);
    // Limit the date range to valid FLEX dates.
    editor->setDateRange(QDate(1975, 1, 1), QDate(2074, 12, 31));
    editor->setSelectedSection(QDateTimeEdit::YearSection);

    return editor;
}

void FlexDateTimeDelegate::setEditorData(QWidget *editor,
                                         const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::EditRole).toDateTime();
    auto *dateTimeEditor = dynamic_cast<QDateTimeEdit *>(editor);
    assert(dateTimeEditor != nullptr);
    dateTimeEditor->setDateTime(value);
}

void FlexDateTimeDelegate::setModelData(QWidget *editor,
                                        QAbstractItemModel *model,
                                        const QModelIndex &index) const
{
    auto *dateTimeEditor = dynamic_cast<QDateTimeEdit *>(editor);
    assert(dateTimeEditor != nullptr);
    auto value = dateTimeEditor->dateTime();
    model->setData(index, value, Qt::EditRole);
    //TODO: Set date and time in model
    QMessageBox::warning(parentWidget, tr("FLEXPlorer Error"),
        "Changing date and time not implemented yet", QMessageBox::Ok);
}

void FlexDateTimeDelegate::updateEditorGeometry(
        QWidget *editor,
        const QStyleOptionViewItem &option,
        const QModelIndex & /*index*/) const
{
    editor->setGeometry(option.rect);
}

FlexFilenameDelegate::FlexFilenameDelegate(QWidget *p_parentWidget) :
    parentWidget(p_parentWidget)
{
}

QWidget *FlexFilenameDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem & /*option*/,
        const QModelIndex &index) const
{
    // The editor is a QLineEdit. It uses a special Validator checking
    // for the FLEX filename syntax and also rejects duplicate filenames.
    QString rxString("[A-Za-z][A-Za-z0-9_-]{0,7}\\.[A-Za-z][A-Za-z0-9_-]{0,2}");
    QRegularExpression regex(rxString);
    const auto *model =
        dynamic_cast<const FlexplorerTableModel *>(index.model());
    assert(model != nullptr);
    auto filenames = model->GetFilenames();
    filenames.removeAll(model->GetFilename(index));
    auto *validator =
        new FlexRegularExpressionValidator(regex, parent, filenames);
    auto *editor = new QLineEdit(parent);

    editor->setValidator(validator);
    editor->setMaxLength(12);
    editor->setFrame(false);

    return editor;
}

void FlexFilenameDelegate::setEditorData(QWidget *editor,
                                         const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::EditRole).toString();
    auto *filenameEditor = dynamic_cast<QLineEdit *>(editor);
    assert(filenameEditor != nullptr);
    filenameEditor->setText(value);
}

void FlexFilenameDelegate::setModelData(
        QWidget *editor,
        QAbstractItemModel *abstractModel,
        const QModelIndex &index) const
{
    auto *model = dynamic_cast<FlexplorerTableModel *>(abstractModel);
    assert(model != nullptr);
    auto *filenameEditor = dynamic_cast<QLineEdit *>(editor);
    assert(filenameEditor != nullptr);
    auto newFilename = filenameEditor->text().toUpper();

    try
    {
        model->RenameFile(index, newFilename);
    }
    catch (FlexException &ex)
    {
        QMessageBox::critical(parentWidget, tr("FLEXPlorer Error"),
                              ex.what(), QMessageBox::Ok);
    }
}

void FlexFilenameDelegate::updateEditorGeometry(
        QWidget *editor,
        const QStyleOptionViewItem &option,
        const QModelIndex & /*index*/) const
{
    editor->setGeometry(option.rect);
}

static QString GetAttributesRegularExpression(const QStringList &items)
{
    QStringList regexItems;

    for (const auto &item : items)
    {
        QString regexString;

        for (auto ch : item)
        {
            regexString +=
                QString("[%1%2]").arg(ch.toLower()).arg(ch.toUpper());
        }
        regexItems.append(regexString);
    }

    return regexItems.join('|');
}

FlexAttributesDelegate::FlexAttributesDelegate(
        const QString &p_supportedAttributes, QWidget *parent) :
    supportedAttributes(p_supportedAttributes.toUpper()),
    parentWidget(parent)
{
}

QWidget *FlexAttributesDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem & /*option*/,
        const QModelIndex & /*index*/) const
{
    // The inplace editor is a QLineEdit. An attribute is a bitfield of
    // each of the following four properties:
    //
    // "W": Write protect
    // "D": Delete protect
    // "R": Read protect
    // "C": Catalog protect
    //
    // A regular expression together with a QCompleter combobox is used
    // for a good inline edit support.
    const auto attributeStrings = GetAllCombinations(supportedAttributes);
    auto regexString = GetAttributesRegularExpression(attributeStrings);
    QRegularExpression regex(regexString);
    auto *validator = new QRegularExpressionValidator(regex, parent);
    auto *completer = new QCompleter(attributeStrings, parent);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setMaxVisibleItems(cast_from_qsizetype(attributeStrings.size()));
    auto *editor = new QLineEdit(parent);

    editor->setValidator(validator);
    editor->setCompleter(completer);
    editor->setMaxLength(4);
    editor->setFrame(false);

    return editor;
}

void FlexAttributesDelegate::setEditorData(QWidget *editor,
                                           const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::EditRole).toString();
    auto *filenameEditor = dynamic_cast<QLineEdit *>(editor);
    assert(filenameEditor != nullptr);
    filenameEditor->setText(value);
}

void FlexAttributesDelegate::setModelData(QWidget *editor,
                                          QAbstractItemModel *abstractModel,
                                          const QModelIndex &index) const
{
    auto *model = dynamic_cast<FlexplorerTableModel *>(abstractModel);
    assert(model != nullptr);
    auto *attributesEditor = dynamic_cast<QLineEdit *>(editor);
    assert(attributesEditor != nullptr);
    auto value = attributesEditor->text().toUpper();

    try
    {
        model->SetAttributesString(index, value);
    }
    catch (FlexException &ex)
    {
        QMessageBox::critical(parentWidget, tr("FLEXPlorer Error"),
                              ex.what(), QMessageBox::Ok);
    }
}

void FlexAttributesDelegate::updateEditorGeometry(
        QWidget *editor,
        const QStyleOptionViewItem &option,
        const QModelIndex & /*index*/) const
{
    editor->setGeometry(option.rect);
}

void FlexAttributesDelegate::GetCombinations(const QString &chars, int length,
                                             int start, QString &current,
                                             QStringList &result)
{
    if (length > 0)
    {
        for (int i = start; i <= chars.size() - length; ++i)
        {
            current[current.size() - length] = chars[i];
            GetCombinations(chars, length - 1, i + 1, current, result);
        }
    }
    else
    {
        result.append(current);
    }
}

QStringList FlexAttributesDelegate::GetAllCombinations(
        const QString &attributes)
{
    QStringList result;

    for (int length = 0; length <= attributes.size(); ++length)
    {
        QString current(length, ' ');

        GetCombinations(attributes, length, 0, current, result);
    }
    result.sort();

    return result;
}

