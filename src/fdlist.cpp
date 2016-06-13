/*
    fdlist.cpp


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
#include <wx/strconv.h>

#include <misc1.h>

#include "fdlist.h"
#include "fdirent.h"
#include "filecont.h"
#include "flexerr.h"
#include "fdcframe.h"
#include "ffilebuf.h"
#include "fcinfo.h"
#include "fclipbrd.h"
#include "fcopyman.h"
#include "fmenufac.h"
#include "baddrrng.h"
#include "bmembuf.h"
#include "disconf.h"
#include "da.h"
#include "da6809.h"
#include "ifilecnt.h"
#include "bprocess.h"

#if defined(__WXGTK__) || defined(__WXX11__)
    #include "bitmaps/dnd_copy.xpm"
    #include "bitmaps/dnd_move.xpm"
    #include "bitmaps/dnd_none.xpm"
#endif

// define the columns indices for the wxListCtrl
const int LC_FILENAME   = 0;
const int LC_FILEDESC   = 1;
const int LC_RANDOM = 2;
const int LC_FILESIZE   = 3;
const int LC_FILEDATE   = 4;
const int LC_FILEATTR   = 5;

#ifdef WIN32
    wxString FlexDiskListCtrl::fileViewer = wxT("Notepad.exe");
#endif
#ifdef UNIX
    wxString FlexDiskListCtrl::fileViewer = wxT("xedit");
#endif

static const wxChar *fileDescription[] =
{
    wxT("BIN"), _("Binary file"),
    wxT("TXT"), _("Text file"),
    wxT("CMD"), _("Executable file"),
    wxT("BAS"), _("Basic file"),
    wxT("SYS"), _("System file"),
    wxT("BAK"), _("Backup file"),
    wxT("BAC"), _("Backup file"),
    wxT("DAT"), _("Data file"),
    NULL
};

int CALLBACK compareFlexListItems(long item1, long item2, long sortData)
{
    if (!item1 || !item2)
    {
        return 0;
    }

    FlexDirEntry *pObject1 = (FlexDirEntry *)item1;
    FlexDirEntry *pObject2 = (FlexDirEntry *)item2;

    switch (sortData)
    {
        case LC_RANDOM:
            return (pObject1->IsRandom() && !pObject2->IsRandom()) ? -1 : 1;

        case LC_FILESIZE:
            return (pObject1->GetSize() < pObject2->GetSize()) ? -1 : 1;

        case LC_FILEDATE:
            return (pObject1->GetDate() < pObject2->GetDate()) ? -1 : 1;

        case LC_FILEATTR:
            return (pObject1->GetAttributesString() < pObject2->GetAttributesString()) ?
                   -1 : 1;

        case LC_FILEDESC:
            return (FlexDiskListCtrl::GetFileDescription(pObject1) <
                    FlexDiskListCtrl::GetFileDescription(pObject2)) ? -1 : 1;

        case LC_FILENAME:
        default:
            return (pObject1->GetTotalFileName() < pObject2->GetTotalFileName()) ? -1 : 1;
    }
}
// Event table
BEGIN_EVENT_TABLE(FlexDiskListCtrl, wxListCtrl)
#ifdef wxUSE_DRAG_AND_DROP
    #ifndef __WXMOTIF__
        EVT_LIST_BEGIN_DRAG(LIST_CTRL, FlexDiskListCtrl::OnBeginDrag)
    #endif
#endif
    EVT_LIST_KEY_DOWN(LIST_CTRL, FlexDiskListCtrl::OnListKeyDown)
    EVT_LIST_COL_CLICK(LIST_CTRL, FlexDiskListCtrl::OnColClick)
    EVT_LIST_ITEM_SELECTED(LIST_CTRL, FlexDiskListCtrl::OnSelected)
    EVT_LIST_ITEM_DESELECTED(LIST_CTRL, FlexDiskListCtrl::OnDeselected)
    EVT_LIST_ITEM_ACTIVATED(LIST_CTRL, FlexDiskListCtrl::OnActivated)
    EVT_LIST_INSERT_ITEM(LIST_CTRL, FlexDiskListCtrl::OnInsertItem)
    EVT_LIST_DELETE_ITEM(LIST_CTRL, FlexDiskListCtrl::OnDeleteItem)
    EVT_RIGHT_DOWN(FlexDiskListCtrl::OnRightMouseDown)
    EVT_LEFT_DCLICK(FlexDiskListCtrl::OnLeftMouseDClick)
    EVT_MENU(MDI_SELECTALL, FlexDiskListCtrl::OnSelectAll)
    EVT_MENU(MDI_DESELECTALL, FlexDiskListCtrl::OnDeselectAll)
    EVT_MENU(MDI_FIND, FlexDiskListCtrl::OnFind)
    EVT_MENU(MDI_COPY, FlexDiskListCtrl::OnCopy)
    EVT_MENU(MDI_PASTE, FlexDiskListCtrl::OnPaste)
    EVT_MENU(MDI_DELETE, FlexDiskListCtrl::OnDelete)
    EVT_MENU(MDI_RENAME, FlexDiskListCtrl::OnRename)
    EVT_MENU(MDI_VIEW, FlexDiskListCtrl::OnView)
    EVT_MENU(MDI_SET_WRITEPROTECT, FlexDiskListCtrl::OnSetWriteProtect)
    EVT_MENU(MDI_CLEAR_WRITEPROTECT, FlexDiskListCtrl::OnClearWriteProtect)
    EVT_MENU(MDI_SET_READPROTECT, FlexDiskListCtrl::OnSetReadProtect)
    EVT_MENU(MDI_CLEAR_READPROTECT, FlexDiskListCtrl::OnClearReadProtect)
    EVT_MENU(MDI_SET_DELETEPROTECT, FlexDiskListCtrl::OnSetDeleteProtect)
    EVT_MENU(MDI_CLEAR_DELETEPROTECT, FlexDiskListCtrl::OnClearDeleteProtect)
    EVT_MENU(MDI_SET_CATALOGPROTECT, FlexDiskListCtrl::OnSetCatalogProtect)
    EVT_MENU(MDI_CLEAR_CATALOGPROTECT, FlexDiskListCtrl::OnClearCatalogProtect)
END_EVENT_TABLE()

FlexDiskListCtrl::FlexDiskListCtrl(wxWindow *parent, wxWindowID id,
                                   const wxPoint &pos, const wxSize &size, long style,
                                   FileContainerIf *container,
                                   const wxValidator &validator, const wxString &name) :
    wxListCtrl(parent, id, pos, size, style, validator, name),
    m_container(container), m_popupMenu(NULL), m_totalSize(0)
{
    // create all columns
    InsertColumn(LC_FILENAME, _("Filename"),   wxLIST_FORMAT_LEFT, 120);
    InsertColumn(LC_FILEDESC, _("Filetype"),   wxLIST_FORMAT_LEFT, 120);
    InsertColumn(LC_RANDOM,   _("Random"),     wxLIST_FORMAT_RIGHT, 65);
    InsertColumn(LC_FILESIZE, _("Size"),       wxLIST_FORMAT_RIGHT, 65);
    InsertColumn(LC_FILEDATE, _("Date"),       wxLIST_FORMAT_RIGHT, 90);
    InsertColumn(LC_FILEATTR, _("Attributes"), wxLIST_FORMAT_RIGHT, 60);

    m_popupMenu = FlexMenuFactory::CreateMenu(fEditMenuId);

    UpdateItems();

    if (m_container->IsWriteProtected())
    {
        SetBackgroundColour(*wxLIGHT_GREY);
    }
    else
    {
        SetBackgroundColour(*wxWHITE);
#ifdef wxUSE_DRAG_AND_DROP
#ifndef __WXMOTIF__
        SetDropTarget(new FlexFileDropTarget(this));
        //SetDropTarget(new FileDropTarget(this));
#endif
#endif
    }
}

int FlexDiskListCtrl::UpdateItems(void)
{
    int     index = 0;
    wxListItem  anItem;
    FileContainerIterator it;

    BeforeDeleteAllItems();
    DeleteAllItems();

    if (!m_container)
    {
        return index;
    }

    // the new FlexDirEntry can be referenced by anItem.m_data !
    try
    {
        for (it = m_container->begin(); it != m_container->end(); ++it)
        {
            FlexDirEntry *pDe = new FlexDirEntry(*it);
            wxString m_text(pDe->GetTotalFileName().chars(),
                            *wxConvCurrent);
            anItem.m_text   = m_text;
            anItem.m_itemId = index;
            anItem.m_col    = 0;
            anItem.m_data   = (long)pDe;
            anItem.m_mask   = wxLIST_MASK_TEXT | wxLIST_MASK_DATA;
            InsertItem(anItem);
            UpdateItem(index++, *pDe);
        }
    }
    catch (FlexException &ex)
    {
        wxMessageBox(ex.wwhat(), _("FLEXplorer Error"),
                     wxOK | wxCENTRE | wxICON_EXCLAMATION, this);
    }

    return index;
}

FlexDiskListCtrl::~FlexDiskListCtrl()
{
    BeforeDeleteAllItems();
    DeleteAllItems();
    delete m_popupMenu;

    if (m_container != NULL)
    {
        m_container->Close();
        delete m_container;
        m_container = NULL;
    }
}

void FlexDiskListCtrl::UpdateItem(int item, FlexDirEntry &de)
{
    wxString filesize;

    filesize.Printf(wxT("%d"), de.GetSize());
    SetItem(item, LC_RANDOM,   de.IsRandom() ? _("Yes") : wxT(""));
    SetItem(item, LC_FILESIZE, filesize);
    wxString date(de.GetDate().GetDateString(), *wxConvCurrent);
    SetItem(item, LC_FILEDATE, date);
    wxString attributes(de.GetAttributesString().chars(), *wxConvCurrent);
    SetItem(item, LC_FILEATTR, attributes);
    SetItem(item, LC_FILEDESC, GetFileDescription(&de));
}

const wxMenu *FlexDiskListCtrl::GetMenu(void)
{
    return m_popupMenu;
}

/* get the index of each selected item  */
/* the pointer returned MUST BE DELETED */
/* with delete[]                        */
int FlexDiskListCtrl::GetSelections(long **pItems) const
{
    long *items;
    int selectionCount;

    *pItems = NULL;
    selectionCount = GetSelectedItemCount();

    if (selectionCount > 0)
    {
        int i;
        long item = -1;

        *pItems = items = new long[selectionCount];

        for (i = 0; i < selectionCount; i++)
        {
            item = GetNextItem(item, wxLIST_NEXT_ALL,
                               wxLIST_STATE_SELECTED);
            items[i] = item;
        } // for
    } // if

    return selectionCount;
}

void FlexDiskListCtrl::DeleteSelectedItems(bool askUser /* = TRUE */)
{
    wxString fileName;
    long *pItems;
    int count = 0;

    count = GetSelections(&pItems);

    if (m_container && count > 0)
    {
        wxMessageDialog *dialog = new wxMessageDialog(
            this, _("Delete all selected files"),
            _("Delete Files"),
            wxYES_NO | wxICON_QUESTION | wxCENTRE);

        if (!askUser || (dialog->ShowModal() == wxID_YES))
        {
            for (; count > 0; count--)
            {
                long item = pItems[count - 1];

                if (item >= 0)
                {
                    fileName = GetItemText(item);

                    try
                    {
                        m_container->DeleteFile(fileName.mb_str(*wxConvCurrent));
                    }
                    catch (FlexException &ex)
                    {
                        int r = wxMessageBox(ex.wwhat(),
                                             _("FLEXplorer Error"),
                                             wxOK | wxCANCEL | wxCENTRE,
                                             this);

                        if (r != wxOK)
                        {
                            break;
                        }

                        continue;
                    }

                    DeleteItem(item);
                }
            } // for
        } // if

        delete dialog;
    } // if

    delete [] pItems;
    m_totalSize = 0;
    Notify();
}

void FlexDiskListCtrl::RenameSelectedItems(void)
{
    long            *pItems;
    wxString        itemText;
    wxTextEntryDialog   *dialog = NULL;

    int count = GetSelections(&pItems);

    if (m_container && count > 0)
    {
        wxString fName;
        wxString dialogText;
        bool do_rename = false;

        // edit the last selected item
        long item = pItems[count - 1];
        itemText = GetItemText(item);
        dialogText = _("Please input the new file name");

        do
        {

            dialog = new wxTextEntryDialog(this,
                                           dialogText, _("Rename file"), itemText);
            do_rename = dialog->ShowModal() == wxID_OK;
            fName  = dialog->GetValue();
            dialogText =  _("Wrong file name specified.");
            dialogText += wxT("\n");
            dialogText += _("Please input the new file name");
        }
        while (do_rename &&
               !m_container->CheckFilename(fName.mb_str(*wxConvCurrent)));

        if (do_rename)
        {
            try
            {
                FlexDirEntry *pDe;
                fName.MakeUpper();
                m_container->RenameFile(
                    itemText.mb_str(*wxConvCurrent),
                    fName.mb_str(*wxConvCurrent));
                pDe = (FlexDirEntry *)GetItemData(item);
                pDe->SetTotalFileName(fName.mb_str(*wxConvCurrent));
                SetItemText(item, fName);
                UpdateItem(item, *pDe);
            }
            catch (FlexException &ex)
            {
                wxMessageBox(ex.wwhat(), _("FLEXplorer Error"),
                             wxOK | wxCENTRE | wxICON_EXCLAMATION);
            }
        }

        delete dialog;
    } // if

    delete [] pItems;
}

void FlexDiskListCtrl::ViewSelectedItems(void)
{
    FlexFileBuffer  buffer;
    long        *pItems;
    wxString    fileName;
    BProcess    process(fileViewer.mb_str(*wxConvCurrent), ".");

    int count = GetSelections(&pItems);

    if (m_container && count > 0)
    {
        // edit the last selected item
        long item = pItems[count - 1];
        fileName = GetItemText(item);

        try
        {

            m_container->ReadToBuffer(fileName.mb_str(*wxConvCurrent), buffer);

            /* unfinished
            {
            FlexDirEntry *pDe;

            pDe = (FlexDirEntry *)GetItemData(item);
            if (!strcmp(pDe->GetFileExt(), "CMD")) {
                ProcessCmdFile(fileName, &buffer);
                delete [] pItems;
                return;
            }
            } */
            if ((m_container->GetContainerType() & TYPE_CONTAINER) &&
                buffer.IsFlexTextFile())
            {
                buffer.ConvertFromFlex();
            }

#ifdef WIN32
            char path[MAX_PATH], tempPath[MAX_PATH];

            if (!GetTempPath(MAX_PATH, tempPath))
            {
                throw FlexException(GetLastError(),
                      _("In function GetTempPath"));
            }

            if (!GetTempFileName(tempPath, _("FLX"), 0, path))
            {
                throw FlexException(GetLastError(),
                                    _("In function GetTempFileName"));
            }

            if (buffer.WriteToFile(path))
            {
                process.AddArgument(path);

                if (!process.Start())
                {
                    throw FlexException(GetLastError(), fileViewer);
                }
            }
            else
            {
                throw FlexException(FERR_CREATE_TEMP_FILE, path);
            }

#else
            int fd;
            char cTemplate[PATH_MAX];

            getcwd(cTemplate, PATH_MAX - 10);
            strcat(cTemplate, "/FLXXXXXXX");
            fd = mkstemp(cTemplate);

            if (fd != -1 && buffer.WriteToFile(fd))
            {
                close(fd);
                process.AddArgument(cTemplate);

                if (!process.Start())
                {
                    throw FlexException(
                        FERR_CREATE_PROCESS,
                        fileViewer.mb_str(*wxConvCurrent),
                        cTemplate);
                }
            }
            else
            {
                throw FlexException(FERR_CREATE_TEMP_FILE, cTemplate);
            }

#endif
        }
        catch (FlexException &ex)
        {
            wxMessageBox(ex.wwhat(), _("FLEXplorer Error"),
                         wxOK | wxCENTRE | wxICON_EXCLAMATION);
        }
    } // if

    delete [] pItems;
}

void FlexDiskListCtrl::ProcessCmdFile(const char *, FlexFileBuffer *buffer)
{
    BAddressRanges ranges;
    BMemoryBuffer *memory;
    DisassemblerConfig config;
    Disassembler da;

    if (!buffer)
    {
        return;
    }

    if (!ranges.ReadFrom(buffer))
    {
        throw FlexException(FERR_WRONG_FLEX_BIN_FORMAT);
    }

    memory = new BMemoryBuffer(ranges.GetMax() - ranges.GetMin() + 1,
                               ranges.GetMin());
    memory->FillWith(0);
    buffer->CopyTo(*memory);

    da.SetLbLDisassembler(new Da6809());
    config.Add(ranges);
    config.SetStartAddress(ranges.GetStartAddress(), "BEGIN");
    da.DisassembleWithConfig(config, memory);
    //ranges.PrintOn(stdout);
    return;
}

#ifdef wxUSE_DRAG_AND_DROP
#ifndef __WXMOTIF__
void FlexDiskListCtrl::OnBeginDrag(wxListEvent &event)
{
    if (m_container != NULL)
    {
        wxDragResult    result;
        int     flags = 0;
        FlexDnDFiles    files;
        FlexFileDataObject *dragData;
        FlexFileList    fileList;
        FlexFileList::Node *node = NULL;
        int count = 0;

        GetFileList(fileList);

        for (node = fileList.GetFirst(); node; node = node->GetNext())
        {
            FlexFileBuffer *pFileBuffer = new FlexFileBuffer;
            BString fileName(node->GetData()->mb_str(*wxConvCurrent));

            try
            {
                m_container->ReadToBuffer(fileName, *pFileBuffer);
                files.Add(pFileBuffer);
            }
            catch (FlexException &ex)
            {
                int r = wxMessageBox(ex.wwhat(),
                                     _("FLEXPlorer Error"),
                                     wxOK | wxCANCEL | wxCENTRE, this);

                if (r == wxCANCEL)
                {
                    delete pFileBuffer;
                    return;
                }
            }

            count++;
        }

        fileList.DeleteContents(TRUE);
        dragData = new FlexFileDataObject();
        dragData->GetDataFrom(files);
        wxDropSource dragSource(this,
                                wxDROP_ICON(dnd_copy),
                                wxDROP_ICON(dnd_move),
                                wxDROP_ICON(dnd_none));
        dragSource.SetData(*dragData);
        flags = wxDrag_CopyOnly;
#ifdef __WXMSW__
        // seems to be a bug in wxMSW that
        // the move flag has to be set otherwise
        // a Drop (copy or move) is not possible
        flags |= wxDrag_AllowMove;
#endif
        //if (!m_container->IsWriteProtected())
        //  flags |= wxDrag_AllowMove;
        // When allowing to move files it must be
        // save to either copy all of them or
        // abort the Drag&Drop with an error
        result = dragSource.DoDragDrop(flags);

        switch (result)
        {
            case wxDragMove:
                break;

            // unfinished file moved, source should be deleted;
            case wxDragCopy:
                break;

            case wxDragCancel:
                break;

            case wxDragError:
                break;

            case wxDragLink:
                break;

            case wxDragNone:
                break;
        }

        delete dragData; // savely delete after DoDragDrop, also deletes pData
    }
    else
    {
        event.Skip();
    }
}
#endif
#endif

void FlexDiskListCtrl::OnSelected(wxListEvent &event)
{
    FlexDirEntry    de;

    if (m_container && m_container->FindFile(
            GetItemText(event.m_itemIndex).mb_str(*wxConvCurrent), de))
    {
        m_totalSize += de.GetSize();
    }

    Notify();
}

void FlexDiskListCtrl::OnDeselected(wxListEvent &event)
{
    FlexDirEntry    de;
    wxString        t;

    if (m_container && m_container->FindFile(
            GetItemText(event.m_itemIndex).mb_str(*wxConvCurrent), de))
    {
        m_totalSize -= de.GetSize();
    }

    Notify();
}

void FlexDiskListCtrl::Notify(void)
{
    if (m_statusbarObserver != NULL)
    {
        int id = OBSERVE_STATUS_BAR;

        m_statusbarObserver->UpdateFrom(&id);
    }
}

void FlexDiskListCtrl::OnActivated(wxListEvent &WXUNUSED(event))
{
    Notify();
}

void FlexDiskListCtrl::OnColClick(wxListEvent &event)
{
    SortItems(compareFlexListItems, event.m_col);
}


void FlexDiskListCtrl::OnListKeyDown(wxListEvent &event)
{
    int keyCode = event.GetKeyCode();

    //char s[33];
    //sprintf((char *)s, "keyCode: %08x\n", keyCode);
    //printf(s);
    switch (keyCode)
    {
        // Delete-key: delete all selected files
        case WXK_DELETE:
            DeleteSelectedItems();
            break;

        case WXK_F5:
            UpdateItems();
            break;

        // unfinished: not supported on Windows yet:
        case wxT('C') - wxT('A') + 1 :
            CopyToClipboard();
            break;

        case wxT('V') - wxT('A') + 1 :
            PasteFromClipboard();
            break;
    } // switch
}

void FlexDiskListCtrl::SetPropertyOnSelectedItems(int protection,
        bool isToBeSet)
{
    wxString fileName;
    long *pItems;
    int count = 0;
    int setMask;
    int clearMask;

    if (m_container == NULL)
    {
        return;
    }

    for (count = GetSelections(&pItems); count > 0; count--)
    {
        FlexDirEntry de, *pDe;
        long item;

        fileName = GetItemText(item = pItems[count - 1]);

        try
        {
            setMask = isToBeSet ? protection : 0;
            clearMask = isToBeSet ? 0 : protection;
            pDe = (FlexDirEntry *)GetItemData(item);
            m_container->SetAttributes(
                fileName.mb_str(*wxConvCurrent),
                setMask, clearMask);

            // read back properties from container
            // (because maybe not all properties are
            // supported)
            if (m_container->FindFile(
                    fileName.mb_str(*wxConvCurrent), de))
            {
                setMask = de.GetAttributes();
                clearMask = ~setMask;
            }

            pDe->SetAttributes(setMask, clearMask);
            UpdateItem(item, *pDe);
        }
        catch (FlexException &ex)
        {
            int r = wxMessageBox(ex.wwhat(), _("FLEXPlorer Error"),
                                 wxOK | wxCANCEL | wxCENTRE, this);

            if (r != wxOK)
            {
                break;
            }
        }
    } // for

    delete [] pItems;
}

void FlexDiskListCtrl::OnViewProperties(wxCommandEvent &)
{
    FlexContainerInfo   info;
    wxString        str, caption;
    int         t, s;

    if (m_container)
    {
        try
        {
            m_container->GetInfo(info);
        }
        catch (FlexException &ex)
        {
            wxMessageBox(ex.wwhat(), _("FLEXPlorer Error"),
                         wxOK | wxCENTRE, this);
            return;
        }

        caption.Printf(_("Container %s"), info.GetName());
        info.GetTrackSector(&t, &s);
        str.Append(_("Path: "));
        wxString path(info.GetPath(), *wxConvCurrent);
        str += path;
        str += wxT("\n");
        str.Append(_("Type: "));
        wxString type(info.GetTypeString(), *wxConvCurrent);
        type = wxGetTranslation(type);
        str += type;
        str += wxT("\n");
        str.Append(_("Date: "));
        wxString date(info.GetDate().GetDateString(), *wxConvCurrent);
        str += date;

        if (t && s)
        {
            str += wxT("\n");
            str.Append(_("Tracks: "));
            str << t;
            str += wxT("\n");
            str.Append(_("Sectors: "));
            str << s;
        }

        str += wxT("\n");
        str.Append(_("Size: "));
        str << info.GetTotalSize();
        str.Append(_(" KByte"));
        str += wxT("\n");
        str.Append(_("Free: "));
        str << info.GetFree();
        str.Append(_(" KByte"));
        str += wxT("\n");

        if (info.GetAttributes() & FLX_READONLY)
        {
            str.Append(_("Attributes: read-only"));
            str += wxT("\n");
        }

        wxMessageBox(str, caption, wxOK | wxCENTRE, this);

    }
}

void FlexDiskListCtrl::OnLeftMouseDClick(wxMouseEvent &WXUNUSED(event))
{
    ViewSelectedItems();
}

void FlexDiskListCtrl::OnRightMouseDown(wxMouseEvent &event)
{
    if (m_popupMenu)
    {
        PopupMenu(m_popupMenu, event.GetX(), event.GetY());
    }
}

void FlexDiskListCtrl::OnInsertItem(wxListEvent &)
{
}

void FlexDiskListCtrl::OnDeleteItem(wxListEvent &event)
{
    // delete the FlexDirEntry * object
    // stored in m_data
    FlexDirEntry *pDe = (FlexDirEntry *)event.GetData();
    delete pDe;
}

void FlexDiskListCtrl::BeforeDeleteAllItems()
{
    // delete the FlexDirEntry * object
    // stored in m_data
    int item = -1;
    FlexDirEntry *pDe;

    while (true)
    {
        item = GetNextItem(item);

        if (item == -1)
        {
            break;
        }

        pDe = (FlexDirEntry *)GetItemData(item);
        delete pDe;
        SetItemData(item, 0);
    }
}

IMPLEMENT_MODIFY_PROPERTY(OnSetWriteProtect,     WRITE_PROTECT,   TRUE)
IMPLEMENT_MODIFY_PROPERTY(OnClearWriteProtect,   WRITE_PROTECT,   FALSE)
IMPLEMENT_MODIFY_PROPERTY(OnSetReadProtect,      READ_PROTECT,    TRUE)
IMPLEMENT_MODIFY_PROPERTY(OnClearReadProtect,    READ_PROTECT,    FALSE)
IMPLEMENT_MODIFY_PROPERTY(OnSetDeleteProtect,    DELETE_PROTECT,  TRUE)
IMPLEMENT_MODIFY_PROPERTY(OnClearDeleteProtect,  DELETE_PROTECT,  FALSE)
IMPLEMENT_MODIFY_PROPERTY(OnSetCatalogProtect,   CATALOG_PROTECT, TRUE)
IMPLEMENT_MODIFY_PROPERTY(OnClearCatalogProtect, CATALOG_PROTECT, FALSE)

IMPLEMENT_SIMPLE_MENUCOMMAND(OnSelectAll, SelectAllFiles)
IMPLEMENT_SIMPLE_MENUCOMMAND(OnDeselectAll, DeselectAllFiles)
IMPLEMENT_SIMPLE_MENUCOMMAND(OnFind, FindFiles)
IMPLEMENT_SIMPLE_MENUCOMMAND(OnCopy, CopyToClipboard)
IMPLEMENT_SIMPLE_MENUCOMMAND(OnPaste, PasteFromClipboard)
IMPLEMENT_SIMPLE_MENUCOMMAND(OnDelete, DeleteSelectedItems)
IMPLEMENT_SIMPLE_MENUCOMMAND(OnRename, RenameSelectedItems)
IMPLEMENT_SIMPLE_MENUCOMMAND(OnView, ViewSelectedItems)

void FlexDiskListCtrl::GetFileList(FlexFileList &fileList)
{
    int     count;
    long        *pItems;

    fileList.DeleteContents(TRUE);
    count = GetSelections(&pItems);

    for (int i = 0; i < count; i++)
    {
        wxString *pFileName = new wxString(GetItemText(pItems[i]));
#ifdef UNIX
        pFileName->MakeLower();
#endif
        fileList.Append(pFileName);
    }

    delete [] pItems;
}

void FlexDiskListCtrl::SelectAllFiles(void)
{
    int i;

    for (i = GetItemCount() - 1; i >= 0; i--)
    {
        SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void FlexDiskListCtrl::DeselectAllFiles(void)
{
    int i;

    for (i = GetItemCount() - 1; i >= 0; i--)
    {
        SetItemState(i, 0, wxLIST_STATE_SELECTED);
    }
}

void FlexDiskListCtrl::FindFiles(void)
{
    static wxString value(_("*.*"));

    wxTextEntryDialog dialog(this,
                             _("Enter a pattern for files to look for."
                               "Wildcards ? and * are allowed"),
                             _("Find Files"));

    dialog.SetValue(value);
    int result = dialog.ShowModal();

    if (result == wxID_OK)
    {
        value = dialog.GetValue();
        BString filePattern(value.mb_str(*wxConvCurrent));
        FileContainerIterator it(filePattern);
        BString bFileName;

        DeselectAllFiles();

        for (it = m_container->begin(); it != m_container->end(); ++it)
        {
            int i;

            bFileName = (*it).GetTotalFileName();
            wxString fileName(bFileName, *wxConvCurrent);

            if (bFileName.multimatches(filePattern, ';', true) &&
                (i = FindItem(-1, fileName)) >= 0)
            {
                SetItemState(i, wxLIST_STATE_SELECTED,
                             wxLIST_STATE_SELECTED);
            }
        }
    }
}

void FlexDiskListCtrl::CopyToClipboard(void)
{
    FlexFileList fileList;
    FlexDnDFiles files;
    FlexFileList::Node *node;
    int count = 0;
    FlexFileDataObject *pClipboardData;

    wxClipboardLocker locker; // implicit open/close wxTheClipboard

    if (!locker)
    {
        wxLogError(_("Can't open clipboard."));
        wxBell();
        return;
    }

    GetFileList(fileList);

    for (node = fileList.GetFirst(); node; node = node->GetNext())
    {
        FlexFileBuffer *pFileBuffer = new FlexFileBuffer;
        BString fileName(node->GetData()->mb_str(*wxConvCurrent));

        try
        {
            m_container->ReadToBuffer(fileName, *pFileBuffer);
            files.Add(pFileBuffer);
            count++;
        }
        catch (FlexException &ex)
        {
            wxMessageBox(ex.wwhat(), _("FLEXplorer Error"),
                         wxOK | wxCENTRE | wxICON_EXCLAMATION, this);
        }
    }

    pClipboardData = new FlexFileDataObject();
    pClipboardData->GetDataFrom(files);

    if (!wxTheClipboard->SetData(pClipboardData))
    {
        wxLogError(_("Can't copy data to the clipboard"));
        wxBell();
    }

    fileList.DeleteContents(TRUE);
}

bool FlexDiskListCtrl::PasteFromClipboard(void)
{
    FlexFileDataObject flexFileData;

    if (m_container->IsWriteProtected())
    {
        wxLogError(_("Container is read-only"));
        return false;
    }

    wxClipboardLocker locker; // implicit open/close wxTheClipboard

    if (!locker)
    {
        wxLogError(_("Can't open clipboard."));
        wxBell();
        return false;
    }

    if (!wxTheClipboard->IsSupported(FlexFileFormatId))
    {
        wxLogWarning(_("No Flex file data on clipboard"));
        wxBell();
        return false;
    }

    if (!wxTheClipboard->GetData(flexFileData))
    {
        wxLogWarning(_("Unable to paste Flex file data from clipboard"));
        wxBell();
        return false;
    }

    FlexDnDFiles files;
    bool     result;

    flexFileData.SetDataTo(files);

    result = PasteFrom(files);

    return result;
}

bool FlexDiskListCtrl::PasteFrom(FlexDnDFiles &files)
{
    const char      *p;
    int             index;
    unsigned int    i;
    FlexDirEntry    *pDe;

    for (i = 0; i < files.GetFileCount(); i++)
    {
        try
        {
            p = files.GetBuffer(i).GetFilename();
            GetContainer()->WriteFromBuffer(files.GetBuffer(i));
            pDe = new FlexDirEntry;

            if (GetContainer()->FindFile(p, *pDe))
            {
                wxListItem anItem;

                wxString m_text(pDe->GetTotalFileName(),
                                *wxConvCurrent);
                anItem.m_text   = m_text;
                anItem.m_itemId = 0;
                anItem.m_col    = 0;
                anItem.m_data   = (long)pDe;
                anItem.m_mask   = wxLIST_MASK_TEXT |
                                  wxLIST_MASK_DATA;
                index = InsertItem(anItem);
                UpdateItem(index, *pDe);
            }
        }
        catch (FlexException &ex)
        {
            if (ex.GetErrorCode() == FERR_DISK_FULL_WRITING)
            {
                wxMessageBox(ex.wwhat(), _("FLEXplorer Error"),
                             wxOK | wxCENTRE | wxICON_EXCLAMATION, this);
                return false;
            }
            else
            {
                wxMessageBox(ex.wwhat(), _("FLEXplorer Error"),
                             wxOK | wxCENTRE, this);
            }
        }
    }

    return true;
}

wxString FlexDiskListCtrl::GetFileDescription(const FlexDirEntry *pDe)
{
    wxString        tmp;
    wxChar          **pFDesc;

    pFDesc = (wxChar **)fileDescription;
    wxString extension(pDe->GetFileExt(), *wxConvCurrent);

    while (*pFDesc != NULL)
    {
        tmp = *pFDesc;

        if (tmp.CmpNoCase(extension) == 0)
        {
            tmp = wxGetTranslation(*(++pFDesc));
            return tmp;
        }

        pFDesc += 2;
    }

    extension += wxT(" ");
    extension += _("file");
    return extension;
}

