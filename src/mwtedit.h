/*
    mwtedit.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025-2026  W. Schwotzer

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


#ifndef MEMORYWINDOWTEXTEDIT_INCLUDED
#define MEMORYWINDOWTEXTEDIT_INCLUDED

#include "warnoff.h"
#include <QString>
#include <QTextEdit>
#include <QEvent>
#include "warnon.h"

class QWidget;
class QKeyEvent;
class QMouseEvent;
class QInputMethodEvent;

// A subclass of QTextEdit is needed here to suppress any event
// which would modify the hex-dump format (copy, paste, delete,
// select and replace).
class MemoryWindowTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit MemoryWindowTextEdit(QWidget *parent);

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
#if defined(__linux__) || defined(__APPLE__)
    void inputMethodEvent(QInputMethodEvent *event) override;
#endif
    void SetValidCharacters(const QString &p_validCharacters);

    static const QString &GetAllHexCharacters();
    static const QString &GetAllAsciiCharacters();

signals:
    // Notification about a key press event with valid key in progress.
    // It comes before the QTextEdit::cursorPositionChanged signal.
    void NotifyKeyPressed(QKeyEvent *event);
    // Signal emitted if event type changed. Supported event types:
    // QEvent::None, QEvent::MouseButtonDblClick, QEvent::MouseButtonPress,
    // QEvent::MouseMove, QEvent::KeyPressEvent
    void EventTypeChanged(QEvent::Type eventType);

protected:
    void UpdateEventType(QEvent::Type eventType);

private:
    QEvent::Type lastEventType{QEvent::None};
    QString validCharacters;
};

#endif
