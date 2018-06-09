/*
    fsetup.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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

#include "misc1.h"

#include "fsetpdlg.h"
#include "fsetup.h"


IMPLEMENT_APP(FlexemuSetup)

/*------------------------------------------------------
 flexemuSetup implementation (The Application class)
--------------------------------------------------------*/
typedef char *PCHAR;
bool FlexemuSetup::OnInit()
{
    dialog = nullptr;
#ifdef _UNICODE
    char **mb_argv;
    int i;

    mb_argv = new PCHAR[argc];

    for (i = 0; i < argc; i++)
    {
        mb_argv[i] =
            const_cast<char *>((const char *)wxConvCurrent->cWX2MB(argv[i]));
    }

    optionMan.InitOptions(&guiOptions, &options, argc, mb_argv);
#else
    optionMan.InitOptions(&guiOptions, &options, argc, argv);
#endif
    wxLocale::AddCatalogLookupPathPrefix(wxT("."));
    wxLocale::AddCatalogLookupPathPrefix(wxT("./locale"));

    m_locale.Init();
    m_locale.AddCatalog(wxT("flexemu"));

    optionMan.GetOptions(&guiOptions, &options);
    optionMan.GetEnvironmentOptions(&guiOptions, &options);
    SetAppName(wxT("FlexemuSetup"));
    SetExitOnFrameDelete(true);

    dialog = new FlexemuOptionsDialog(&guiOptions, &options,
                                      nullptr, -1, _("Flexemu Options Dialog"),
                                      wxDefaultPosition, wxDefaultSize,
                                      wxCAPTION | wxSYSTEM_MENU |
                                      wxDIALOG_NO_PARENT | wxRESIZE_BORDER);

    SetTopWindow(dialog);

    if (dialog->ShowModal() == wxID_OK)
    {
        optionMan.WriteOptions(&guiOptions, &options);
    }

    dialog->Destroy();

#ifdef _UNICODE
    delete [] mb_argv;
#endif
    return true;
}

