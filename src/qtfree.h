/*
    qtfree.h


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

#ifndef QTFREE_INCLUDED
#define QTFREE_INCLUDED

#include "warnoff.h"
#include <QSize>
#include <QString>
#include <QFont>
#include <QWidget>
#include <QToolBar>
#include <optional>
#include "warnon.h"
#include "typedefs.h"
#include "e2.h"
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

class QAction;
class QMenu;
class FlexDiskAttributes;

using ItemPairList_t = std::vector<std::pair<
          std::string, std::vector<std::string> > >;

extern QFont GetFont(const QString &fontName);
extern QString GetWindowGeometry(const QWidget &w);
extern void UpdateWindowGeometry(QWidget &w, const QString &geometry);
extern QString StripPath(const QString &path, int maxSize = 64);
extern void OpenDiskStatusDialog(QWidget *parent,
        const QString &title,
        const FlexDiskAttributes &diskAttributes,
        std::optional<Word> driveNumber = std::nullopt);
extern QAction *CreateIconSizeAction(QMenu &menu, uint16_t index);
extern int IconSizeToIndex(const QSize &iconSize);
extern QString ConvertItemPairListToHtml(const ItemPairList_t &pairs);

class UpdateWindowGeometryFtor
{
    QWidget *widget;
    QString geometry;

public:
    UpdateWindowGeometryFtor() = delete;
    UpdateWindowGeometryFtor(QWidget *w, const std::string &p_geometry) :
        widget(w)
      , geometry(QString::fromStdString(p_geometry)) { }
    UpdateWindowGeometryFtor(const UpdateWindowGeometryFtor &f) = default;
    void operator() ()
    {
        ::UpdateWindowGeometry(*widget, geometry);
    }
};

class FlexemuToolBar : public QToolBar
{
    int pixelSize;

public:
    FlexemuToolBar() = delete;
    FlexemuToolBar(const QString &title, QWidget *parent)
      : QToolBar(title, parent)
      , pixelSize(1) { }

    void SetPixelSize(int p_pixelSize)
    {
        pixelSize = p_pixelSize;
    }

    QSize sizeHint() const override
    {
        auto sizeHint = QToolBar::sizeHint();
        if (pixelSize <= 1)
        {
            // If pixel size is 1 limit it's width to a maximum of
            // Eurocom II window width. This avoids a streched Eurocom II
            // window if icon size is >= 32.
            static const auto maxWidth = static_cast<int>(WINDOWWIDTH);

            sizeHint.setWidth(std::min(maxWidth, sizeHint.width()));
        }

        return sizeHint;
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

