/*                                                                              
    qtfree.h

 
    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023  W. Schwotzer

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

#include <QString>
#include <QFont>
#include <string>

class QWidget;

extern QFont GetFont(const QString &fontName);
extern QString GetWindowGeometry(const QWidget &w);
extern void UpdateWindowGeometry(QWidget &w, const QString &geometry);

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

#endif

