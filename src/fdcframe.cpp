/*
    fdcframe.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2018  W. Schwotzer

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

#include "fmenufac.h"
#include "fdcframe.h"
#include "fcopyman.h"

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
    #include "bitmaps/container.xpm"
    /*
    #include "bitmaps/new_con.xpm"
    #include "bitmaps/open_con.xpm"
    #include "bitmaps/open_dir.xpm"
    #include "bitmaps/copy.xpm"
    #include "bitmaps/paste.xpm"
    */
#endif

// Event table
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

FlexChildFrame::FlexChildFrame(
    wxMDIParentFrame *parent,
    const wxString &title,
    const wxPoint &pos,
    const wxSize &size,
    const long style,
    FileContainerIf *container) :
    wxMDIChildFrame(parent, -1, title, pos, size, style),
    m_listCtrl(NULL), m_clipboardObserver(NULL)
{
    // Give it an icon
#ifdef __WXMSW__
    SetIcon(wxIcon(wxT("CONTAINER_ICON")));
#else
    SetIcon(wxIcon(container_xpm));
#endif


    // create a list control
    int width, height;
    GetClientSize(&width, &height);
    m_listCtrl = new FlexDiskListCtrl(
        this, LIST_CTRL, wxPoint(0, 0), wxSize(width, height),
        wxLC_REPORT | /*wxLC_EDIT_LABELS |*/ wxSUNKEN_BORDER |
        wxVSCROLL | wxHSCROLL, container);

    wxLayoutConstraints *c = new wxLayoutConstraints;
    c->top.SameAs(this, wxTop);
    c->left.SameAs(this, wxLeft);
    c->width.SameAs(this, wxWidth);
    c->height.SameAs(this, wxHeight);
    m_listCtrl->SetConstraints(c);

    wxMenuBar  *pMenuBar = new wxMenuBar(wxMB_DOCKABLE);

    pMenuBar->Append(FlexMenuFactory::CreateMenu(fFileMenuId),
                     _("&File"));
    pMenuBar->Append(FlexMenuFactory::CreateMenu(fEditMenuId),
                     _("&Edit"));
    pMenuBar->Append(FlexMenuFactory::CreateMenu(fContainerMenuId),
                     _("&Container"));
    pMenuBar->Append(FlexMenuFactory::CreateMenu(fExtrasMenuId),
                     _("&Extras"));
    pMenuBar->Append(FlexMenuFactory::CreateMenu(fHelpMenuId),
                     _("&Help"));

    SetMenuBar(pMenuBar);

#ifdef __WXMSW__
    // create a status bar
    wxStatusBar *statusBar = this->CreateStatusBar(3);
    int id = OBSERVE_STATUS_BAR;

    const int fieldWidth[3] = { 300, -1, -1 };
    statusBar->SetFieldsCount(3, fieldWidth);

    // Attach itself for statusbar update
    m_listCtrl->Attach(this);
    UpdateFrom(&id);
#endif
}

FlexChildFrame::~FlexChildFrame()
{
    if (m_clipboardObserver && m_listCtrl)
    {
        m_clipboardObserver->UpdateFrom(m_listCtrl->GetContainer());
    }

    delete m_listCtrl;
    m_listCtrl = NULL;
}

void FlexChildFrame::Attach(BObserver *clipboardObserver)
{
    m_clipboardObserver = clipboardObserver;
}

void FlexChildFrame::Detach(BObserver *clipboardObserver)
{
    if (m_clipboardObserver == clipboardObserver)
    {
        m_clipboardObserver = NULL;
    }
}

void FlexChildFrame::OnActivate(wxActivateEvent &event)
{
    if (event.GetActive() && m_listCtrl)
    {
        int id = OBSERVE_STATUS_BAR;

        m_listCtrl->SetFocus();
        UpdateFrom(&id);
    }
}

void FlexChildFrame::OnSetFocus(wxFocusEvent &)
{
    if (m_listCtrl)
    {
        int id = OBSERVE_STATUS_BAR;

        m_listCtrl->SetFocus();
        UpdateFrom(&id);
    }
}

void FlexChildFrame::OnCloseChild(wxCommandEvent &WXUNUSED(event))
{
    Close(TRUE);
}

void FlexChildFrame::UpdateFrom(const void *pObject)
{
    const int id = *static_cast<const int *>(pObject);

    if (id == OBSERVE_STATUS_BAR)
    {
        wxStatusBar *sBar;

        if (m_listCtrl && (sBar = GetStatusBar()))
        {
            wxString buf;

            buf.Printf(_("%d File(s) selected"), m_listCtrl->GetFileCount());
            sBar->SetStatusText(buf, 1);
            buf.Printf(_("%d Byte"), m_listCtrl->GetTotalSize());
            sBar->SetStatusText(buf, 2);
        }
    }
}

FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnSelectAll)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnDeselectAll)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnFind)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnCopy)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnPaste)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnDelete)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnRename)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnView)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnSetWriteProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnClearWriteProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnSetReadProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnClearReadProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnSetDeleteProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnClearDeleteProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnSetCatalogProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnClearCatalogProtect)
FORWARD_MENUCOMMAND_TO(FlexChildFrame, m_listCtrl, OnViewProperties)

