/*
    fdlist.cpp


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
#include <wx/strconv.h>

#include "misc1.h"

#include "fdlist.h"
#include "fdirent.h"
#include "filecont.h"
#include "flexerr.h"
#include "fdcframe.h"
#include "ffilebuf.h"
#include "fcinfo.h"
#include "fcopyman.h"
#include "fmenufac.h"
#include "ifilecnt.h"
#include "bprocess.h"
#include "cvtwchar.h"
#include "bdir.h"
#include <memory>
#include <algorithm>

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

static const char *fileDescription[] =
{
    "BIN", _("Binary file"),
    "TXT", _("Text file"),
    "CMD", _("Executable file"),
    "BAS", _("Basic file"),
    "SYS", _("System file"),
    "BAK", _("Backup file"),
    "BAC", _("Backup file"),
    "DAT", _("Data file"),
    nullptr
};

int CALLBACK compareFlexListItems(wxIntPtr item1, wxIntPtr item2,
                                  wxIntPtr sortData)
{
    if (!item1 || !item2)
    {
        return 0;
    }

    auto dirEntry1 = *reinterpret_cast<FlexDirEntry *>(item1);
    auto dirEntry2 = *reinterpret_cast<FlexDirEntry *>(item2);

    switch (sortData)
    {
        case LC_RANDOM:
            return (dirEntry1.IsRandom() && !dirEntry2.IsRandom()) ? -1 : 1;

        case LC_FILESIZE:
            return (dirEntry1.GetSize() < dirEntry2.GetSize()) ? -1 : 1;

        case LC_FILEDATE:
            return (dirEntry1.GetDate() < dirEntry2.GetDate()) ? -1 : 1;

        case LC_FILEATTR:
            return (dirEntry1.GetAttributesString() <
                    dirEntry2.GetAttributesString()) ?  -1 : 1;

        case LC_FILEDESC:
            return (FlexDiskListCtrl::GetFileDescription(dirEntry1) <
                    FlexDiskListCtrl::GetFileDescription(dirEntry2)) ? -1 : 1;

        case LC_FILENAME:
        default:
            return (dirEntry1.GetTotalFileName() <
                    dirEntry2.GetTotalFileName()) ? -1 : 1;
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
                                   const wxPoint &pos, const wxSize &size,
                                   long style, FileContainerIf *container,
                                   const wxValidator &validator,
                                   const wxString &name) :
    wxListCtrl(parent, id, pos, size, style, validator, name),
    m_container(container), m_popupMenu(nullptr), m_totalSize(0)
{
    // create all columns
    InsertColumn(LC_FILENAME, _("Filename"),   wxLIST_FORMAT_LEFT, 120);
    InsertColumn(LC_FILEDESC, _("Filetype"),   wxLIST_FORMAT_LEFT, 120);
    InsertColumn(LC_RANDOM,   _("Random"),     wxLIST_FORMAT_RIGHT, 65);
    InsertColumn(LC_FILESIZE, _("Size"),       wxLIST_FORMAT_RIGHT, 65);
    InsertColumn(LC_FILEDATE, _("Date"),       wxLIST_FORMAT_RIGHT, 90);
    InsertColumn(LC_FILEATTR, _("Attributes"), wxLIST_FORMAT_RIGHT, 60);

    m_popupMenu.reset(FlexMenuFactory::CreateEditMenu());

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
#endif
#endif
    }
}

int FlexDiskListCtrl::UpdateItems()
{
    int     index = 0;
    wxListItem  anItem;
    FileContainerIterator iter;

    BeforeDeleteAllItems();
    DeleteAllItems();

    if (!m_container)
    {
        return index;
    }

    // the new FlexDirEntry can be referenced by anItem.m_data !
    try
    {
        for (iter = m_container->begin(); iter != m_container->end(); ++iter)
        {
            auto dirEntry = new FlexDirEntry(*iter);
            wxString m_text(iter->GetTotalFileName().c_str(), wxConvUTF8);
            anItem.m_text   = m_text;
            anItem.m_itemId = index;
            anItem.m_col    = 0;
            anItem.m_data   = reinterpret_cast<wxUIntPtr>(dirEntry);
            anItem.m_mask   = wxLIST_MASK_TEXT | wxLIST_MASK_DATA;
            InsertItem(anItem);
            UpdateItem(index++, *dirEntry);
        }
    }
    catch (FlexException &ex)
    {
        wxMessageBox(ex.what(), _("FLEXplorer Error"),
                     wxOK | wxCENTRE | wxICON_EXCLAMATION, this);
    }

    return index;
}

FlexDiskListCtrl::~FlexDiskListCtrl()
{
    BeforeDeleteAllItems();
    DeleteAllItems();

    if (m_container != nullptr)
    {
        m_container->Close();
    }
}

void FlexDiskListCtrl::UpdateItem(int item, FlexDirEntry &dirEntry)
{
    wxString filesize;

    filesize.Printf("%d", dirEntry.GetSize());
    SetItem(item, LC_RANDOM, dirEntry.IsRandom() ? _("Yes") : "");
    SetItem(item, LC_FILESIZE, filesize);
    wxString date(dirEntry.GetDate().GetDateString(), wxConvUTF8);
    SetItem(item, LC_FILEDATE, date);
    wxString attributes(dirEntry.GetAttributesString().c_str(), wxConvUTF8);
    SetItem(item, LC_FILEATTR, attributes);
    SetItem(item, LC_FILEDESC, GetFileDescription(dirEntry));
}

const wxMenu *FlexDiskListCtrl::GetMenu()
{
    return m_popupMenu.get();
}

/* Get a vector of indices of each selected item  */
std::vector<long> FlexDiskListCtrl::GetSelections() const
{
    std::vector<long> items;
    auto selectionCount = GetSelectedItemCount();

    if (selectionCount > 0)
    {
        long item = -1;

        items.reserve(selectionCount);

        for (;;)
        {
            item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (item == -1)
            {
                break;
            }
            else
            {
                items.push_back(item);
            }
        }
    }

    return items;
}

void FlexDiskListCtrl::DeleteSelectedItems(bool askUser /* = true */)
{
    auto items = GetSelections();

    if (m_container && !items.empty())
    {
        auto dialog = std::unique_ptr<wxMessageDialog>(new wxMessageDialog(
            this, _("Delete all selected files"),
            _("Delete Files"),
            wxYES_NO | wxICON_QUESTION | wxCENTRE));

        if (!askUser || (dialog->ShowModal() == wxID_YES))
        {
            std::reverse(std::begin(items), std::end(items));

            for (auto item : items)
            {
                auto fileName = GetItemText(item);

                try
                {
                    m_container->DeleteFile(fileName.ToUTF8());
                }
                catch (FlexException &ex)
                {
                    int r = wxMessageBox(ex.what(),
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
            } // for
        } // if
    } // if

    m_totalSize = 0;
    Notify();
}

void FlexDiskListCtrl::RenameSelectedItems()
{
    auto items = GetSelections();

    if (m_container && !items.empty())
    {
        wxString fName;
        wxString dialogText;
        bool do_rename = false;

        // edit the last selected item
        auto item = items[items.size() - 1];
        auto itemText = GetItemText(item);
        dialogText = _("Please input the new file name");

        do
        {
            auto dialog = std::unique_ptr<wxTextEntryDialog>(
                    new wxTextEntryDialog(this, dialogText, _("Rename file"),
                                          itemText));
            do_rename = dialog->ShowModal() == wxID_OK;
            fName  = dialog->GetValue();
            dialogText =  _("Wrong file name specified.");
            dialogText += "\n";
            dialogText += _("Please input the new file name");
        }
        while (do_rename && !m_container->CheckFilename(fName.ToUTF8()));

        if (do_rename)
        {
            try
            {
                fName.MakeUpper();
                m_container->RenameFile(itemText.ToUTF8(), fName.ToUTF8());
                auto dirEntry =
                    *reinterpret_cast<FlexDirEntry *>(GetItemData(item));
                dirEntry.SetTotalFileName(fName.ToUTF8());
                SetItemText(item, fName);
                UpdateItem(item, dirEntry);
            }
            catch (FlexException &ex)
            {
                wxMessageBox(ex.what(), _("FLEXplorer Error"),
                             wxOK | wxCENTRE | wxICON_EXCLAMATION);
            }
        }
    } // if
}

void FlexDiskListCtrl::ViewSelectedItems()
{
    if (!m_container)
    {
        return;
    }

    for (const auto &item : GetSelections())
    {
        std::string fileName(GetItemText(item).ToUTF8().data());
        std::transform(fileName.begin(), fileName.end(),
                        fileName.begin(), ::tolower);

        try
        {
            auto buffer = m_container->ReadToBuffer(fileName.c_str());

            if (buffer.IsFlexTextFile())
            {
                buffer.ConvertToTextFile();
            }
            else
            {
                buffer.ConvertToDumpFile(16);
            }

#ifdef _WIN32
            // Windows ShellExtensions works best with a
            // well known file extension.
            if (getFileExtension(fileName) != ".txt")
            {
                fileName += ".txt";
            }
#endif

            auto tempPath = getTempPath() + PATHSEPARATORSTRING "flexplorer";

            if (!BDirectory::Exists(tempPath))
            {
                if (!BDirectory::Create(tempPath))
                {
                    throw FlexException(FERR_UNABLE_TO_CREATE, tempPath);
                }
            }

            tempPath += PATHSEPARATORSTRING +
                            getFileName(m_container->GetPath());
            if (!BDirectory::Exists(tempPath))
            {
                if (!BDirectory::Create(tempPath))
                {
                    throw FlexException(FERR_UNABLE_TO_CREATE, tempPath);
                }
            }

            const std::string tempFile =
                tempPath + PATHSEPARATORSTRING + fileName;

            if (buffer.WriteToFile(tempFile.c_str()))
            {
#ifdef _WIN32
                SHELLEXECUTEINFO execInfo;
                std::wstring wTempFile = ConvertToUtf16String(tempFile);

                memset(&execInfo, 0, sizeof(execInfo));
                execInfo.cbSize = sizeof(execInfo);
                execInfo.lpVerb = L"open";
                execInfo.lpFile = wTempFile.c_str();
                execInfo.lpDirectory = L".";
                execInfo.nShow = SW_SHOWNORMAL;

                if (!ShellExecuteEx(&execInfo))
                {
                    throw FlexException(GetLastError(),
                        std::string("In function ViewSelectedItems()."));
                }
#else
                // On Unix/Linux the mime type is used depending
                // on the file contents. It can have any file extension.
                BProcess process("xdg-open", ".");

                process.AddArgument(tempFile);

                if (!process.Start())
                {
                    throw FlexException(FERR_CREATE_PROCESS,
                                        "xdg-open", tempFile);
                }
#endif
            }
            else
            {
                throw FlexException(FERR_UNABLE_TO_CREATE, tempFile);
            }
        }
        catch (FlexException &ex)
        {
            wxMessageBox(ex.what(), _("FLEXplorer Error"),
                         wxOK | wxCENTRE | wxICON_EXCLAMATION);
        }
    } // for
}

#ifdef wxUSE_DRAG_AND_DROP
#ifndef __WXMOTIF__
void FlexDiskListCtrl::OnBeginDrag(wxListEvent &event)
{
    if (m_container != nullptr)
    {
        wxDragResult    result;
        int     flags = 0;
        FlexDnDFiles    files;
        int count = 0;

        for (auto fileName : GetSelectedFileNames())
        {
            try
            {
                files.Add(m_container->ReadToBuffer(fileName.c_str()));
            }
            catch (FlexException &ex)
            {
                int r = wxMessageBox(ex.what(),
                                     _("FLEXPlorer Error"),
                                     wxOK | wxCANCEL | wxCENTRE, this);

                if (r == wxCANCEL)
                {
                    return;
                }
            }

            count++;
        }

        auto dragData =
                 std::unique_ptr<FlexFileDataObject>(new FlexFileDataObject);
        dragData->ReadDataFrom(files);
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
    FlexDirEntry dirEntry;

    if (m_container && m_container->FindFile(
            GetItemText(event.m_itemIndex).ToUTF8(), dirEntry))
    {
        m_totalSize += dirEntry.GetSize();
    }

    Notify();
}

void FlexDiskListCtrl::OnDeselected(wxListEvent &event)
{
    FlexDirEntry dirEntry;

    if (m_container && m_container->FindFile(
            GetItemText(event.m_itemIndex).ToUTF8(), dirEntry))
    {
        m_totalSize -= dirEntry.GetSize();
    }

    Notify();
}

void FlexDiskListCtrl::Notify()
{
    BObserved::Notify(NotifyId::UpdateStatusBar);
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

    switch (keyCode)
    {
        // Delete-key: delete all selected files
        case WXK_DELETE:
            DeleteSelectedItems();
            break;

        case WXK_F5:
            UpdateItems();
            break;

        case 'C' - 'A' + 1 :
            CopyToClipboard();
            break;

        case 'V' - 'A' + 1 :
            PasteFromClipboard();
            break;
    } // switch
}

void FlexDiskListCtrl::SetPropertyOnSelectedItems(Byte protection,
        bool isToBeSet)
{
    if (m_container == nullptr)
    {
        return;
    }

    for (auto item : GetSelections())
    {
        FlexDirEntry dirEntry;

        auto fileName = GetItemText(item);

        try
        {
            Byte setMask = isToBeSet ? protection : 0;
            Byte clearMask = isToBeSet ? 0 : protection;
            auto newDirEntry =
                *reinterpret_cast<FlexDirEntry *>(GetItemData(item));
            m_container->SetAttributes(fileName.ToUTF8(), setMask, clearMask);

            // read back properties from container
            // (because maybe not all properties are
            // supported)
            if (m_container->FindFile(fileName.ToUTF8(), dirEntry))
            {
                setMask = dirEntry.GetAttributes();
                clearMask = ~setMask;
            }

            newDirEntry.SetAttributes(setMask, clearMask);
            UpdateItem(item, newDirEntry);
        }
        catch (FlexException &ex)
        {
            int r = wxMessageBox(ex.what(), _("FLEXPlorer Error"),
                                 wxOK | wxCANCEL | wxCENTRE, this);

            if (r != wxOK)
            {
                break;
            }
        }
    } // for
}

void FlexDiskListCtrl::OnViewProperties(wxCommandEvent &)
{
    FlexContainerInfo   info;
    wxString        str, caption;
    int tracks, sectors;

    if (m_container)
    {
        try
        {
            m_container->GetInfo(info);
        }
        catch (FlexException &ex)
        {
            wxMessageBox(ex.what(), _("FLEXPlorer Error"),
                         wxOK | wxCENTRE, this);
            return;
        }

        caption.Printf(_("Container %s"), info.GetName());
        info.GetTrackSector(&tracks, &sectors);
        str.Append(_("Path: "));
        wxString path(info.GetPath().c_str(), wxConvUTF8);
        str += path;
        str += "\n";
        str.Append(_("Type: "));
        wxString type(info.GetTypeString().c_str(), wxConvUTF8);
        type = wxGetTranslation(type);
        str += type;
        str += "\n";
        str.Append(_("Date: "));
        wxString date(info.GetDate().GetDateString(), wxConvUTF8);
        str += date;

        str += "\n";
        str.Append(_("Tracks: "));
        str << tracks;
        str += "\n";
        str.Append(_("Sectors: "));
        str << sectors;

        str += "\n";
        str.Append(_("Size: "));
        str << info.GetTotalSize();
        str.Append(_(" KByte"));
        str += "\n";
        str.Append(_("Free: "));
        str << info.GetFree();
        str.Append(_(" KByte"));
        str += "\n";

        if (info.GetAttributes() & FLX_READONLY)
        {
            str.Append(_("Attributes: read-only"));
            str += "\n";
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
        PopupMenu(m_popupMenu.get(), event.GetX(), event.GetY());
    }
}

void FlexDiskListCtrl::OnInsertItem(wxListEvent &)
{
}

void FlexDiskListCtrl::OnDeleteItem(wxListEvent &event)
{
    // delete the FlexDirEntry * object
    // stored in m_data
    auto dirEntry = reinterpret_cast<FlexDirEntry *>(event.GetData());
    delete dirEntry;
}

void FlexDiskListCtrl::BeforeDeleteAllItems()
{
    // delete the FlexDirEntry * object
    // stored in m_data
    int item = -1;

    do
    {
        item = GetNextItem(item);

        if (item >= 0)
        {
            auto dirEntry = reinterpret_cast<FlexDirEntry *>(GetItemData(item));
            delete dirEntry;
            SetItemData(item, 0);
        }
    } while (item >= 0);
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

FileNames FlexDiskListCtrl::GetSelectedFileNames()
{
    FileNames fileList;

    for (auto item : GetSelections())
    {
        std::string fileName(GetItemText(item).ToUTF8());
#ifdef UNIX
        std::transform(fileName.begin(), fileName.end(), fileName.begin(),
                       ::tolower);
#endif
        fileList.push_back(fileName);
    }

    return fileList;
}

void FlexDiskListCtrl::SelectAllFiles()
{
    int i;

    for (i = GetItemCount() - 1; i >= 0; i--)
    {
        SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void FlexDiskListCtrl::DeselectAllFiles()
{
    int i;

    for (i = GetItemCount() - 1; i >= 0; i--)
    {
        SetItemState(i, 0, wxLIST_STATE_SELECTED);
    }
}

void FlexDiskListCtrl::FindFiles()
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
        std::string filePattern(value.ToUTF8());
        FileContainerIterator it(filePattern.c_str());
        std::string sFileName;

        DeselectAllFiles();

        for (it = m_container->begin(); it != m_container->end(); ++it)
        {
            int i;

            sFileName = (*it).GetTotalFileName();
            wxString fileName(sFileName.c_str(), wxConvUTF8);

            if (multimatches(sFileName.c_str(), filePattern.c_str(),
                             ';', true) &&
                (i = FindItem(-1, fileName)) >= 0)
            {
                SetItemState(i, wxLIST_STATE_SELECTED,
                             wxLIST_STATE_SELECTED);
            }
        }
    }
}

void FlexDiskListCtrl::CopyToClipboard()
{
    FlexDnDFiles files;
    int count = 0;
    FlexFileDataObject *pClipboardData;

    wxClipboardLocker locker; // implicit open/close wxTheClipboard

    if (!locker)
    {
        wxLogError(_("Can't open clipboard."));
        wxBell();
        return;
    }

    for (auto fileName : GetSelectedFileNames())
    {
        try
        {
            files.Add(m_container->ReadToBuffer(fileName.c_str()));
            count++;
        }
        catch (FlexException &ex)
        {
            wxMessageBox(ex.what(), _("FLEXplorer Error"),
                         wxOK | wxCENTRE | wxICON_EXCLAMATION, this);
        }
    }

    pClipboardData = new FlexFileDataObject();
    pClipboardData->ReadDataFrom(files);

    if (!wxTheClipboard->SetData(pClipboardData))
    {
        wxLogError(_("Can't copy data to the clipboard"));
        wxBell();
    }
}

bool FlexDiskListCtrl::PasteFromClipboard()
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

    flexFileData.WriteDataTo(files);

    result = PasteFrom(files);

    return result;
}

bool FlexDiskListCtrl::PasteFrom(FlexDnDFiles &files)
{
    const char *fileName;
    unsigned int index;
    FlexDirEntry *dirEntry;

    for (index = 0; index < files.GetFileCount(); ++index)
    {
        try
        {
            fileName = files.GetBufferAt(index).GetFilename();
            GetContainer()->WriteFromBuffer(files.GetBufferAt(index));
            dirEntry = new FlexDirEntry;

            if (GetContainer()->FindFile(fileName, *dirEntry))
            {
                wxListItem anItem;

                wxString text(dirEntry->GetTotalFileName().c_str(), wxConvUTF8);
                anItem.m_text   = text;
                anItem.m_itemId = 0;
                anItem.m_col    = 0;
                anItem.m_data   = reinterpret_cast<wxUIntPtr>(dirEntry);
                anItem.m_mask   = wxLIST_MASK_TEXT | wxLIST_MASK_DATA;
                auto itemIndex = InsertItem(anItem);
                UpdateItem(itemIndex, *dirEntry);
            }
            else
            {
                delete dirEntry;
            }
        }
        catch (FlexException &ex)
        {
            if (ex.GetErrorCode() == FERR_DISK_FULL_WRITING)
            {
                wxMessageBox(ex.what(), _("FLEXplorer Error"),
                             wxOK | wxCENTRE | wxICON_EXCLAMATION, this);
                return false;
            }
            else
            {
                wxMessageBox(ex.what(), _("FLEXplorer Error"),
                             wxOK | wxCENTRE, this);
            }
        }
    }

    return true;
}

wxString FlexDiskListCtrl::GetFileDescription(const FlexDirEntry &dirEntry)
{
    wxString        tmp;
    char            **pFDesc;

    pFDesc = (char **)fileDescription;
    wxString extension(dirEntry.GetFileExt().c_str(), wxConvUTF8);

    while (*pFDesc != nullptr)
    {
        tmp = *pFDesc;

        if (tmp.CmpNoCase(extension) == 0)
        {
            tmp = wxGetTranslation(*(++pFDesc));
            return tmp;
        }

        pFDesc += 2;
    }

    extension += " ";
    extension += _("file");
    return extension;
}

