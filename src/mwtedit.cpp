/*
    mwtedit.cpp


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


#include "mwtedit.h"
#include "warnoff.h"
#include <QString>
#include <QWidget>
#include <QTextEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QInputMethodEvent>
#include <QGuiApplication>
#include <QSet>
#include "warnon.h"
#include <cassert>


MemoryWindowTextEdit::MemoryWindowTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    assert(GetAllAsciiCharacters().size() == 128U - 32U - 1U);

#ifdef UNIX
    // Set attribute to receive input method events.
    setAttribute(Qt::WA_InputMethodEnabled, true);
#endif
}

void MemoryWindowTextEdit::mouseMoveEvent(QMouseEvent *event)
{
    // Avoid selections.
    UpdateEventType(event->type());
}

// Support mouse press event to position the text cursor.
void MemoryWindowTextEdit::mousePressEvent(QMouseEvent *event)
{
    UpdateEventType(event->type());
    QTextEdit::mousePressEvent(event);
}

void MemoryWindowTextEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Avoid selections.
    UpdateEventType(event->type());
}

// Filter key press events. Only allow:
// * cursor navigation
// * HEX or ASCII characters
// * HEX characters are converted to uppercase
void MemoryWindowTextEdit::keyPressEvent(QKeyEvent *event)
{
    // List of supported keys (without modifier).
    static const QSet<int> supportedKeys{
        Qt::Key_Up,
        Qt::Key_Down,
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_PageUp,
        Qt::Key_PageDown,
        Qt::Key_Home,
        Qt::Key_End,
    };
    // List of supported control keys.
    static const QSet<int> supportedControlKeys{
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_Home,
        Qt::Key_End,
    };

    UpdateEventType(event->type());

    switch (event->modifiers())
    {
        case Qt::NoModifier:
        case Qt::KeypadModifier:
            if (supportedKeys.contains(event->key()))
            {
                NotifyKeyPressed(event);
                QTextEdit::keyPressEvent(event);
                return;
            }
            break;

        case Qt::ControlModifier:
            if (event->key() == Qt::Key_A)
            {
                NotifyKeyPressed(event);
            }
            else if (supportedControlKeys.contains(event->key()))
            {
                NotifyKeyPressed(event);
                QTextEdit::keyPressEvent(event);
                return;
            }
            break;
    }

    // If the key represents a lowercase character and it is about
    // editing a hex value create a new event containing the corresponding
    // uppercase character.
    // Check for currently valid character (no Ctrl keys).
    const bool isValidChar = (event->text().size() == 1 &&
            validCharacters.contains(event->text().at(0).toLatin1()));

    if (isValidChar)
    {
        bool toUpperCase = !validCharacters.contains(' ') &&
                event->text().size() == 1 &&
                event->text()[0] >= 'a' && event->text()[0] <= 'z';
        const auto modifier = event->modifiers() |
            (toUpperCase ?  Qt::ShiftModifier : Qt::NoModifier);
        const auto newText = toUpperCase ?
            event->text().toUpper() : event->text();
        QKeyEvent newEvent(QEvent::KeyPress, event->key(), modifier, newText);

        NotifyKeyPressed(&newEvent);
        QTextEdit::keyPressEvent(&newEvent);
    }

    event->accept();
}

// This input method is used on Linux and MacOS to process dead keys.
// The input method event processing dead keys does not work in this
// context because overwrite mode is active and the input method
// processing tries to insert characters which breaks the hex-dump
// format.
#if defined(__linux__) || defined(__APPLE__)
void MemoryWindowTextEdit::inputMethodEvent(QInputMethodEvent *event)
{
    if (auto ch = event->preeditString()[0].toLatin1();
        event->preeditString().size() == 1 && ch >= ' ' && ch <= '~')
    {
        // If pre-edit string contains an ASCII key generate a key press
        // event with it. All other keys are ignored.
        QKeyEvent newEvent(QEvent::KeyPress, ch, Qt::NoModifier,
                           event->preeditString());
        keyPressEvent(&newEvent);
    }

#if !defined(__APPLE__)
    // Abort the pending dead key processing.
    QGuiApplication::inputMethod()->reset();
#endif

    event->accept();
}
#endif

void MemoryWindowTextEdit::SetValidCharacters(const QString &p_validCharacters)
{
    validCharacters = p_validCharacters;
}

const QString &MemoryWindowTextEdit::GetAllHexCharacters()
{
    static const QString hexCharacters{ "0123456789ABCDEFabcdef" };

    return hexCharacters;
}

const QString &MemoryWindowTextEdit::GetAllAsciiCharacters()
{
    static const QString asciiCharacters{
        " !\"#$%&'()*+,-./0123456789:;<=>?@"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
        "abcdefghijklmnopqrstuvwxyz{|}~"
    };

    return asciiCharacters;
}

void MemoryWindowTextEdit::UpdateEventType(QEvent::Type eventType)
{
    if (lastEventType != eventType)
    {
        lastEventType = eventType;
        EventTypeChanged(lastEventType);
    }
}
