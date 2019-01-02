/*
    fsetup.cpp


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
#include <vector>
#include <memory>


IMPLEMENT_APP(FlexemuSetup)

/*------------------------------------------------------
 flexemuSetup implementation (The Application class)
--------------------------------------------------------*/
bool FlexemuSetup::OnInit()
{
#ifdef _UNICODE
    int i;

    std::vector<wxCharBuffer> args;
    auto mb_argv = std::unique_ptr<char *[]>(new char *[argc]);
    args.reserve(argc);

    for (i = 0; i < argc; i++)
    {
        args.push_back(wxConvCurrent->cWX2MB(argv[i]));
    }
    for (i = 0; i < argc; i++)
    {
        mb_argv[i] = (char *)args[i].data();
    }

    optionMan.InitOptions(&guiOptions, &options, argc, mb_argv.get());
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

    auto dialog = new FlexemuOptionsDialog(&guiOptions, &options,
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

    return true;
}

