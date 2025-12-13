/*
    e2screen.h  A QWidget subclass implementing the Eurocom II screen.


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2025  W. Schwotzer

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

#ifndef E2SCREEN_INCLUDED
#define E2SCREEN_INCLUDED

#include "typedefs.h"
#include "bobservd.h"
#include "warnoff.h"
#include <QVector>
#include <QSize>
#include <QRgb>
#include <QWidget>
#include <QPixmap>
#include "warnon.h"
#include "blinxsys.h" // After qt include to avoid automoc issue
#include "soptions.h" // After qt include to avoid automoc issue

class VideoControl2;
class QPaintEvent;
class QByteArray;
class QEvent;
class QResizeEvent;
class QMouseEvent;
class QPaintEvent;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
class QEnterEvent;
#endif
class Scheduler;
class JoystickIO;
class KeyboardIO;
class Pia1;


class E2Screen : public QWidget, public BObserved
{
    Q_OBJECT

public:
    enum class CursorType : uint8_t
    {
        Invisible,
        Default,
    };

    E2Screen() = delete;
    E2Screen(Scheduler &p_scheduler, JoystickIO &p_joystickIO,
             KeyboardIO &p_keyboardIO, Pia1 &p_pia1,
             sOptions &p_options,
             const QColor &p_backgroundColor, QWidget *parent = nullptr);
    ~E2Screen() override = default;

    QSize GetScaledSize() const;
    void UpdateBlock(Byte firstRasterLine, int displayBlock,
                     const QByteArray& data);
    void RepaintScreen();
    void UpdateMouse();
    int GetPixelSizeX() const;
    int GetPixelSizeY() const;
    void ToggleSmoothDisplay();
    void ToggleMouseCapture();
    void ReleaseMouseCapture();
    QString GetTitle();
    void ResizeToFactor(int factor);
    bool IsSmoothDisplay() const;
    void SetBackgroundColor(const QColor &color);

    // QWidget Overrides
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    static uint32_t ConvertMouseButtonState(Qt::MouseButtons mouseButtons);
    static uint32_t GetKeyModifiersState();
    int TranslateToPAT09Key(QKeyEvent *event);
    void MouseWarp(int dx, int dy);
    void SetMouseCoordinatesAndButtons(QMouseEvent *event);
    void SetCursorType(CursorType p_cursorType);
    void SetCursorPosition(int x, int y);
    void InitializeNumLockIndicatorMask();
    bool IsNumLockOn() const;

    Scheduler &scheduler;
    JoystickIO &joystickIO;
    KeyboardIO &keyboardIO;
    Pia1 &pia1;
    QColor backgroundColor;
    QPixmap screen;
    QPixmap scaledScreen;
    Qt::TransformationMode transformationMode;
    Byte firstRasterLine;
    int mouseX;
    int mouseY;
    int previousMouseX;
    int previousMouseY;
    int warpDx{};
    int warpDy{};
    int warpHomeX;
    int warpHomeY;
    unsigned mouseButtonState;
    int pixelSize;
    CursorType cursorType;
    bool doScaledScreenUpdate;
    QSize preferredScreenSize;
    QSize scaledScreenSize;
    QPoint origin;
    unsigned int numLockIndicatorMask;
#ifdef __linux__
    BLinuxSysInfo sysInfo;
#endif
};
#endif

