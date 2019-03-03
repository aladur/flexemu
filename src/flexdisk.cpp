/*
    flexdisk.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2019  W. Schwotzer

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
#include <wx/clipbrd.h>

#include "misc1.h"

#include "flexdisk.h"
#include "fdpframe.h"
#include "fdcframe.h"
#include "ffilecnt.h"
#include "fdirent.h"
#include "fcopyman.h"
#include "fdlist.h"
#include "bregistr.h"
#include "brcfile.h"
#include "benv.h"
#include "flexerr.h"


IMPLEMENT_APP(FLEXplorer)

/*------------------------------------------------------
 FLEXplorer implementation (The Application class)
--------------------------------------------------------*/
bool FLEXplorer::OnInit()
{
    wxLocale::AddCatalogLookupPathPrefix(wxT("."));
    wxLocale::AddCatalogLookupPathPrefix(wxT("./locale"));

    m_locale.Init();
    m_locale.AddCatalog(wxT("flexemu"));

    ReadDefaultOptions();
    SetAppName(_("FLEXplorer"));
#ifdef wxUSE_DRAG_AND_DROP
    wxTheClipboard->UsePrimarySelection();
#endif

    int width = 820;

    // Create the main frame window
    FlexParentFrame *frame =
        new FlexParentFrame((wxFrame *)nullptr, -1, GetAppName(),
                            wxPoint(-1, -1), wxSize(width, 700),
                            wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL);
    frame->Show(true);
    SetTopWindow(frame);

    for (int i = 1; i < argc; ++i)
    {
        if (!frame->OpenContainer(wxConvCurrent->cWX2MB(argv[i])))
        {
            break;
        }
    }

    return true;
}

int FLEXplorer::OnExit()
{
    WriteDefaultOptions();
    return 0;
}

void FLEXplorer::WriteDefaultOptions()
{
#ifdef WIN32
    BRegistry reg(BRegistry::currentUser, FLEXPLOREREG);
    reg.SetValue(FLEXPLORERBOOTSECTORFILE, FlexFileContainer::bootSectorFile);
    reg.SetValue(FLEXPLORERTEXTFLAG, FlexCopyManager::autoTextConversion ?
                  1 : 0);
#endif
#ifdef UNIX
    std::string rcFileName;
    BEnvironment env;

    if (!env.GetValue("HOME", rcFileName))
    {
        rcFileName = ".";
    }

    rcFileName += PATHSEPARATORSTRING FLEXPLORERRC;
    BRcFile rcFile(rcFileName.c_str());
    rcFile.Initialize(); // truncate file
    rcFile.SetValue(FLEXPLORERBOOTSECTORFILE,
                    FlexFileContainer::bootSectorFile.c_str());
    rcFile.SetValue(FLEXPLORERTEXTFLAG,
                    FlexCopyManager::autoTextConversion ? 1 : 0);
#endif
}

void FLEXplorer::ReadDefaultOptions()
{
    int int_result;
    std::string string_result;
#ifdef WIN32
    BRegistry reg(BRegistry::localMachine, FLEXPLOREREG);

    if (!reg.GetValue(FLEXPLORERBOOTSECTORFILE, string_result) &&
        !string_result.empty())
    {
        FlexFileContainer::bootSectorFile = string_result;
    }

    if (!reg.GetValue(FLEXPLORERTEXTFLAG, int_result))
    {
        FlexCopyManager::autoTextConversion = (int_result != 0);
    }
#endif
#ifdef UNIX
    std::string rcFileName;
    BEnvironment env;

    if (!env.GetValue("HOME", rcFileName))
    {
        rcFileName = ".";
    }

    rcFileName += PATHSEPARATORSTRING FLEXPLORERRC;
    BRcFile rcFile(rcFileName.c_str());

    if (!rcFile.GetValue(FLEXPLORERBOOTSECTORFILE, string_result) &&
        !string_result.empty())
    {
        FlexFileContainer::bootSectorFile = string_result;
    }

    if (!rcFile.GetValue(FLEXPLORERTEXTFLAG, int_result))
    {
        FlexCopyManager::autoTextConversion = (int_result != 0);
    }
#endif
}

