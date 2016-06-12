/*
    xabsgui.h: abstract user interface for X11


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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



#ifndef __xabsgui_h__
#define __xabsgui_h__

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
};

#include "absgui.h"

enum {
        ALL_COLORS   = 0,
	GRAY_SCALE   = 1,
	RED_COLORS   = 2,
	GREEN_COLORS = 3,
	BLUE_COLORS  = 4
};


class XAbstractGui : public AbstractGui {
protected:
enum {
	FLX_INVISIBLE_CURSOR = 10,
	FLX_DEFAULT_CURSOR   = 11
};
enum tWindowType {
	FLX_MAIN 	= 34,
	FLX_E2SCREEN 	= 35
};
	Window		 e2window;
	GC		 e2gc;
	// images for display with one plane (depth = 1)
	XImage		*image1[MAX_GUIXSIZE][MAX_GUIYSIZE];
	// images for display with six planes (depth >= 8)
	XImage		*image6[MAX_GUIXSIZE][MAX_GUIYSIZE];
	int		oldX, oldY;	// old pointer position
	Byte		*copy_block;
	Visual		*visual;
	Colormap	cmap;
	int		own_cmap;
	int		depth;
	Cursor		cursor;
	int		cursor_type;

protected:

	virtual int 	SetColors(Display *dpy);
	virtual Visual *GetBestVisual(Display *dpy, int *vClass, int *depth);
	virtual void	create_message_dialog(Widget parent);
	virtual void	set_cursor(int type = FLX_DEFAULT_CURSOR);
	virtual void 	update_block(int block_number);
	virtual void 	update_color_block(int block_number);
	virtual void 	update_bw_block(int block_number);
	virtual int 	convert_buttonmask(int);
	virtual SWord	translate_to_ascii(XKeyEvent *pevent);
	virtual Display *getDisplay(void);
	virtual Window	getWindow(tWindowType t = FLX_E2SCREEN);
	virtual void	initialize(struct sGuiOptions *pOptions);
	virtual int	popup_help(void);
	virtual char    *get_title(void);
	virtual void	toggle_mouse_capture(void);
	virtual void	release_mouse_capture(void);

public:

	virtual void	set_bell(int percent);
	virtual void    c_focusIn(XEvent *pevent);
	virtual void    c_focusOut(XEvent *pevent);
public:

	XAbstractGui(Mc6809*, Memory*, Scheduler*, Inout*, E2video*,
		struct sGuiOptions *);
	virtual ~XAbstractGui();
};

#endif // HAVE_X11
#endif // __xabsgui_h__

