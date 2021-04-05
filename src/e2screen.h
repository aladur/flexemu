/*
    e2screen.h  A QWidget subclass implementing the Eurocom II screen. 


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2021  W. Schwotzer

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

#include "soptions.h"
#include <misc1.h>
#include <memory>
#include <QVector>
#include <QSize>
#include <QRgb>
#include <QWidget>
#include <QPixmap>

class VideoControl2;
class QPaintEvent;
class QByteArray;
class QEvent;
class QResizeEvent;
class QMouseEvent;
class QPaintEvent;
class Scheduler;
class JoystickIO;
class KeyboardIO;
class Pia1;


class E2Screen : public QWidget
{
    Q_OBJECT

public:
    enum
    {
        FLX_INVISIBLE_CURSOR = 10,
        FLX_DEFAULT_CURSOR   = 11
    };

    E2Screen() = delete;
    E2Screen(Scheduler &x_scheduler, JoystickIO &x_joystickIO,
             KeyboardIO &x_keyboardIO, Pia1 &x_pia1,
             sOptions &x_options,
             const QColor &backgroundColor, QWidget *parent = nullptr);
    virtual ~E2Screen() = default;

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
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    static int ConvertMouseButtonState(Qt::MouseButtons mouseButtons);
    static int GetKeyModifiersState();
    int TranslateToAscii(QKeyEvent *event);
    void MouseWarp(int dx, int dy);
    void SetMouseCoordinatesAndButtons(QMouseEvent *event);
    void SetCursorType(int type = FLX_DEFAULT_CURSOR);
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
    int warpDx;
    int warpDy;
    int warpHomeX;
    int warpHomeY;
    int mouseButtonState;
    int pixelSize;
    int cursorType;
    bool doScaledScreenUpdate;
    QSize preferredScreenSize;
    QSize scaledScreenSize;
    QPoint origin;
    unsigned int numLockIndicatorMask;
};
#endif

