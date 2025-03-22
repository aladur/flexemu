/*
    qtfree.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023-2025  W. Schwotzer

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
#include <QObject>
#include <QLatin1Char>
#include <QString>
#include <QStringList>
#include <QLocale>
#include <QDate>
#include <QPixmap>
#include <QFontDatabase>
#include <QMenu>
#include <QWidget>
#include <QDialog>
#include <QStandardItem>
#include <QStandardItemModel>
#include <optional>
#include "warnon.h"
#include "typedefs.h"
#include "qtfree.h"
#include "propsui.h"
#include "fcinfo.h"
#include "e2.h"
#include <cmath>


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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            return QFontDatabase::font(family, style, pointSize);
#else
            auto fontDatabase = QFontDatabase();
            return fontDatabase.font(family, style, pointSize);
#endif
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
    QStandardItemModel model;
    auto *dialog = new QDialog(parent);
    Ui::Properties ui;

    ui.setupUi(dialog);
    ui.SetDiskAttributes(diskAttributes);
    model.setColumnCount(2);

    if (diskAttributes.IsValid())
    {
        const auto tYes = QObject::tr("yes");
        const auto tNo = QObject::tr("no");
        const auto options = diskAttributes.GetOptions();
        const bool is_trk_sec_valid =
            (diskAttributes.GetType() != DiskType::Directory) ||
            (options & DiskOptions::HasSectorIF) != DiskOptions::NONE;
        int row = 0;
        auto rowCount = 5;
        QString text;

        rowCount += driveNumber.has_value() ? 1 : 0;
        rowCount += diskAttributes.GetIsFlexFormat() ? 4 : 0;
        rowCount += is_trk_sec_valid ? 3 : 0;
        rowCount += (diskAttributes.GetType() == DiskType::DSK) ? 1 : 0;
        model.setRowCount(rowCount);

        if (driveNumber.has_value())
        {
            model.setItem(row, 0, new QStandardItem(QObject::tr("Drive")));
            text = QString("#%1").arg(driveNumber.value());
            model.setItem(row++, 1, new QStandardItem(text));
        }

        model.setItem(row, 0, new QStandardItem(QObject::tr("Type")));
        text = diskAttributes.GetTypeString().c_str();
        model.setItem(row++, 1, new QStandardItem(text));
        model.setItem(row, 0, new QStandardItem(QObject::tr("Path")));
        text = QString::fromStdString(diskAttributes.GetPath().u8string());
        model.setItem(row++, 1, new QStandardItem(text));

        if (diskAttributes.GetIsFlexFormat())
        {
            model.setItem(row, 0, new QStandardItem(QObject::tr("Name")));
            text = QString::fromStdString(diskAttributes.GetDiskname());
            model.setItem(row++, 1, new QStandardItem(text));
            model.setItem(row, 0, new QStandardItem(QObject::tr("Number")));
            text = QString::number(diskAttributes.GetNumber());
            model.setItem(row++, 1, new QStandardItem(text));
            model.setItem(row, 0, new QStandardItem(QObject::tr("Date")));
            const auto& date = diskAttributes.GetDate();
            auto qdate = QDate(
                date.GetYear(), date.GetMonth(), date.GetDay());
            text = QLocale::system().toString(qdate);
            model.setItem(row++, 1, new QStandardItem(text));
            text = QObject::tr("Free [KByte]");
            model.setItem(row, 0, new QStandardItem(text));
            const auto free = static_cast<double>(diskAttributes.GetFree()) /
                                  1024;
            model.setItem(row++, 1, new QStandardItem(QString::number(free)));
        }

        model.setItem(row, 0, new QStandardItem(QObject::tr("Size [KByte]")));
        const auto size = static_cast<double>(diskAttributes.GetTotalSize()) /
                               1024.0;
        model.setItem(row++, 1, new QStandardItem(QString::number(size)));

        if (is_trk_sec_valid)
        {
            int tracks;
            int sectors;

            text = QObject::tr("Sectorsize [Byte]");
            model.setItem(row, 0, new QStandardItem(text));
            text = QString::number(diskAttributes.GetSectorSize());
            model.setItem(row++, 1, new QStandardItem(text));
            diskAttributes.GetTrackSector(tracks, sectors);
            model.setItem(row, 0, new QStandardItem(QObject::tr("Tracks")));
            model.setItem(row++, 1, new QStandardItem(QString::number(tracks)));
            model.setItem(row, 0, new QStandardItem(QObject::tr("Sectors")));
            text = QString::number(sectors);
            model.setItem(row++, 1, new QStandardItem(text));
        }

        text = QObject::tr("Write-protect");
        model.setItem(row, 0, new QStandardItem(text));
        text = diskAttributes.GetIsWriteProtected() ? tYes : tNo;
        model.setItem(row++, 1, new QStandardItem(text));
        model.setItem(row, 0, new QStandardItem(QObject::tr("FLEX format")));
        text = diskAttributes.GetIsFlexFormat() ? tYes : tNo;
        model.setItem(row++, 1, new QStandardItem(text));

        if (diskAttributes.GetType() == DiskType::DSK)
        {
            auto header = diskAttributes.GetJvcFileHeader();

            text = QObject::tr("JVC header");
            model.setItem(row, 0, new QStandardItem(text));
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
            model.setItem(0, 0, new QStandardItem(QObject::tr("Drive")));
            auto text = QString("#%1").arg(driveNumber.value());
            model.setItem(0, 1, new QStandardItem(text));
        }

        model.setItem(1, 0, new QStandardItem(QObject::tr("Status")));
        model.setItem(1, 1, new QStandardItem(QObject::tr("Not ready")));
    }

    dialog->setWindowTitle(title);
    dialog->setModal(true);
    dialog->setSizeGripEnabled(true);

    ui.SetModel(model, { "Property", "Value" });
    ui.SetMinimumSize(dialog);

    dialog->exec();
}

QAction *CreateIconSizeAction(QMenu &menu, uint16_t index)
{
    static const QStringList menuText{
        QObject::tr("&Small"),
        QObject::tr("&Medium"),
        QObject::tr("&Large"),
    };
    static const QStringList toolTipText{
        QObject::tr("Show small size Icons"),
        QObject::tr("Show medium size Icons"),
        QObject::tr("Show large size Icons"),
    };

    assert(menuText.size() == ICON_SIZES);
    assert(toolTipText.size() == ICON_SIZES);
    assert(index < ICON_SIZES);

    auto *action = menu.addAction(menuText[index]);
    action->setCheckable(true);
    action->setStatusTip(toolTipText[index]);

    return action;
  }

int IconSizeToIndex(const QSize &iconSize)
{
   int index = (iconSize.width() >= 24 || iconSize.height() >= 24) ? 1 : 0;
   return (iconSize.width() >= 32 || iconSize.height() >= 32) ? 2 : index;
}

