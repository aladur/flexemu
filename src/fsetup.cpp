/*
    fsetup.cpp


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


// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <misc1.h>

#include "fsetpdlg.h"
#include "fsetup.h"


IMPLEMENT_APP(FlexemuSetup)

/*------------------------------------------------------
 flexemuSetup implementation (The Application class)
--------------------------------------------------------*/
bool FlexemuSetup::OnInit(void)
{
	dialog = NULL;

	optionMan.InitOptions(&guiOptions, &options, argc, argv);
	optionMan.GetOptions(&guiOptions, &options);
	optionMan.GetEnvironmentOptions(&guiOptions, &options);
	SetAppName("FlexemuSetup");
	SetExitOnFrameDelete(true);

	dialog = new FlexemuOptionsDialog(&guiOptions, &options,
		NULL, -1, "Flexemu Options Dialog", wxDefaultPosition,
		wxDefaultSize,
		wxTHICK_FRAME | wxCAPTION | wxSYSTEM_MENU | wxDIALOG_NO_PARENT |
		wxRESIZE_BORDER);

	SetTopWindow(dialog);
	dialog->Show(true);

	return true;
}

int FlexemuSetup::OnExit(void)
{
	if (GetReturnCode() == wxID_OK)
		optionMan.WriteOptions(&guiOptions, &options);

	return 0;
}

