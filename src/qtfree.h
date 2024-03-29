/*
    qtfree.h


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

#ifndef QTFREE_INCLUDED
#define QTFREE_INCLUDED

#include "warnoff.h"
#include <QString>
#include <QFont>
#include "warnon.h"
#include <string>

class QWidget;

extern QFont GetFont(const QString &fontName);
extern QString GetWindowGeometry(const QWidget &w);
extern void UpdateWindowGeometry(QWidget &w, const QString &geometry);
extern QString StripPath(const QString &path, int maxSize = 64);

class UpdateWindowGeometryFtor
{
    QWidget *widget;
    QString geometry;

public:
    UpdateWindowGeometryFtor() = delete;
    UpdateWindowGeometryFtor(QWidget *w, const std::string &p_geometry) :
        widget(w)
      , geometry(QString(p_geometry.c_str())) { }
    UpdateWindowGeometryFtor(const UpdateWindowGeometryFtor &f) :
        widget(f.widget)
      , geometry(f.geometry) { }
    void operator() ()
    {
        ::UpdateWindowGeometry(*widget, geometry);
    }
};

// Conditional cast from long long int or long int to int.
// This is used for Qt5/Qt6 size_type compatibility, and should only
// be used in this context.
// On Qt5 size_type is int, no cast is needed.
// On Qt6 size_type is posix type ssize_t (long int or long long int,
// depending on the data model), a cast is needed.
// See also https://en.cppreference.com/w/cpp/language/types
extern int cast_from_qsizetype(int source);
extern int cast_from_qsizetype(long int source);
extern int cast_from_qsizetype(long long int source);

#endif

