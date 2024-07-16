/*
    propsui.h


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


#ifndef PROPSUI_H
#define PROPSUI_H

#include "warnoff.h"
#include <QtGlobal>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QTableWidget>
#include <QStandardItemModel>
#include <QHeaderView>
#include "warnon.h"
#include <numeric>
#include "drawnwid.h"
#include "qtfree.h"

class Ui_Properties
{
private:
    QHBoxLayout *mainLayout{};
    DrawnWidget *w_drawn{};
    QTableWidget *w_table{};
    QDialogButtonBox *g_buttons{};
    const int space = 2;
    int tableWidth = 80;

public:
    void setupUi(QDialog *dialog)
    {
        if (dialog->objectName().isEmpty())
        {
            dialog->setObjectName(QString::fromUtf8("PropertiesDialog"));
        }
        dialog->setSizeGripEnabled(false);

        mainLayout = new QHBoxLayout(dialog);
        mainLayout->setSpacing(space);
        mainLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        mainLayout->setContentsMargins(space, space, space, space);

        w_drawn = new DrawnWidget(dialog);
        w_drawn->setObjectName(QString::fromUtf8("w_drawn"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        w_drawn->setSizePolicy(sizePolicy);
        mainLayout->addWidget(w_drawn);

        w_table = new QTableWidget(dialog);
        w_table->setObjectName(QString::fromUtf8("w_table"));
        mainLayout->addWidget(w_table);

        g_buttons = new QDialogButtonBox(dialog);
        g_buttons->setObjectName(QString::fromUtf8("g_buttons"));
        g_buttons->setOrientation(Qt::Vertical);
        g_buttons->setStandardButtons(QDialogButtonBox::Ok);
        mainLayout->addWidget(g_buttons);

        mainLayout->setStretch(1, 1);

        retranslateUi(dialog);
        QObject::connect(g_buttons, SIGNAL(accepted()), dialog, SLOT(accept()));
        QObject::connect(g_buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialog);
    }

    void SetPixmap(const QPixmap &pixmap) const
    {
        assert(w_drawn != nullptr);

        w_drawn->SetPixmap(pixmap);
    }

    void SetDriveAttributes(Word driveNumber,
                            const FlexDiskAttributes &diskAttributes) const
    {
        assert(w_drawn != nullptr);

        w_drawn->SetDriveAttributes(driveNumber, diskAttributes);
    }

    void SetModel(const QStandardItemModel &model,
                  const QStringList &headerLabels)
    {
        QVector<int> columnWidth(model.columnCount());
        int column;

        assert(w_table != nullptr);
        assert(model.columnCount() >= 1);

        w_table->setRowCount(model.rowCount());
        w_table->setColumnCount(model.columnCount());
        w_table->setHorizontalHeaderLabels(headerLabels);
        auto mode = QHeaderView::ResizeToContents;
        w_table->horizontalHeader()->setSectionResizeMode(0, mode);
        mode = QHeaderView::Stretch;
        w_table->horizontalHeader()->setSectionResizeMode(1, mode);
        w_table->verticalHeader()->setVisible(false);

        QFontMetrics fontMetrics(w_table->font());
        for (column = 0; column < headerLabels.size(); ++column)
        {
            columnWidth[column] =
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
            columnWidth[column] =
                fontMetrics.horizontalAdvance(headerLabels[column]);
#else
            columnWidth[column] = fontMetrics.width(headerLabels[column]);
#endif
        }

        for (int row = 0; row < model.rowCount(); ++row)
        {
            for (column = 0; column < model.columnCount(); ++column)
            {
                auto *item = model.item(row, column);
                const auto text = item->text();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
                int width = fontMetrics.horizontalAdvance(text);
#else
                int width = fontMetrics.width(text);
#endif
                columnWidth[column] = std::max(width, columnWidth[column]);
                auto *newItem = new QTableWidgetItem(text);
                newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                w_table->setItem(row, column, newItem);
            }
        }

        tableWidth = std::accumulate(columnWidth.begin(), columnWidth.end(), 0);
        tableWidth += cast_from_qsizetype(columnWidth.size() * 22);
        tableWidth = std::min(tableWidth, 800);
    }

    void SetMinimumSize(QDialog *dialog) const
    {
        auto drawnSize = w_drawn->sizeHint();
        auto width = drawnSize.width() + tableWidth + g_buttons->width();
        auto rowHeight = w_table->rowHeight(0);
        auto height = rowHeight * (w_table->rowCount() + 1);
        height = 2 * space + std::max(drawnSize.height(), height);
        dialog->setMinimumSize(QSize(width, height));
    }

    static void retranslateUi(QDialog *dialog)
    {
        dialog->setWindowTitle(
                QApplication::translate("PropertiesDialog", "Dialog", nullptr));
    }

};

namespace Ui {
    class Properties: public Ui_Properties {};
}

#endif
