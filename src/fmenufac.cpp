/*
    fmenufac.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004  W. Schwotzer

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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    // Include your minimal set of headers here, or wx.h
    #include <wx/wx.h>
#endif
//#include <wx/menu.h>
//#include <wx/menuitem.h>

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
wxMenu *FlexMenuFactory::CreateMenu(tFlxMenuId menuId)
{
    wxMenu     *pMenu = new wxMenu;
    wxMenuItem *pItem;
    wxBitmap *bitmaps[3];

    switch (menuId)
    {
        case fFileMenuId:
#ifdef __WXMSW__
            bitmaps[0] = new wxBitmap("new_con",  wxBITMAP_TYPE_RESOURCE);
            bitmaps[1] = new wxBitmap("open_con", wxBITMAP_TYPE_RESOURCE);
            bitmaps[2] = new wxBitmap("open_dir", wxBITMAP_TYPE_RESOURCE);
#else
            bitmaps[0] = new wxBitmap(new_con_xpm);
            bitmaps[1] = new wxBitmap(open_con_xpm);
            bitmaps[2] = new wxBitmap(open_dir_xpm);
#endif

            // Create File menu
            //pMenu->SetTitle("File");
            pItem = new wxMenuItem(pMenu, MDI_NEW_CONTAINER,
                                   _("&New Container...\tCtrl+N"));
            pItem->SetBitmap(*bitmaps[0]);
            pMenu->Append(pItem);
            pItem = new wxMenuItem(pMenu, MDI_OPEN_CONTAINER,
                                   _("&Open Container...\tCtrl+O"));
            pItem->SetBitmap(*bitmaps[1]);
            pMenu->Append(pItem);
            pItem = new wxMenuItem(pMenu, MDI_OPEN_DIRECTORY,
                                   _("Open &Directory...\tCtrl+D"));
            pItem->SetBitmap(*bitmaps[2]);
            pMenu->Append(pItem);
            pMenu->AppendSeparator();
            pMenu->Append(MDI_QUIT, _("E&xit\tCtrl+X"));

            for (int i = 0; i < 3; i++)
            {
                delete bitmaps[i];
            }

            break;

        case fEditMenuId:
#ifdef __WXMSW__
            bitmaps[0] = new wxBitmap("copy",     wxBITMAP_TYPE_RESOURCE);
            bitmaps[1] = new wxBitmap("paste",    wxBITMAP_TYPE_RESOURCE);
#else
            bitmaps[0] = new wxBitmap(copy_xpm);
            bitmaps[1] = new wxBitmap(paste_xpm);
#endif
            // Create Edit menu
            //pMenu->SetTitle("Edit");
            pMenu->Append(MDI_SELECTALL, _("Select &All\tCtrl+A"));
            pMenu->Append(MDI_DESELECTALL, _("D&eselect All"));
            pMenu->Append(MDI_FIND, _("&Find Files...\tCtrl+F"));
            pMenu->AppendSeparator();
            pItem = new wxMenuItem(pMenu, MDI_COPY, _("&Copy\tCtrl+C"));
            pItem->SetBitmap(*bitmaps[0]);
            pMenu->Append(pItem);
            pItem = new wxMenuItem(pMenu, MDI_PASTE, _("&Paste\tCtrl+V"));
            pItem->SetBitmap(*bitmaps[1]);
            pMenu->Append(pItem);
            pMenu->AppendSeparator();
            pMenu->Append(MDI_DELETE, _("&Delete\tDEL"));
            pMenu->Append(MDI_RENAME, _("&Rename"));
            pMenu->Append(MDI_VIEW, _("&View"));
            pMenu->AppendSeparator();
            pMenu->Append(MDI_SET_WRITEPROTECT,   _("&Set Write Protect"));
            pMenu->Append(MDI_SET_READPROTECT,   _("Set Read Protect"));
            pMenu->Append(MDI_SET_DELETEPROTECT,   _("Set Delete Protect"));
            pMenu->Append(MDI_SET_CATALOGPROTECT,   _("Set Catalog Protect"));
            pMenu->AppendSeparator();
            pMenu->Append(MDI_CLEAR_WRITEPROTECT, _("Clear &Write Protect"));
            pMenu->Append(MDI_CLEAR_READPROTECT, _("Clear Read Protect"));
            pMenu->Append(MDI_CLEAR_DELETEPROTECT, _("Clear Delete Protect"));
            pMenu->Append(MDI_CLEAR_CATALOGPROTECT, _("Clear Catalog Protect"));
            delete bitmaps[0];
            delete bitmaps[1];
            break;

        case fContainerMenuId:
            // Create Container menu
            //pMenu->SetTitle("Container");
            pMenu->Append(MDI_CONTAINER_PROPERTIES, _("&Properties..."));
            pMenu->AppendSeparator();
            pMenu->Append(MDI_CONTAINER_CLOSE, _("&Close"));
            break;

        case fExtrasMenuId:
            // Create Extras menu
            //pMenu->SetTitle("Extras");
            pMenu->Append(MDI_OPTIONS, _("&Options..."));
            break;

        case fHelpMenuId:
            // Create Help menu
            //pMenu->SetTitle("Help");
            pMenu->Append(MDI_ABOUT, _("&About..."));
            break;

        default:
            delete pMenu;
            pMenu = NULL;
    }

    return pMenu;
}

