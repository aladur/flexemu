/*
    qtfree.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023-2024  W. Schwotzer

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


#include "warnoff.h"
#include <QLatin1Char>
#include <QString>
#include <QLocale>
#include <QDate>
#include <QPixmap>
#include <QFontDatabase>
#include <QWidget>
#include <QDialog>
#include <QStandardItem>
#include <QStandardItemModel>
#include "warnon.h"
#include "typedefs.h"
#include "qtfree.h"
#include "propsui.h"
#include "fcinfo.h"
#include <cmath>
#include <optional>

QFont GetFont(const QString &fontName)
{
    auto list = fontName.split(QLatin1Char(','));

    if (list.size() >= 4)
    {
        bool ok = false;
        const auto &family = list[0];
        const auto &style = list[3];
        const auto &pointSizeString = list[1];
        auto pointSize =
            static_cast<int>(std::round(pointSizeString.toFloat(&ok)));

        if (ok && pointSize > 0)
        {
            auto fontDatabase = QFontDatabase();
            return fontDatabase.font(family, style, pointSize);
        }
    }

    return {};
}

QString GetWindowGeometry(const QWidget &w)
{
    return QString(
        "%1,%2,%3,%4").arg(w.width()).arg(w.height()).arg(w.x()).arg(w.y());
}

void UpdateWindowGeometry(QWidget &w, const QString &geometry)
{
    auto list = geometry.split(QLatin1Char(','));
    bool ok1 = false;
    bool ok2 = false;

    if (list.size() >= 2)
    {
        auto width = list[0].toInt(&ok1);
        auto height = list[1].toInt(&ok2);
        if (ok1 && ok2)
        {
            w.resize(QSize(width, height));
        }
    }

    if (list.size() == 4)
    {
        auto x = list[2].toInt(&ok1);
        auto y = list[3].toInt(&ok2);
        if (ok1 && ok2)
        {
            w.move(QPoint(x, y));
        }
    }
}

// Strip a path to a max. length (for data display).
QString StripPath(const QString &path, int maxSize)
{
    if (path.size() > maxSize)
    {
        return path.left(maxSize/2 - 1) + "..." + path.right(maxSize/2 - 2);
    }

    return path;
}

int cast_from_qsizetype(int source)
{
    return source;
}

int cast_from_qsizetype(long int source)
{
    return static_cast<int>(source);
}

int cast_from_qsizetype(long long int source)
{
    return static_cast<int>(source);
}

void OpenDiskStatusDialog(QWidget *parent,
        const QString &title,
        const FlexDiskAttributes &diskAttributes,
        std::optional<Word> driveNumber)
{
    QString text;
    int tracks;
    int sectors;
    QStandardItemModel model;
    auto *dialog = new QDialog(parent);
    Ui::Properties ui;
    int row = 0;

    ui.setupUi(dialog);
    ui.SetDriveAttributes(diskAttributes);

    model.setColumnCount(2);
    if (diskAttributes.IsValid())
    {
        const auto tYes = QObject::tr("yes");
        const auto tNo = QObject::tr("no");
        auto rowCount = 7;

        rowCount += diskAttributes.GetIsFlexFormat() ? 4 : 0;
        rowCount += (diskAttributes.GetType() == DiskType::DSK) ? 1 : 0;
        rowCount += driveNumber.has_value() ? 1 : 0;
        model.setRowCount(rowCount);
        diskAttributes.GetTrackSector(tracks, sectors);
        if (driveNumber.has_value())
        {
            model.setItem(row++, 0, new QStandardItem(QObject::tr("Drive")));
        }
        model.setItem(row++, 0, new QStandardItem(QObject::tr("Type")));
        model.setItem(row++, 0, new QStandardItem(QObject::tr("Path")));
        if (diskAttributes.GetIsFlexFormat())
        {
            model.setItem(row++, 0, new QStandardItem(QObject::tr("Name")));
            model.setItem(row++, 0, new QStandardItem(QObject::tr("Number")));
            model.setItem(row++, 0, new QStandardItem(QObject::tr("Date")));
            text = QObject::tr("Free [KByte]");
            model.setItem(row++, 0, new QStandardItem(text));
        }
        model.setItem(row++, 0, new QStandardItem(QObject::tr("Size [KByte]")));
        model.setItem(row++, 0, new QStandardItem(QObject::tr("Tracks")));
        model.setItem(row++, 0, new QStandardItem(QObject::tr("Sectors")));
        text = QObject::tr("Write-protect");
        model.setItem(row++, 0, new QStandardItem(text));
        model.setItem(row++, 0, new QStandardItem(QObject::tr("FLEX format")));
        if (diskAttributes.GetType() == DiskType::DSK)
        {
            text = QObject::tr("JVC header");
            model.setItem(row++, 0, new QStandardItem(text));
        }

        row = 0;
        if (driveNumber.has_value())
        {
            text = QString("#%1").arg(driveNumber.value());
            model.setItem(row++, 1, new QStandardItem(text));
        }
        text = diskAttributes.GetTypeString().c_str();
        model.setItem(row++, 1, new QStandardItem(text));
        text = diskAttributes.GetPath().c_str();
        model.setItem(row++, 1, new QStandardItem(text));
        if (diskAttributes.GetIsFlexFormat())
        {
            text = diskAttributes.GetName().c_str();
            model.setItem(row++, 1, new QStandardItem(text));
            text = QString::number(diskAttributes.GetNumber());
            model.setItem(row++, 1, new QStandardItem(text));
            const auto& date = diskAttributes.GetDate();
            auto qdate = QDate(
                date.GetYear(), date.GetMonth(), date.GetDay());
            text = QLocale::system().toString(qdate);
            model.setItem(row++, 1, new QStandardItem(text));
            const auto free = static_cast<double>(diskAttributes.GetFree()) /
                                  1024;
            model.setItem(row++, 1, new QStandardItem(QString::number(free)));
        }
        const auto size = static_cast<double>(diskAttributes.GetTotalSize()) /
                               1024.0;
        model.setItem(row++, 1, new QStandardItem(QString::number(size)));
        model.setItem(row++, 1, new QStandardItem(QString::number(tracks)));
        model.setItem(row++, 1, new QStandardItem(QString::number(sectors)));
        text = diskAttributes.GetIsWriteProtected() ? tYes : tNo;
        model.setItem(row++, 1, new QStandardItem(text));
        text = diskAttributes.GetIsFlexFormat() ? tYes : tNo;
        model.setItem(row++, 1, new QStandardItem(text));
        if (diskAttributes.GetType() == DiskType::DSK)
        {
            auto header = diskAttributes.GetJvcFileHeader();

            if (header.empty())
            {
                text = QObject::tr("none");
            }
            else
            {
                text = "";
                bool isAppend = false;
                for (const auto value : header)
                {
                    text += (isAppend ? "," : "");
                    text += QString::number(static_cast<Word>(value));
                    isAppend = true;
                }
            }
            model.setItem(row++, 1, new QStandardItem(text));
        }

        auto floppyPixmap = QPixmap(":/resource/floppy256.png");
        ui.SetPixmap(floppyPixmap);
    }
    else
    {
        model.setRowCount(driveNumber.has_value() ? 2 : 1);
        if (driveNumber.has_value())
        {
            model.setItem(row++, 0, new QStandardItem(QObject::tr("Drive")));
        }
        model.setItem(row++, 0, new QStandardItem(QObject::tr("Status")));

        row = 0;
        if (driveNumber.has_value())
        {
            text = QString("#%1").arg(driveNumber.value());
            model.setItem(row++, 1, new QStandardItem(text));
        }
        model.setItem(row++, 1, new QStandardItem(QObject::tr("Not ready")));
    }

    dialog->setWindowTitle(title);
    dialog->setModal(true);
    dialog->setSizeGripEnabled(true);

    ui.SetModel(model, { "Property", "Value" });
    ui.SetMinimumSize(dialog);

    dialog->exec();
}

