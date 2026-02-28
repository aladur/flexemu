/*
    e2screen.cpp  A QWidget subclass implementing the Eurocom II screen.


    flexemu, an MC6809 emulator running FLEX
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


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "config.h"
#include "typedefs.h"
#include "e2screen.h"
#include "bcommand.h"
#include "bobshelp.h"
#include "e2.h"
#include "schedule.h"
#include "joystick.h"
#include "keyboard.h"
#include "pia1.h"
#include "cacttrns.h"
#include "soptions.h"
#include "warnoff.h"
#include <QtGlobal>
#include <QPainter>
#include <QColor>
#include <QPixmap>
#include <QCursor>
#include <QByteArray>
#include <QEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QEnterEvent>
#endif
#include <QApplication>
#include <QGuiApplication>
#include <QWidget>
#include "warnon.h"
#include <cassert>
#include <cstdint>
#include <array>
#include <string>
#if defined(UNIX) && !defined(X_DISPLAY_MISSING)
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)) && !defined(__APPLE__)
#include <QX11Info>
#endif
    // Qt < 6.0.0
#include <X11/XKBlib.h>
#endif // #if defined(UNIX) && !defined(X_DISPLAY_MISSING)


E2Screen::E2Screen(Scheduler &p_scheduler,
                   JoystickIO &p_joystickIO, KeyboardIO &p_keyboardIO,
                   Pia1 &p_pia1, sOptions &p_options,
                   // There are differences between Qt5 and Qt6.
                   // NOLINTNEXTLINE(modernize-pass-by-value)
                   const QColor &p_backgroundColor,
                   QWidget *parent)
    : QWidget(parent)
    , scheduler(p_scheduler)
    , joystickIO(p_joystickIO)
    , keyboardIO(p_keyboardIO)
    , pia1(p_pia1)
    , backgroundColor(p_backgroundColor)
    , screen(QPixmap(WINDOWWIDTH, WINDOWHEIGHT))
    , transformationMode(Qt::FastTransformation)
    , firstRasterLine(0)
    , mouseX(-1)
    , mouseY(-1)
    , previousMouseX(-1)
    , previousMouseY(-1)
    , warpHomeX(0)
    , warpHomeY(0)
    , mouseButtonState(0)
    , pixelSize(p_options.pixelSize)
    , cursorType(CursorType::Default)
    , doScaledScreenUpdate(true)
    , preferredScreenSize(WINDOWWIDTH * p_options.pixelSize,
                          WINDOWHEIGHT * p_options.pixelSize)
    , numLockIndicatorMask(0U)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setBaseSize({WINDOWWIDTH, WINDOWHEIGHT});
    setMinimumSize({WINDOWWIDTH, WINDOWHEIGHT});
    setMouseTracking(true);
    warpHomeX = static_cast<int>((pixelSize * WINDOWWIDTH) / 2U);
    warpHomeY = static_cast<int>((pixelSize * WINDOWHEIGHT) / 2U);
    InitializeNumLockIndicatorMask();
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

    if (!mouseNotificationRect.isNull())
    {
        const bool currentIsInRect =
            mouseNotificationRect.contains(event->x(), event->y());
        // Only notify on value changed.
        if (!isInRect.has_value() || currentIsInRect != isInRect.value())
        {
            isInRect = currentIsInRect;
            emit NotifyMouseInRect(currentIsInRect);
        }
    }
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

void E2Screen::leaveEvent(QEvent * /* event */)
{
    mouseX = previousMouseX = -1;
    mouseY = previousMouseY = -1;
    mouseButtonState = 0;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void E2Screen::enterEvent(QEnterEvent * /* event */)
#else
void E2Screen::enterEvent(QEvent * /*event*/)
#endif
{
    mouseX = previousMouseX = -1;
    mouseY = previousMouseY = -1;
    mouseButtonState = 0;
}

void E2Screen::keyPressEvent(QKeyEvent *event)
{
    assert(event != nullptr);

    const auto key = TranslateToPAT09Key(event);
    if (key >= 0)
    {
        bool do_notify = false;
        auto bKey = static_cast<Byte>(key);

        keyboardIO.put_char_parallel(bKey, do_notify);
        if (do_notify)
        {
            auto command = BCommandSPtr(
                new CActiveTransition(pia1, Mc6821::ControlLine::CA1));
            scheduler.sync_exec(std::move(command));
        }
        Notify(NotifyId::KeyPressed, &bKey);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void E2Screen::resizeEvent(QResizeEvent *event)
{
    assert(event != nullptr);

    int width = event->size().width();
    int height = event->size().height();

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

void E2Screen::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillRect(QRect(0, 0, width() - 1, height() - 1), backgroundColor);

    if (firstRasterLine != 0U)
    {
        auto scaledFirstRasterLine = 0;

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
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    auto pos = event->position();
    mouseX = static_cast<int>(pos.x());
    mouseY = static_cast<int>(pos.y());
#else
    mouseX = event->x();
    mouseY = event->y();
#endif
    mouseButtonState = ConvertMouseButtonState(event->buttons());
}

void E2Screen::ReleaseMouseCapture()
{
    if (cursorType == CursorType::Invisible)
    {
        ToggleMouseCapture();
    }
}

void E2Screen::ToggleMouseCapture()
{
    cursorType = (cursorType == CursorType::Default) ?
                 CursorType::Invisible : CursorType::Default;

    window()->setWindowTitle(GetTitle());

    if (cursorType == CursorType::Default)
    {
        releaseMouse();
    }
    else
    {
        grabMouse();
    }

    SetCursorType(cursorType);
}

void E2Screen::SetCursorType(CursorType p_cursorType)
{
    switch (p_cursorType)
    {
        case CursorType::Default:
            unsetCursor();
            break;

        case CursorType::Invisible:
            setCursor(Qt::BlankCursor);
            break;
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

void E2Screen::SetBackgroundColor(const QColor &color)
{
    backgroundColor = color;
}

void E2Screen::ToggleSmoothDisplay()
{
    transformationMode = (transformationMode == Qt::FastTransformation) ?
        Qt::SmoothTransformation : Qt::FastTransformation;
    doScaledScreenUpdate = true;
}

bool E2Screen::IsSmoothDisplay() const
{
    return transformationMode == Qt::SmoothTransformation;
}

QString E2Screen::GetTitle()
{
    if (cursorType == CursorType::Default)
    {
        return QString(PACKAGE_NAME " V" VERSION " - ") +
               tr("Press CTRL F10 to capture mouse");
    }

    return QString(PACKAGE_NAME " V" VERSION " - ") +
           tr("Press CTRL F10 to release mouse");
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
    int dx = 0;
    int dy = 0;

    if ((previousMouseX != -1) && (mouseX != -1) &&
        (previousMouseY != -1) && (mouseY != -1))
    {
        if (cursorType == CursorType::Invisible)
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

uint32_t E2Screen::GetKeyModifiersState()
{
    uint32_t state = 0;

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

uint32_t E2Screen::ConvertMouseButtonState(Qt::MouseButtons mouseButtons)
{
    uint32_t state = 0;

    if (mouseButtons & Qt::LeftButton)
    {
        state |= L_MB;
    }
    if (mouseButtons & Qt::MiddleButton)
    {
        state |= M_MB;
    }
    if (mouseButtons & Qt::RightButton)
    {
        state |= R_MB;
    }

    return state;
}

bool E2Screen::IsNumLockOn() const
{
    // Get state of Num Lock indicator is not available in Qt.
    // Use Win32/X11 low level access.
#ifdef _WIN32
    return (0x0001U & GetKeyState(VK_NUMLOCK)) != 0U;
#else
#if defined(UNIX) && !defined(X_DISPLAY_MISSING)
    Display *display = nullptr;

    if (qApp->platformName() == "xcb")
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        auto *x11App =
            qApp->nativeInterface<QNativeInterface::QX11Application>();
        if (x11App != nullptr)
        {
            display = x11App->display();
        }
#else
    // 6.0.0 <= Qt < 6.2.0
#error Only Qt6 versions 6.2.0 or higher are supported
#endif // Qt >= 6.2.0
#else
#if !defined(__APPLE__)
        // Qt < 6.0.0
        display = QX11Info::display();
#endif
#endif // Qt >= 6.0.0
    }

    if (display != nullptr)
    {
        unsigned int state;

        if (Success != XkbGetIndicatorState(display, XkbUseCoreKbd, &state))
        {
            return false;
        }

        return (state & numLockIndicatorMask) != 0;
    }

#endif // #if defined(UNIX) && !defined(X_DISPLAY_MISSING)
#ifdef __linux__
    // If Linux kernel present read Num Lock LED status from /sys filesystem
    auto status = sysInfo.Read(BLinuxSysInfoType::LED, "numlock");
    return std::stoi(status) != 0;
#endif // #ifdef __linux__
    return false;
#endif // #ifdef __WIN32
}

int E2Screen::TranslateToPAT09Key(QKeyEvent *event)
{
    constexpr const auto modifiers =
        Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier;
    constexpr const int NR = 11; // Number of rows in following table.
    constexpr const int NC = 4; // Number of columns in following table.
    // On Eurocom II PAT09 keyboard the following keys are mapped:
    //
    // keyboard input           | Eurocom II mapped key
    // --------------------------------------------------
    // Key_Left                 | Left Arrow
    // Key_Right                | Right Arrow
    // Key_Up                   | Up Arrow
    // Key_Down                 | Down Arrow
    // Key_Home                 | Thick Left Arrow
    // Key_End                  | Thick Right Arrow
    // Key_PageUp               | Thick Upper Left Arrow
    // Key_PageDown             | Thick Lower Right Arrow
    // Key_Clear                | Mode
    // Key_Insert (Keypad only) | Left limit
    // Key_Delete (Keypad only) | Right limit
    // Key_F1                   | F1
    // Key_F2                   | F2
    // Key_F3                   | F3
    // Key_F4                   | F4
    // Key_F5                   | F5
    // Key_F6                   | F6
    // Key_F7                   | F7
    // Key_F8                   | F8
    // Key_F9                   | F9
    // Key_F10                  | F10
    // Shift+Key_F1             | F11
    // Shift+Key_F2             | F12
    // Shift+Key_F3             | F13
    // Shift+Key_F4             | F14
    // Shift+Key_F5             | F15
    // Shift+Key_F6             | F16
    // Shift+Key_F7             | F17
    // Shift+Key_F8             | F18
    // Shift+Key_F9             | F19
    //
    // Coding of keypad keys if Num Lock is off:
    //
    //     +--------------------- key only
    //     |     +--------------- Shift+key
    //     |     |     +--------- Ctrl+key
    //     |     |     |     +--- Shift+Ctrl+key
    //     |     |     |     |
    constexpr const std::array <int, NR * NC> cursorCtrlCode = {
        0xFA, 0xEA, 0xFA, 0xEA, // Key_Insert
        0xF9, 0xE9, 0xB9, 0xA9, // Key_End
        0xF2, 0xE2, 0xB2, 0xA2, // Key_Down
        0xF3, 0xE3, 0xB3, 0xA3, // Key_PageDown
        0xF4, 0xE4, 0xB4, 0xA4, // Key_Left
        0xF5, 0xE5, 0xB5, 0xA5, // Key_Clear
        0xF6, 0xE6, 0xB6, 0xA6, // Key_Right
        0xF1, 0xE1, 0xB1, 0xA1, // Key_Home
        0xF8, 0xE8, 0xB8, 0xA8, // Key_Up
        0xF7, 0xE7, 0xB7, 0xA7, // Key_PageUp
        0x91, 0x81, 0x91, 0x81, // Key_Delete
    };
    // Define key_code for Delete
    constexpr const std::array<int, 4> deleteKeyCode{
        0x7F, 0x7F, 0x1F, 0x7F
    };
    Word index = event->modifiers() & Qt::ShiftModifier ? 1U : 0U;
    index |= event->modifiers() & Qt::ControlModifier ? 2U : 0U;
    assert(index < NC);

    // Process keys which behave the same for any modifier set.
    switch (event->key())
    {
        case Qt::Key_Backspace:
            return 0x08;
    }

    // Process Keypad keys. It depends on the Num Lock indicator.
    if (event->modifiers() & Qt::KeypadModifier)
    {
        switch (event->key())
        {
            case Qt::Key_0:
            case Qt::Key_Insert:
                return IsNumLockOn() ? 0x30 : cursorCtrlCode[0 * NC + index];

            case Qt::Key_1:
            case Qt::Key_End:
                return IsNumLockOn() ? 0x31 : cursorCtrlCode[1 * NC + index];

            case Qt::Key_2:
            case Qt::Key_Down:
                return IsNumLockOn() ? 0x32 : cursorCtrlCode[2 * NC + index];

            case Qt::Key_3:
            case Qt::Key_PageDown:
                return IsNumLockOn() ? 0x33 : cursorCtrlCode[3 * NC + index];

            case Qt::Key_4:
            case Qt::Key_Left:
                return IsNumLockOn() ? 0x34 : cursorCtrlCode[4 * NC + index];

            case Qt::Key_5:
            case Qt::Key_Clear:
                return IsNumLockOn() ? 0x35 : cursorCtrlCode[5 * NC + index];

            case Qt::Key_6:
            case Qt::Key_Right:
                return IsNumLockOn() ? 0x36 : cursorCtrlCode[6 * NC + index];

            case Qt::Key_7:
            case Qt::Key_Home:
                return IsNumLockOn() ? 0x37 : cursorCtrlCode[7 * NC + index];

            case Qt::Key_8:
            case Qt::Key_Up:
                return IsNumLockOn() ? 0x38 : cursorCtrlCode[8 * NC + index];

            case Qt::Key_9:
            case Qt::Key_PageUp:
                return IsNumLockOn() ? 0x39 : cursorCtrlCode[9 * NC + index];

            case Qt::Key_Comma:
            case Qt::Key_Period:
            case Qt::Key_Delete:
                return
                    IsNumLockOn() ? 0x2E : cursorCtrlCode[10 * NC + index];
        }
    }

    // Process keys without Ctrl modifier set.
    if ((event->modifiers() & Qt::ControlModifier) == 0)
    {
        switch (event->key())
        {
            case Qt::Key_acute:
            case Qt::Key_Dead_Acute:
                return 0x27;

            case Qt::Key_AsciiCircum:
            case Qt::Key_Dead_Circumflex:
                return 0x5E;

            case Qt::Key_Dead_Grave:
                return 0x60;

        }
    }

    // Process keys with Ctrl modifier set.
    if ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
    {
        switch (event->key())
        {
            case Qt::Key_0:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
                return event->key();

            case Qt::Key_QuoteLeft:
            case Qt::Key_At:
            case Qt::Key_Dead_Grave:
                return 0x00;

            case Qt::Key_BracketLeft:
            case Qt::Key_BraceLeft:
                return 0x1B;

            case Qt::Key_Backslash:
            case Qt::Key_Bar:
                return 0x1C;

            case Qt::Key_BracketRight:
            case Qt::Key_BraceRight:
                return 0x1D;

            case Qt::Key_AsciiCircum:
            case Qt::Key_Dead_Circumflex:
            case Qt::Key_AsciiTilde:
            case Qt::Key_Dead_Tilde:
                return 0x1E;

            case Qt::Key_Underscore:
                return 0x1F;
        }
    }

    // Process keys (not on Keypad)
    switch (event->key())
    {
        case Qt::Key_Insert:
        case Qt::Key_degree:
            return -1;

        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            return 0x09;

        case Qt::Key_End:
            return cursorCtrlCode[1 * NC + index];

        case Qt::Key_Down:
            return cursorCtrlCode[2 * NC + index];

        case Qt::Key_PageDown:
            return cursorCtrlCode[3 * NC + index];

        case Qt::Key_Left:
            return cursorCtrlCode[4 * NC + index];

        case Qt::Key_Clear:
            return cursorCtrlCode[5 * NC + index];

        case Qt::Key_Right:
            return cursorCtrlCode[6 * NC + index];

        case Qt::Key_Home:
            return cursorCtrlCode[7 * NC + index];

        case Qt::Key_Up:
            return cursorCtrlCode[8 * NC + index];

        case Qt::Key_PageUp:
            return cursorCtrlCode[9 * NC + index];

        case Qt::Key_Delete:
            return deleteKeyCode[index];
    }

    switch (event->modifiers() & modifiers)
    {
        case Qt::AltModifier:
            return -1;

        case Qt::ControlModifier:
            switch (event->key())
            {
                case Qt::Key_F10:
                    ToggleMouseCapture();
                    return -1;
            }
            break;

        case Qt::ShiftModifier:
            switch (event->key())
            {
                case Qt::Key_F1:
                    return 0xca; // PAT09: F11

                case Qt::Key_F2:
                    return 0xcb; // PAT09: F12

                case Qt::Key_F3:
                    return 0xcc; // PAT09: F13

                case Qt::Key_F4:
                    return 0xcd; // PAT09: F14

                case Qt::Key_F5:
                    return 0xce; // PAT09: F15

                case Qt::Key_F6:
                    return 0xcf; // PAT09: F16

                case Qt::Key_F7:
                    return 0xd0; // PAT09: F17

                case Qt::Key_F8:
                    return 0xd1; // PAT09: F18

                case Qt::Key_F9:
                    return 0xd2; // PAT09: F19
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
            }
            break;
    }

    if (event->text().size() == 1 && !(event->text()[0].unicode() & 0xFF80U))
    {
        return event->text()[0].unicode();
    }

    return -1;
}

void E2Screen::InitializeNumLockIndicatorMask()
{
#if defined(UNIX) && !defined(X_DISPLAY_MISSING)
    Display *display = nullptr;

    if (qApp->platformName() == "xcb")
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
        auto *x11App =
            qApp->nativeInterface<QNativeInterface::QX11Application>();
        if (x11App != nullptr)
        {
            display = x11App->display();
        }
#else
    // 6.0.0 <= Qt < 6.2.0
#error Only Qt6 versions 6.2.0 or higher are supported
#endif // Qt >= 6.2.0
#else
#if !defined(__APPLE__)
        // Qt < 6.0.0
        display = QX11Info::display();
#endif
#endif // Qt >= 6.0.0
    }

    if (display != nullptr)
    {
        XkbDescRec* kbDesc = XkbAllocKeyboard();
        uint32_t index;

        if (display == nullptr || kbDesc == nullptr ||
            (Success != XkbGetNames(display, XkbIndicatorNamesMask, kbDesc)))
        {
            return;
        }

        for (index = 0; index < XkbNumIndicators; ++index)
        {
            static const std::string strNumLock{"Num Lock"};

            if (!kbDesc->names->indicators[index])
            {
                continue;
            }

            char *atomName =
                XGetAtomName(display, kbDesc->names->indicators[index]);

            if (0 == strNumLock.compare(atomName))
            {
                numLockIndicatorMask = 1U << index;
                XFree(atomName);
                break;
            }
            XFree(atomName);
        }

        XkbFreeKeyboard(kbDesc, 0, True);
    }
#endif // #if defined(UNIX) && !defined(X_DISPLAY_MISSING)
}

void E2Screen::SetMouseNotificationRect(const QRect &rect)
{
    mouseNotificationRect = rect;

    if (rect.isNull())
    {
        isInRect.reset();
    }
}
