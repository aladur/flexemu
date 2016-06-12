/*
    fsetup.h


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

#ifndef __fsetup_h__
#define __fsetup_h__

#include <wx/defs.h>
#include <wx/app.h>

#include "misc1.h"
#include "absgui.h"
#include "foptman.h"


class FlexemuSetup;
class FlexemuOptionsDialog;

// Application class can be globally accessed by wxGetApp():

DECLARE_APP(FlexemuSetup)

/*------------------------------------------------------
 FlexemuSetup
 The main application class 
 A setup dialog for editing options for flexemu
--------------------------------------------------------*/
class FlexemuSetup : public wxApp
{
public:
	bool OnInit(void);
	int OnExit(void);
	void SetReturnCode(int r) { ret = r; };
	int GetReturnCode(void) { return ret; };

private:
	int ret;
	FlexOptionManager optionMan;
	FlexemuOptionsDialog *dialog;
	struct sOptions options;
	struct sGuiOptions guiOptions;
};

#endif

