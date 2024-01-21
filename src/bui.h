/*
    bui.h


    Flexemu, an MC6809 emulator running FLEX
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


#ifndef BUI_INCLUDE
#define BUI_INCLUDE

#include "efslctle.h"
#include "warnoff.h"
#include <QString>
#include <QLineEdit>
#include <QLatin1Char>
#include "warnon.h"


extern void InstallSelectionEventFilter(QLineEdit &lineEdit, QObject *parent);

template<class T1, class T2>
void SetData(T1 value, T2 &widget)
{
    if (value <= 0xFFFF)
    {
        widget.setText(QString("%1").arg(value, 4, 16, QLatin1Char('0')));
    }
    else
    {
        widget.setText("");
    }
}

template<class T1, class T2>
T1 GetData(T2 &widget)
{
    T1 value;
    bool isSuccess;

    value = widget.text().toUInt(&isSuccess, 16);
    if (!isSuccess)
    {
        value = 0x10000;
    }

    return value;
}

#endif

