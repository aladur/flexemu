/*
    xres.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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


#include <misc1.h>

#ifdef HAVE_XTK

#include <X11/Intrinsic.h>

extern String fallback_resources[];

String fallback_resources[] =
{
    const_cast<String>("*font:			*-fixed-*-*-*-*-10-*"),
    const_cast<String>("*screen.accelerators:	"
    "#replace <Expose>:popup_no_resources()"),
    const_cast<String>("*Command*width:		50"),
    const_cast<String>("*message.title:		Error"),
    const_cast<String>("*messageButton.fromVert:	messageText"),
    const_cast<String>("*messageButton.label:		Ok"),
    const_cast<String>("*messageButton.horizDistance:	75"),
    const_cast<String>("*messageText.width:		200"),
    const_cast<String>("*messageText.height:		60"),
    const_cast<String>("*messageText.displayCaret:	false"),
    const_cast<String>("*messageText*string:		\
You have no application defaults\\n\
file installed. Copy the file\\n\
Flexemu.ad to $HOME/Flexemu and\\n\
set the environment variable\\n\
XUSERFILESEARCHPATH to $HOME/%N"),
    const_cast<String>("*authorWidget.width:		200"),
    const_cast<String>("*authorWidget.height:		60"),
    nullptr
};
#endif // HAVE_XTK

