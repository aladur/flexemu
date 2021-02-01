/*
    fdcframe.h


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

#ifndef FDCFRAME_INCLUDED
#define FDCFRAME_INCLUDED

#include "misc1.h"
// For compilers that support precompilation, includes "wx.h".
#include "warnoff.h"
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    // Include your minimal set of headers here, or wx.h
    #include <wx/wx.h>
#endif
#include "warnon.h"

#include "fdlist.h"
#include "fmenufac.h"
#include "bobserv.h"
#include <memory>


class wxMDIParentFrame;

/*---------------------------------------------------------
 FlexChildFrame
 A MDI child frame used to display one FLEX container each
-----------------------------------------------------------*/
class FlexChildFrame: public wxMDIChildFrame, public BObserver
{
public:
    FlexChildFrame(wxMDIParentFrame *parent, const wxString &title,
                   const wxPoint &pos, const wxSize &size, const long style,
                   FileContainerIf *container);
    virtual ~FlexChildFrame();
    wxMenu *GetPopupMenu();
    void UpdateFrom(NotifyId id, void *param = nullptr) override;
    void ViewProperties();
    void OnActivate(wxActivateEvent &event);
    void OnSetFocus(wxFocusEvent &event);
    FlexDiskListCtrl &GetListControl()
    {
        return *listCtrl.get();
    };

    DECLARE_MENUCOMMAND(OnSelectAll)
    DECLARE_MENUCOMMAND(OnDeselectAll)
    DECLARE_MENUCOMMAND(OnFind)
    DECLARE_MENUCOMMAND(OnCopy)
    DECLARE_MENUCOMMAND(OnPaste)
    DECLARE_MENUCOMMAND(OnDelete)
    DECLARE_MENUCOMMAND(OnRename)
    DECLARE_MENUCOMMAND(OnView)
    DECLARE_MENUCOMMAND(OnSetWriteProtect)
    DECLARE_MENUCOMMAND(OnClearWriteProtect)
    DECLARE_MENUCOMMAND(OnSetReadProtect)
    DECLARE_MENUCOMMAND(OnClearReadProtect)
    DECLARE_MENUCOMMAND(OnSetDeleteProtect)
    DECLARE_MENUCOMMAND(OnClearDeleteProtect)
    DECLARE_MENUCOMMAND(OnSetCatalogProtect)
    DECLARE_MENUCOMMAND(OnClearCatalogProtect)
    DECLARE_MENUCOMMAND(OnViewProperties)
    DECLARE_MENUCOMMAND(OnCloseChild)
#ifdef __MSGTK__
    DECLARE_MENUCOMMAND(OnSortFilename)
    DECLARE_MENUCOMMAND(OnSortRandom)
    DECLARE_MENUCOMMAND(OnSortFilesize)
    DECLARE_MENUCOMMAND(OnSortFiledate)
    DECLARE_MENUCOMMAND(OnSortFileattr)
    DECLARE_MENUCOMMAND(OnSortFiledesc)
#endif

private:
    std::unique_ptr<FlexDiskListCtrl> listCtrl;
#include "warnoff.h"
    DECLARE_EVENT_TABLE()
#include "warnon.h"
};

#endif // FDCFRAME_INCLUDED

