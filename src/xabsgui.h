/*
    xabsgui.h: abstract user interface for X11


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2020  W. Schwotzer

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



#ifndef XABSGUI_INCLUDED
#define XABSGUI_INCLUDED

#include <misc1.h>

#ifdef HAVE_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xcms.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>

extern "C" {
    extern Status _XInitImageFuncPtrs(
#if NeedFunctionPrototypes
        XImage* /* image */
#endif
    );
}

#include "absgui.h"

enum
{
    ALL_COLORS   = 0,
    GRAY_SCALE   = 1,
    RED_COLORS   = 2,
    GREEN_COLORS = 3,
    BLUE_COLORS  = 4
};


class XAbstractGui : public AbstractGui
{
protected:
    enum
    {
        FLX_INVISIBLE_CURSOR = 10,
        FLX_DEFAULT_CURSOR   = 11
    };
    enum tWindowType
    {
        FLX_MAIN    = 34,
        FLX_E2SCREEN    = 35
    };
    Window       e2window;
    GC       e2gc;
    XImage      *image[MAX_PIXELSIZEX][MAX_PIXELSIZEY];
    int     oldX, oldY; // old pointer position
    Visual      *visual;
    Colormap    cmap;
    int     own_cmap;
    int     depth;
    Cursor      cursor;
    int     cursor_type;

protected:

    void update_block(int block_number) override;
    void initialize(struct sGuiOptions &options) override;

    virtual void create_message_dialog(Widget parent);
    virtual Display *getDisplay();
    virtual Window getWindow(tWindowType t = FLX_E2SCREEN);

    int SetColors(Display *dpy);
    Visual *GetBestVisual(Display *dpy, int *vClass, int *depth);
    void set_cursor(int type = FLX_DEFAULT_CURSOR);
    int convert_buttonmask(int);
    int convert_keymask(int);
    SWord translate_to_ascii(XKeyEvent *pevent);
    int popup_help();
    const char *get_title();
    void toggle_mouse_capture();
    void release_mouse_capture();

public:

    void set_bell(int percent) override;
    virtual void c_focusIn(XEvent *pevent);
    virtual void c_focusOut(XEvent *pevent);

public:

    XAbstractGui(
            Mc6809 &,
            Memory &,
            Scheduler &,
            Inout &,
            VideoControl1 &,
            VideoControl2 &,
            JoystickIO &,
            KeyboardIO &,
            TerminalIO &,
            struct sGuiOptions &);
    virtual ~XAbstractGui();
};

#endif // HAVE_X11
#endif // XABSGUI_INCLUDED

