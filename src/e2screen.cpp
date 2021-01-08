/*
    e2screen.cpp  A QWidget subclass implementing the Eurocom II screen.


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020  W. Schwotzer

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


#include "e2screen.h"
#include "e2.h"
#include "mc6809.h"
#include "schedule.h"
#include "joystick.h"
#include "keyboard.h"
#include "pia1.h"
#include "cacttrns.h"
#include <QPainter>
#include <QPixmap>
#include <QByteArray>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QApplication>

class JoystickIO;


E2Screen::E2Screen(Scheduler &x_scheduler,
                   JoystickIO &x_joystickIO, KeyboardIO &x_keyboardIO,
                   Pia1 &x_pia1, sGuiOptions &x_guiOptions,
                   const QColor &x_backgroundColor,
                   QWidget *parent)
    : QWidget(parent)
    , scheduler(x_scheduler)
    , joystickIO(x_joystickIO)
    , keyboardIO(x_keyboardIO)
    , pia1(x_pia1)
    , backgroundColor(x_backgroundColor)
    , screen(QPixmap(WINDOWWIDTH, WINDOWHEIGHT))
    , transformationMode(Qt::FastTransformation)
    , firstRasterLine(0)
    , mouseX(-1)
    , mouseY(-1)
    , previousMouseX(-1)
    , previousMouseY(-1)
    , warpHomeX(0)
    , warpHomeY(0)
    , mouseButtonState(-1)
    , pixelSize(x_guiOptions.pixelSize)
    , cursorType(FLX_DEFAULT_CURSOR)
    , doScaledScreenUpdate(true)
    , preferredScreenSize(WINDOWWIDTH * x_guiOptions.pixelSize,
                          WINDOWHEIGHT * x_guiOptions.pixelSize)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setBaseSize({WINDOWWIDTH, WINDOWHEIGHT});
    setMinimumSize({WINDOWWIDTH, WINDOWHEIGHT});
    setMouseTracking(true);
    warpHomeX = (pixelSize * WINDOWWIDTH) >> 1;
    warpHomeY = (pixelSize * WINDOWHEIGHT) >> 1;
}

QSize E2Screen::GetScaledSize() const
{
    return scaledScreenSize;
}

void E2Screen::ResizeToFactor(int factor)
{
    if (factor >= 1)
    {
        preferredScreenSize =
            QSize(WINDOWWIDTH * factor, WINDOWHEIGHT * factor);
        updateGeometry();
    }
}

void E2Screen::mouseMoveEvent(QMouseEvent *event)
{
    assert(event != nullptr);

    SetMouseCoordinatesAndButtons(event);
}

void E2Screen::mousePressEvent(QMouseEvent *event)
{
    assert(event != nullptr);

    SetMouseCoordinatesAndButtons(event);
    event->accept();
}

void E2Screen::mouseReleaseEvent(QMouseEvent *event)
{
    assert(event != nullptr);

    SetMouseCoordinatesAndButtons(event);
    event->accept();
}

void E2Screen::leaveEvent(QEvent *)
{
    mouseX = previousMouseX = -1;
    mouseY = previousMouseY = -1;
    mouseButtonState = 0;
}

void E2Screen::enterEvent(QEvent *)
{
    mouseX = previousMouseX = -1;
    mouseY = previousMouseY = -1;
    mouseButtonState = 0;
}

void E2Screen::keyPressEvent(QKeyEvent *event)
{
    int key;

    assert(event != nullptr);

    if ((key = TranslateToAscii(event)) >= 0)
    {
        bool do_notify = false;

        keyboardIO.put_char_parallel((Byte)key, do_notify);
        if (do_notify)
        {
            auto command = BCommandPtr(
                new CActiveTransition(pia1, Mc6821::ControlLine::CA1));
            scheduler.sync_exec(std::move(command));
        }
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void E2Screen::resizeEvent(QResizeEvent *event)
{
    int width = event->size().width();
    int height = event->size().height();

    assert(event != nullptr);

    if (width / height >= 2)
    {
        width = height * 2;
        origin = QPoint((event->size().width() - width) / 2, 0);
    }
    else
    {
        if (width % 1)
        {
            width -= 1;
        }

        height = width / 2;
        origin = QPoint(0, (event->size().height() - height) / 2);
    }
    scaledScreenSize = QSize(width, height);
    doScaledScreenUpdate = true;

    preferredScreenSize = event->size();
    warpHomeX = preferredScreenSize.width() / 2;
    warpHomeY = preferredScreenSize.height() / 2;

    event->accept();
}

QSize E2Screen::minimumSizeHint() const
{
    return { WINDOWWIDTH, WINDOWHEIGHT };
}

QSize E2Screen::sizeHint() const
{
    return preferredScreenSize;
}

bool E2Screen::hasHeightForWidth() const
{
    return true;
}

int E2Screen::heightForWidth(int width) const
{
    return width / 2;
}

void E2Screen::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillRect(QRect(0, 0, width() - 1, height() - 1), backgroundColor);

    if (firstRasterLine != 0U)
    {
        auto scaledFirstRasterLine = 0U;

        if (doScaledScreenUpdate)
        {
            scaledScreen = screen.scaled(scaledScreenSize, Qt::KeepAspectRatio,
                           transformationMode);
            doScaledScreenUpdate = false;
        }
        scaledFirstRasterLine =
            firstRasterLine * scaledScreenSize.height() / WINDOWHEIGHT;

        // Paint first half display on the bottom of the screen
        auto origin1 =
            QPoint(origin.x(), origin.y() + scaledScreenSize.height() -
                   scaledFirstRasterLine);
        auto rect1 = QRect(0, 0, scaledScreenSize.width() - 1,
                           scaledFirstRasterLine);
        painter.drawPixmap(origin1, scaledScreen, rect1);
        // Paint second half display on the top of screen
        auto rect2 = QRect(0, scaledFirstRasterLine,
                           scaledScreenSize.width() - 1,
                           scaledScreenSize.height() - scaledFirstRasterLine);
        painter.drawPixmap(origin, scaledScreen, rect2);
    }
    else
    {
        if (doScaledScreenUpdate)
        {
            scaledScreen = screen.scaled(scaledScreenSize, Qt::KeepAspectRatio,
                           transformationMode);
            doScaledScreenUpdate = false;
        }

        painter.drawPixmap(origin, scaledScreen);
    }
}

void E2Screen::SetMouseCoordinatesAndButtons(QMouseEvent *event)
{
    mouseX = event->x();
    mouseY = event->y();
    mouseButtonState = ConvertMouseButtonState(event->buttons());
}

void E2Screen::ReleaseMouseCapture()
{
    if (cursorType == FLX_INVISIBLE_CURSOR)
    {
        ToggleMouseCapture();
    }
}

void E2Screen::ToggleMouseCapture()
{
    cursorType = (cursorType == FLX_DEFAULT_CURSOR) ?
                 FLX_INVISIBLE_CURSOR : FLX_DEFAULT_CURSOR;

    window()->setWindowTitle(GetTitle());

    if (cursorType == FLX_DEFAULT_CURSOR)
    {
        releaseMouse();
    }
    else
    {
        grabMouse();
    }

    SetCursorType(cursorType);
}

void E2Screen::SetCursorType(int type /* = FLX_DEFAULT_CURSOR */)
{
    if(type == FLX_DEFAULT_CURSOR)
    {
        unsetCursor();
    }
    else if(type == FLX_INVISIBLE_CURSOR)
    {
        setCursor(Qt::BlankCursor);
    }
}

void E2Screen::SetCursorPosition(int dx, int dy)
{
    if (warpDx || warpDy || dx || dy)
    {
        warpDx = dx;
        warpDy = dy;
        QPoint cursorPos(mouseX + dx, mouseY + dy);
        QCursor::setPos(mapToGlobal(cursorPos));
    }
}

void E2Screen::ToggleSmoothDisplay()
{
    transformationMode = (transformationMode == Qt::FastTransformation) ?
        Qt::SmoothTransformation : Qt::FastTransformation;
    update();
}

bool E2Screen::IsSmoothDisplay() const
{
    return transformationMode == Qt::SmoothTransformation;
}

QString E2Screen::GetTitle()
{
    if (cursorType == FLX_DEFAULT_CURSOR)
    {
        return QString(PROGRAMNAME " V" PROGRAM_VERSION " - ") +
               tr("Press CTRL F10 to capture mouse");
    }
    else
    {
        return QString(PROGRAMNAME " V" PROGRAM_VERSION " - ") +
               tr("Press CTRL F10 to release mouse");
    }
}

void E2Screen::UpdateBlock(Byte p_firstRasterLine, int displayBlock,
                           const QByteArray& bmpData)
{
    firstRasterLine = p_firstRasterLine;
    QPixmap pixmap;

    assert(displayBlock >= 0 && displayBlock < YBLOCKS);

    pixmap.loadFromData(bmpData, "BMP");;
    QPainter painter(&screen);
    painter.drawPixmap(QPoint(0, displayBlock * BLOCKHEIGHT), pixmap);
    doScaledScreenUpdate = true;
}

void E2Screen::RepaintScreen()
{
    repaint();
}

void E2Screen::UpdateMouse()
{
    int dx  = 0;
    int dy  = 0;

    if ((previousMouseX != -1) && (mouseX != -1) &&
        (previousMouseY != -1) && (mouseY != -1))
    {
        if (cursorType == FLX_INVISIBLE_CURSOR)
        {
            dx = mouseX - previousMouseX - warpDx;
            dy = mouseY - previousMouseY - warpDy;
            SetCursorPosition(warpHomeX - mouseX, warpHomeY - mouseY);
        }

        joystickIO.put_values(dx, dy);
        previousMouseX = mouseX;
        previousMouseY = mouseY;
    }
    else if ((mouseX != -1) && (mouseY != -1))
    {
        previousMouseX = mouseX;
        previousMouseY = mouseY;
    }
    else
    {
        previousMouseX = mouseX = -1;
        previousMouseY = mouseY = -1;
    }

    auto keyModifiers = GetKeyModifiersState();
    joystickIO.put_value(mouseButtonState | keyModifiers);
    keyboardIO.put_value(keyModifiers);
}

int E2Screen::GetKeyModifiersState()
{
    int state = 0;

    // Get modifier state of shift and control key.
    if (QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
    {
        state |= SHIFT_KEY;
    }
    if (QApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier))
    {
        state |= CONTROL_KEY;
    }

    return state;
}

int E2Screen::ConvertMouseButtonState(Qt::MouseButtons mouseButtonState)
{
    int state = 0;

    if (mouseButtonState & Qt::LeftButton)
    {
        state |= L_MB;
    }
    if (mouseButtonState & Qt::MiddleButton)
    {
        state |= M_MB;
    }
    if (mouseButtonState & Qt::RightButton)
    {
        state |= R_MB;
    }

    return state;
}

int E2Screen::TranslateToAscii(QKeyEvent *event)
{
    static const auto modifiers =
        Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier;

    switch (event->modifiers() & modifiers)
    {
        case Qt::AltModifier:
            return -1;

        case Qt::ControlModifier | Qt::ShiftModifier:
            switch (event->key())
            {
                case Qt::Key_0:
                    return 0x30;

                case Qt::Key_1:
                    return 0x31;

                case Qt::Key_2:
                    return 0x32;

                case Qt::Key_3:
                    return 0x33;

                case Qt::Key_4:
                    return 0x34;

                case Qt::Key_5:
                    return 0x35;

                case Qt::Key_6:
                    return 0xa1;

                case Qt::Key_7:
                    return 0x37;

                case Qt::Key_8:
                    return 0x38;

                case Qt::Key_9:
                    return 0x39;

                case Qt::Key_Home:
                    return 0xa1;

                case Qt::Key_Up:
                    return 0xa8;

                case Qt::Key_PageUp:
                    return 0xa7;

                case Qt::Key_Left:
                    return 0xa4;

                case Qt::Key_Clear:
                    return 0xa5;

                case Qt::Key_Right:
                    return 0xa6;

                case Qt::Key_End:
                    return 0xa9;

                case Qt::Key_Down:
                    return 0xa2;

                case Qt::Key_PageDown:
                    return 0xa3;

                case Qt::Key_Backtab:
                    return 0x09;
            }
            break;

        case Qt::ControlModifier:
            switch (event->key())
            {
                case Qt::Key_0:
                    return 0x30;

                case Qt::Key_1:
                    return 0x31;

                case Qt::Key_2:
                    return 0x32;

                case Qt::Key_3:
                    return 0x33;

                case Qt::Key_4:
                    return 0x34;

                case Qt::Key_5:
                    return 0x35;

                case Qt::Key_6:
                    return 0x36;

                case Qt::Key_7:
                    return 0x37;

                case Qt::Key_8:
                    return 0x38;

                case Qt::Key_9:
                    return 0x39;

                case Qt::Key_Home:
                    return 0xb1;

                case Qt::Key_Up:
                    return 0xb8;

                case Qt::Key_PageUp:
                    return 0xb7;

                case Qt::Key_Left:
                    return 0xb4;

                case Qt::Key_Clear:
                    return 0xb5;

                case Qt::Key_Right:
                    return 0xb6;

                case Qt::Key_End:
                    return 0xb9;

                case Qt::Key_Down:
                    return 0xb2;

                case Qt::Key_PageDown:
                    return 0xb3;

                case Qt::Key_F10:
                    ToggleMouseCapture();
                    return -1;

                case Qt::Key_F11:
                    return 0xfb; // PAT09: RIGHT MOST

                case Qt::Key_F12:
                    return 0x92; // PAT09: LEFT  MOST

                case Qt::Key_Delete:
                    return 0x1f;

                case Qt::Key_Tab:
                    return 0x09;
            }
            break;

        case Qt::ShiftModifier:                                                 
            switch (event->key())
            {
                case Qt::Key_F1:
                    return 0xca; // PAT09: F11

                case Qt::Key_F2:
                    return 0xcb;

                case Qt::Key_F3:
                    return 0xcc;

                case Qt::Key_F4:
                    return 0xcd;

                case Qt::Key_F5:
                    return 0xce;

                case Qt::Key_F6:
                    return 0xcf;

                case Qt::Key_F7:
                    return 0xd0;

                case Qt::Key_F8:
                    return 0xd1;

                case Qt::Key_F9:
                    return 0xd2; // PAT09: F19

                case Qt::Key_F11:
                    return 0xea; // PAT09: RIGHT MOST

                case Qt::Key_F12:
                    return 0x81; // PAT09: LEFT  MOST

                case Qt::Key_Home:
                    return 0xe1;

                case Qt::Key_Up:
                    return 0xe8;

                case Qt::Key_PageUp:
                    return 0xe7;

                case Qt::Key_Left:
                    return 0xe4;

                case Qt::Key_Clear:
                    return 0xe5;

                case Qt::Key_Right:
                    return 0xe6;

                case Qt::Key_End:
                    return 0xe9;

                case Qt::Key_Down:
                    return 0xe2;

                case Qt::Key_PageDown:
                    return 0xe3;

                case Qt::Key_Delete:
                    return 0x7f;

                case Qt::Key_Backtab:
                    return 0x09;
            }
            break;

       case 0:
            switch (event->key())
            {
                case Qt::Key_F1:
                    return 0xc0;

                case Qt::Key_F2:
                    return 0xc1;

                case Qt::Key_F3:
                    return 0xc2;

                case Qt::Key_F4:
                    return 0xc3;

                case Qt::Key_F5:
                    return 0xc4;

                case Qt::Key_F6:
                    return 0xc5;

                case Qt::Key_F7:
                    return 0xc6;

                case Qt::Key_F8:
                    return 0xc7;

                case Qt::Key_F9:
                    return 0xc8;

                case Qt::Key_F10:
                    return 0xc9;

                case Qt::Key_F11:
                    return 0xfa; // PAT09: RIGHT MOST

                case Qt::Key_F12:
                    return 0x91; // PAT09: LEFT  MOST

                case Qt::Key_Home:
                    return 0xf1;

                case Qt::Key_Up:
                    return 0xf8;

                case Qt::Key_PageUp:
                    return 0xf7;

                case Qt::Key_Left:
                    return 0xf4;

                case Qt::Key_Clear:
                    return 0xf5;

                case Qt::Key_Right:
                    return 0xf6;

                case Qt::Key_End:
                    return 0xf9;

                case Qt::Key_Down:
                    return 0xf2;

                case Qt::Key_PageDown:
                    return 0xf3;

                case Qt::Key_Delete:
                    return 0x7f;

                case Qt::Key_Tab:
                    return 0x09;
            }
            break;
    }

    if (event->text().size() == 1 && !(event->text()[0].unicode() & 0xFF80))
    {
        return event->text()[0].unicode();
    }

    return -1;
}

