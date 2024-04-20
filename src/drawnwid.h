/*
    drawnwid.h  A QWidget subclass implementing a drawn widget.


    flexemu, an MC6809 emulator running FLEX
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

#ifndef DRAWNWID_INCLUDED
#define DRAWNWID_INCLUDED

#include "fcinfo.h"
#include <memory>
#include "warnoff.h"
#include <QSize>
#include <QWidget>
#include <QPixmap>
#include "warnon.h"

class QPaintEvent;
class QByteArray;
class QFont;


class DrawnWidget : public QWidget
{
    Q_OBJECT

public:
    DrawnWidget() = delete;
    explicit DrawnWidget(QWidget *parent);
    ~DrawnWidget() override = default;

    void SetPixmap(const QPixmap &p_pixmap);
    void SetDriveInfo(Word number, const FlexContainerInfo &info);

    // QWidget Overrides
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap pixmap;
    QSize preferredSize;
    Word driveNumber;
    FlexContainerInfo driveInfo;
};
#endif

