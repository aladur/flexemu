/*
    efslctle.h


    Flexemu, an MC6809 emulator running FLEX
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


#ifndef EFSLCTLE_INCLUDED
#define EFSLCTLE_INCLUDED

#include "warnoff.h"
#include <QObject>
#include "warnon.h"


class QEvent;
class QLineEdit;

class SelectAllOnFocusInLE : public QObject
{
    Q_OBJECT

public:
    SelectAllOnFocusInLE() = delete;
    explicit SelectAllOnFocusInLE(QLineEdit &p_widget,
            QObject *parent = Q_NULLPTR);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void DoSelectAll();

private:
    QLineEdit &widget;
};
#endif

