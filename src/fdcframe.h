/*
    fdcframe.h


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

#include <misc1.h>
// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include "fdlist.h"
#include "fmenufac.h"
#include "bobserv.h"

class wxMDIParentFrame;

/*---------------------------------------------------------
 FlexChildFrame
 A MDI child frame used to display one FLEX container each
-----------------------------------------------------------*/
class FlexChildFrame: public wxMDIChildFrame, public BObserver
{
public:
	FlexChildFrame(wxMDIParentFrame *parent, const wxString& title,
		const wxPoint& pos, const wxSize& size, const long style,
		FileContainerIf *container);
	virtual ~FlexChildFrame(void);
	void Attach(BObserver *clipboardObserver);
	void Detach(BObserver *clipboardObserver);
	wxMenu *GetPopupMenu(void);
	virtual void Update(const void *pObject);
	void ViewProperties(void);
	void OnActivate(wxActivateEvent& event);
	void OnSetFocus(wxFocusEvent& event);
	const FlexDiskListCtrl *GetListControl(void) { return m_listCtrl;};

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
	FlexDiskListCtrl	*m_listCtrl;
	BObserver			*m_clipboardObserver;
DECLARE_EVENT_TABLE()
};

