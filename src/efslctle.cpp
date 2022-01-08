/*
    efslctle.cpp


    Flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2022  W. Schwotzer

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


#include "efslctle.h"
#include <cassert>
#include <QEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QLineEdit>


SelectAllOnFocusInLE::SelectAllOnFocusInLE(
        QLineEdit &p_widget, QObject *parent) :
    QObject(parent), widget(p_widget)
{
}

bool SelectAllOnFocusInLE::eventFilter(QObject *object, QEvent *event)
{
    assert(event != nullptr);

    if (event->type() == QEvent::FocusIn)
    {
        auto *focusEvent = static_cast<QFocusEvent *>(event);

        if (focusEvent->gotFocus())
        {
            DoSelectAll();
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::LeftButton)
        {
            DoSelectAll();
        }
    }
    else
    {
        return QObject::eventFilter(object, event);
    }

    return true;
}

void SelectAllOnFocusInLE::DoSelectAll()
{
    widget.selectAll();
}

