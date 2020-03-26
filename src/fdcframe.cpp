/*
    fdcframe.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2020  W. Schwotzer

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

#include "misc1.h"

#include "fmenufac.h"
#include "fdcframe.h"
#include "fcopyman.h"

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
    #include "bitmaps/contain.xpm"
#endif

// Event table
#include "warnoff.h"
BEGIN_EVENT_TABLE(FlexChildFrame, wxMDIChildFrame)
    EVT_SET_FOCUS(FlexChildFrame::OnSetFocus)
    EVT_MENU(MDI_DESELECTALL, FlexChildFrame::OnDeselectAll)
    EVT_MENU(MDI_SELECTALL, FlexChildFrame::OnSelectAll)
    EVT_MENU(MDI_FIND, FlexChildFrame::OnFind)
    EVT_MENU(MDI_COPY, FlexChildFrame::OnCopy)
    EVT_MENU(MDI_PASTE, FlexChildFrame::OnPaste)
    EVT_MENU(MDI_DELETE, FlexChildFrame::OnDelete)
    EVT_MENU(MDI_RENAME, FlexChildFrame::OnRename)
    EVT_MENU(MDI_VIEW, FlexChildFrame::OnView)
    EVT_MENU(MDI_SET_WRITEPROTECT, FlexChildFrame::OnSetWriteProtect)
    EVT_MENU(MDI_CLEAR_WRITEPROTECT, FlexChildFrame::OnClearWriteProtect)
    EVT_MENU(MDI_SET_READPROTECT, FlexChildFrame::OnSetReadProtect)
    EVT_MENU(MDI_CLEAR_READPROTECT, FlexChildFrame::OnClearReadProtect)
    EVT_MENU(MDI_SET_DELETEPROTECT, FlexChildFrame::OnSetDeleteProtect)
    EVT_MENU(MDI_CLEAR_DELETEPROTECT, FlexChildFrame::OnClearDeleteProtect)
    EVT_MENU(MDI_SET_CATALOGPROTECT, FlexChildFrame::OnSetCatalogProtect)
    EVT_MENU(MDI_CLEAR_CATALOGPROTECT, FlexChildFrame::OnClearCatalogProtect)
    EVT_MENU(MDI_CONTAINER_CLOSE, FlexChildFrame::OnCloseChild)
    EVT_MENU(MDI_CONTAINER_PROPERTIES, FlexChildFrame::OnViewProperties)
END_EVENT_TABLE()
#include "warnon.h"

FlexChildFrame::FlexChildFrame(
    wxMDIParentFrame *parent,
    const wxString &title,
    const wxPoint &pos,
    const wxSize &size,
    const long style,
    FileContainerIf *container) :
    wxMDIChildFrame(parent, -1, title, pos, size, style),
    listCtrl(nullptr)
{
    // Give it an icon
#ifdef __WXMSW__
    SetIcon(wxIcon("CONTAINER_ICON"));
#else
    SetIcon(wxIcon(contain_xpm));
#endif


    // create a list control
    int width, height;
    GetClientSize(&width, &height);
    listCtrl = std::unique_ptr<FlexDiskListCtrl>(new FlexDiskListCtrl(
        this, LIST_CTRL, wxPoint(0, 0), wxSize(width, height),
        wxLC_REPORT | /*wxLC_EDIT_LABELS |*/ wxSUNKEN_BORDER |
        wxVSCROLL | wxHSCROLL, container));

    wxLayoutConstraints *c = new wxLayoutConstraints;
    c->top.SameAs(this, wxTop);
    c->left.SameAs(this, wxLeft);
    c->width.SameAs(this, wxWidth);
    c->height.SameAs(this, wxHeight);
    listCtrl->SetConstraints(c);

    wxMenuBar  *pMenuBar = new wxMenuBar(wxMB_DOCKABLE);

    pMenuBar->Append(FlexMenuFactory::CreateFileMenu(), _("&File"));
    pMenuBar->Append(FlexMenuFactory::CreateEditMenu(), _("&Edit"));
    pMenuBar->Append(FlexMenuFactory::CreateContainerMenu(), _("&Container"));
    pMenuBar->Append(FlexMenuFactory::CreateExtrasMenu(), _("&Extras"));
    pMenuBar->Append(FlexMenuFactory::CreateHelpMenu(), _("&Help"));

    SetMenuBar(pMenuBar);

#ifdef __WXMSW__
    // create a status bar
    wxStatusBar *statusBar = this->CreateStatusBar(3);

    const int fieldWidth[3] = { 300, -1, -1 };
    statusBar->SetFieldsCount(3, fieldWidth);

    // Attach itself for statusbar update
    listCtrl->Attach(*this);
    UpdateFrom(NotifyId::UpdateStatusBar);
#endif
}

FlexChildFrame::~FlexChildFrame()
{
}

void FlexChildFrame::OnActivate(wxActivateEvent &event)
{
    if (event.GetActive() && listCtrl)
    {
        listCtrl->SetFocus();
        UpdateFrom(NotifyId::UpdateStatusBar);
    }
}

void FlexChildFrame::OnSetFocus(wxFocusEvent &)
{
    if (listCtrl)
    {
        listCtrl->SetFocus();
        UpdateFrom(NotifyId::UpdateStatusBar);
    }
}

void FlexChildFrame::OnCloseChild(wxCommandEvent &WXUNUSED(event))
{
    Close(TRUE);
}

void FlexChildFrame::UpdateFrom(NotifyId id, void *)
{
    if (!listCtrl)
    {
        return;
    }

    if (id == NotifyId::UpdateStatusBar)
    {        
        wxStatusBar *sBar = GetStatusBar();

        if (sBar != nullptr)
        {
            wxString buf;

            buf.Printf(_("%d File(s) selected"), listCtrl->GetFileCount());
            sBar->SetStatusText(buf, 1);
            buf.Printf(_("%d Byte"), listCtrl->GetTotalSize());
            sBar->SetStatusText(buf, 2);
        }
    }
}

FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnSelectAll)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnDeselectAll)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnFind)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnCopy)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnPaste)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnDelete)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnRename)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnView)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnSetWriteProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnClearWriteProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnSetReadProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnClearReadProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnSetDeleteProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnClearDeleteProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnSetCatalogProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnClearCatalogProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, listCtrl, OnViewProperties)

