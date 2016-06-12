/*
    xres.cpp


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


#include <misc1.h>

#ifdef HAVE_XTK

#include <X11/Intrinsic.h>

extern String fallback_resources[];

String fallback_resources[] = {
"*font:				*-fixed-*-*-*-*-10-*",
"*screen.accelerators:		#replace <Expose>:popup_no_resources()",
"*Command*width:		50",
"*message.title:		Error",
"*messageButton.fromVert:	messageText",
"*messageButton.label:		Ok",
"*messageButton.horizDistance:	75",
"*messageText.width:		200",
"*messageText.height:		60",
"*messageText.displayCaret:	false",
"*messageText*string:		\
You have no application defaults\\n\
file installed. Copy the file\\n\
Flexemu.ad to $HOME/Flexemu and\\n\
set the environment variable\\n\
XUSERFILESEARCHPATH to $HOME/%N",
NULL};
#endif // HAVE_XTK

