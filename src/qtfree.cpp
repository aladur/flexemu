/*                                                                              
    qtfree.cpp

 
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


#include "warnoff.h"
#include <QLatin1Char>
#include <QFontDatabase>
#include <QWidget>
#include "warnon.h"
#include <cmath>
#include "qtfree.h"

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

    return QFont();
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

