/*
    fmenufac.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2021  W. Schwotzer

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

#include "misc1.h"

#include "warnoff.h"
// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    // Include your minimal set of headers here, or wx.h
    #include <wx/wx.h>
#endif
#include "warnon.h"

#include "flexdisk.h"
#include "fmenufac.h"

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
    #include "bitmaps/new_con.xpm"
    #include "bitmaps/open_con.xpm"
    #include "bitmaps/open_dir.xpm"
    #include "bitmaps/copy.xpm"
    #include "bitmaps/paste.xpm"
#endif

// single source for creating a menu:
// can be used for main menu, container menu or popup menus

wxMenu *FlexMenuFactory::CreateFileMenu()
{
    wxMenu *pMenu = new wxMenu;
    wxMenuItem *pItem;

#ifdef __WXMSW__
    auto newContainerBitmap = wxBitmap("new_con",wxBITMAP_TYPE_RESOURCE);
    auto openContainerBitmap = wxBitmap("open_con", wxBITMAP_TYPE_RESOURCE);
    auto openDirectoryBitmap = wxBitmap("open_dir", wxBITMAP_TYPE_RESOURCE);
#else
    auto newContainerBitmap = wxBitmap(new_con_xpm);
    auto openContainerBitmap = wxBitmap(open_con_xpm);
    auto openDirectoryBitmap = wxBitmap(open_dir_xpm);
#endif

    pItem = new wxMenuItem(pMenu, MDI_NEW_CONTAINER,
                           _("&New Container...\tCtrl+N"));
    pItem->SetBitmap(newContainerBitmap);
    pMenu->Append(pItem);

    pItem = new wxMenuItem(pMenu, MDI_OPEN_CONTAINER,
                           _("&Open Container...\tCtrl+O"));
    pItem->SetBitmap(openContainerBitmap);
    pMenu->Append(pItem);

    pItem = new wxMenuItem(pMenu, MDI_OPEN_DIRECTORY,
                           _("Open &Directory...\tCtrl+D"));
    pItem->SetBitmap(openDirectoryBitmap);
    pMenu->Append(pItem);

    pMenu->AppendSeparator();
    pMenu->Append(MDI_QUIT, _("E&xit\tCtrl+X"));

    return pMenu;
}

wxMenu *FlexMenuFactory::CreateEditMenu()
{
    wxMenu *pMenu = new wxMenu;
    wxMenuItem *pItem;

#ifdef __WXMSW__
    auto copyBitmap = wxBitmap("copy", wxBITMAP_TYPE_RESOURCE);
    auto pasteBitmap = wxBitmap("paste", wxBITMAP_TYPE_RESOURCE);
#else
    auto copyBitmap = wxBitmap(copy_xpm);
    auto pasteBitmap = wxBitmap(paste_xpm);
#endif

    pMenu->Append(MDI_SELECTALL, _("Select &All\tCtrl+A"));
    pMenu->Append(MDI_DESELECTALL, _("D&eselect All"));
    pMenu->Append(MDI_FIND, _("&Find Files...\tCtrl+F"));
    pMenu->AppendSeparator();

    pItem = new wxMenuItem(pMenu, MDI_COPY, _("&Copy\tCtrl+C"));
    pItem->SetBitmap(copyBitmap);
    pMenu->Append(pItem);
    pItem = new wxMenuItem(pMenu, MDI_PASTE, _("&Paste\tCtrl+V"));
    pItem->SetBitmap(pasteBitmap);
    pMenu->Append(pItem);
    pMenu->AppendSeparator();

    pMenu->Append(MDI_DELETE, _("&Delete\tDEL"));
    pMenu->Append(MDI_RENAME, _("&Rename"));
    pMenu->Append(MDI_VIEW, _("&View"));
    pMenu->AppendSeparator();

    pMenu->Append(MDI_SET_WRITEPROTECT, _("&Set Write Protect"));
    pMenu->Append(MDI_SET_READPROTECT, _("Set Read Protect"));
    pMenu->Append(MDI_SET_DELETEPROTECT, _("Set Delete Protect"));
    pMenu->Append(MDI_SET_CATALOGPROTECT, _("Set Catalog Protect"));
    pMenu->AppendSeparator();

    pMenu->Append(MDI_CLEAR_WRITEPROTECT, _("Clear &Write Protect"));
    pMenu->Append(MDI_CLEAR_READPROTECT, _("Clear Read Protect"));
    pMenu->Append(MDI_CLEAR_DELETEPROTECT, _("Clear Delete Protect"));
    pMenu->Append(MDI_CLEAR_CATALOGPROTECT, _("Clear Catalog Protect"));

    return pMenu;
}

wxMenu *FlexMenuFactory::CreateContainerMenu()
{
    wxMenu *pMenu = new wxMenu;

    pMenu->Append(MDI_CONTAINER_PROPERTIES, _("&Properties..."));
    pMenu->AppendSeparator();
    pMenu->Append(MDI_CONTAINER_CLOSE, _("&Close"));

    return pMenu;
}

wxMenu *FlexMenuFactory::CreateExtrasMenu()
{
    wxMenu *pMenu = new wxMenu;

    pMenu->Append(MDI_OPTIONS, _("&Options..."));

    return pMenu;
}

wxMenu *FlexMenuFactory::CreateHelpMenu()
{
    wxMenu *pMenu = new wxMenu;

    pMenu->Append(MDI_ABOUT, _("&About..."));

    return pMenu;
}

