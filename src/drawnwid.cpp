/*
    drawnwid.cpp  A QWidget subclass implementing a drawn widget.


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2026  W. Schwotzer

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


#include "drawnwid.h"
#include "fcinfo.h"
#include "warnoff.h"
#include <QObject>
#include <QString>
#include <QPalette>
#include <QBrush>
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFont>
#include <QDate>
#include <QLocale>
#include <QWidget>
#include "warnon.h"
#include <cassert>


DrawnWidget::DrawnWidget(QWidget *parent)
    : QWidget(parent)
    , preferredSize(16, 16)
{
}

QSize DrawnWidget::minimumSizeHint() const
{
    return preferredSize;
}

QSize DrawnWidget::sizeHint() const
{
    return preferredSize;
}

void DrawnWidget::SetPixmap(const QPixmap &p_pixmap)
{
    pixmap = p_pixmap;
    preferredSize = pixmap.size();
    updateGeometry();
}

void DrawnWidget::SetDiskAttributes(const FlexDiskAttributes &p_diskAttributes)
{
    diskAttributes = p_diskAttributes;
}

void DrawnWidget::paintEvent(QPaintEvent *event)
{
    assert(event != nullptr);

    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);

    if (diskAttributes.GetIsFlexFormat() && pixmap.size() == QSize(256, 256))
    {
        int tracks;
        int sectors;
        int x = 16;
        int y = 27;

        diskAttributes.GetTrackSector(tracks, sectors);
        QString trkSec = tr("%1/%2 trk/sec").arg(tracks).arg(sectors);
        auto name = QString::fromStdString((diskAttributes.GetDiskname()));
        QString numberString = tr("#%1").arg(diskAttributes.GetNumber());
        auto date = diskAttributes.GetDate();
        auto qdate = QDate(date.GetYear(), date.GetMonth(), date.GetDay());
        auto locale = QLocale::system();
        auto dateString = locale.toString(qdate, QLocale::ShortFormat);
        painter.setPen(Qt::black);
        painter.drawText(x, y, name);
        painter.drawText(x, y+15, numberString);
        painter.drawText(x, y+30, dateString);
        painter.drawText(x, y+45, trkSec);

        if (!diskAttributes.GetIsWriteProtected())
        {
            // Paint the write enable notch.
            auto backgroundColor = palette().color(QPalette::Window);
            auto brush = QBrush(backgroundColor);
            painter.fillRect(width() - 8, 60, 8, 13, brush);
        }
    }
    event->accept();
}

