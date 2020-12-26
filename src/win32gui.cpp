/*
    win32gui.cpp  user interface for Win32


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2020  W. Schwotzer

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


#ifdef _WIN32
#include "misc1.h"
#include <ctype.h>
#include <new>
#include <math.h>
#include <richedit.h>
#include <sstream>
#ifdef _MSC_VER
    #include <crtdbg.h>
#endif

#include "resource.h"
#include "win32gui.h"
#include "inout.h"
#include "vico1.h"
#include "vico2.h"
#include "schedule.h"
#include "csetfreq.h"
#include "clogfile.h"
#include "mc6809.h"
#include "mc6809st.h"
#include "joystick.h"
#include "keyboard.h"
#include "terminal.h"
#include "pia1.h"
#include "winctxt.h"
#include "cacttrns.h"


#define  TIMER_UPDATE       (20)  // update rate in ms
#define STATUSBAR_HEIGHT    (28)
#define SBAR_ICON_WIDTH    (STATUSBAR_HEIGHT - 6)
#define SBAR_ICON_HEIGHT   (STATUSBAR_HEIGHT - 6)
#define CHECK_TOP               (1)
#define CHECK_BOTTOM            (2)
#define CHECK_LEFT         (4)
#define CHECK_RIGHT        (8)

static Win32Gui *ggui = nullptr;

extern INT_PTR CALLBACK cpuWindowWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

// process window functions
// ATTENTION: all functions must be reentrant!

LRESULT CALLBACK e2windowWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (ggui == nullptr)
    {
        return FALSE;
    }

    switch (message)
    {
        case WM_PAINT:
            ggui->onPaint(hwnd);
            break;

        // must call DefWindowProc here, don't know why?
        //return DefWindowProc(hwnd, message,
        //  wParam, lParam);
        case WM_COMMAND:
            ggui->onCommand(hwnd, LOWORD(wParam), (HWND)lParam);
            break;

        case WM_CHAR:
            ggui->onChar(hwnd,
                         (SWord)LOBYTE(wParam), (int)LOWORD(lParam));
            break;

        case WM_KEYDOWN:
            ggui->onKeyDown(hwnd,
                            (SWord)wParam, (int)LOWORD(lParam));
            break;

        case WM_TIMER:
            ggui->onTimer(hwnd, (UINT)wParam);
            break;

        case WM_SIZE:
            return ggui->onSize(hwnd, (int)wParam,
                                (int)LOWORD(lParam), (int)HIWORD(lParam));

        case WM_DESTROY:
            ggui->onDestroy(hwnd);
            break;

        case WM_CLOSE:
            ggui->onClose(hwnd);
            break;

        case WM_ACTIVATE:
            ggui->onActivate(hwnd, (WORD)LOWORD(wParam), (HWND)lParam);
            break;

        case WM_SETFOCUS:
            ggui->onSetFocus(hwnd, (HWND)wParam);
            break;

        case WM_KILLFOCUS:
            ggui->onKillFocus(hwnd, (HWND)wParam);
            break;

        case WM_GETMINMAXINFO:
            return ggui->onMinMaxInfo((MINMAXINFO *)lParam);

        case WM_PALETTECHANGED:
            ggui->onPaletteChanged(hwnd);
            break;

        case WM_MOUSEMOVE:
            ggui->onMouseMove(hwnd, static_cast<Word>(LOWORD(lParam)),
                HIWORD(lParam), static_cast<Word>(wParam));
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    } // switch

    return TRUE;
} // e2windowWndProc

void Win32Gui::resetMouseMoveCoords()
{
    joystickIO.put_values(0, 0);
}

void Win32Gui::onTimer(HWND hwnd, UINT id)
{
    if (id == idTimer)
    {
        unsigned int newButtonMask;
        short display_block;
        HDC hdc;
        static int count = 0;

        // check every 100 ms for
        // - CPU view update
        // - Disk status update
        // - interrupt info update
        if (++count % (100 / TIMER_UPDATE) == 0)
        {
            count = 0;

            // Check for disk status update every 200 ms
            static DiskStatus status[4];
            DiskStatus newStatus[4];
            static tInterruptStatus irqStat;
            tInterruptStatus newIrqStat;
            static bool firstTime = true;
            static bool lastState[INT_RESET + 1];
            bool bState;
            Word t;

            inout.get_drive_status(newStatus);

            for (t = 0; t < 4; ++t)
            {
                if ((newStatus[t] != status[t]) || firstTime)
                {
                    update_disk_status(t, newStatus[t]);
                    status[t] = newStatus[t];
                }
            }

            scheduler.get_interrupt_status(newIrqStat);

            if (firstTime)
            {
                for (t = INT_IRQ; t <= INT_RESET; ++t)
                {
                    lastState[t] = false;
                    irqStat.count[t] = 0;
                    update_interrupt_status((tIrqType)t, false);
                }

                firstTime = false;
            }
            else
            {
                for (t = INT_IRQ; t <= INT_RESET; ++t)
                {
                    bState = (newIrqStat.count[t] != irqStat.count[t]);

                    if (bState != lastState[t])
                    {
                        update_interrupt_status((tIrqType)t, bState);
                        lastState[t] = bState;
                    }
                }
            }

            memcpy(&irqStat, &newIrqStat, sizeof(tInterruptStatus));
        }

        // check if CPU view has to be updated
        Mc6809CpuStatus *status = (Mc6809CpuStatus *)scheduler.get_status();
        if (status != nullptr)
        {
            bool is_running = (status->state == CpuState::Run ||
                               status->state == CpuState::Next);
            UINT run_checked = is_running ? MF_CHECKED : MF_UNCHECKED;
            UINT stop_checked = !is_running ? MF_CHECKED : MF_UNCHECKED;

            CheckMenuItem(menu2, IDM_RUN, MF_BYCOMMAND | run_checked);
            CheckMenuItem(menu2, IDM_STOP, MF_BYCOMMAND | stop_checked);
            update_cpuview(*status);

            if (status->state == CpuState::Invalid)
            {
                char err_msg[128];

                sprintf(err_msg, "\
Got invalid instruction\n\
pc=%04x instr=%02x %02x %02x %02x\n\
Processor stopped. To\n\
continue press Reset button",
                        status->pc,
                        status->instruction[0],
                        status->instruction[1],
                        status->instruction[2],
                        status->instruction[3]);
                popup_message(err_msg);
            }
        }

        // update graphic display (only if display memory has changed)
        hdc = GetDC(hwnd);

        for (display_block = 0; display_block < YBLOCKS; display_block++)
        {
            if (memory.has_changed(display_block))
            {
                update_block(display_block, hdc);
            }
        }

        ReleaseDC(hwnd, hdc);

        if (GetActiveWindow() == e2screen)
        {
            newButtonMask = 0;

            if (IsLButtonDown())
            {
                newButtonMask |= L_MB;
            }

            if (IsMButtonDown())
            {
                newButtonMask |= M_MB;
            }

            if (IsRButtonDown())
            {
                newButtonMask |= R_MB;
            }

            if (GetKeyState(VK_CONTROL) < 0)
            {
                newButtonMask |= CONTROL_KEY;
            }

            if (GetKeyState(VK_SHIFT) < 0)
            {
                newButtonMask |= SHIFT_KEY;
            }

            joystickIO.put_value(newButtonMask);
            mouse_update(e2screen);
        }

        // check if program can be savely shut down
        if (scheduler.is_finished())
        {
            CloseApp(hwnd);
        }
    }
} // onTimer

void Win32Gui::mouse_update(HWND w)
{
    int dx  = 0;
    int dy  = 0;

    if ((prev_x != -1) && (current_x != -1) &&
        (prev_y != -1) && (current_y != -1))
    {
        if (cursor_type == FLX_INVISIBLE_CURSOR)
        {
            dx = current_x - prev_x - warp_dx;
            dy = current_y - prev_y - warp_dy;
            mouse_warp(w, warp_home_x - current_x,
                       warp_home_y - current_y);
        }

        joystickIO.put_values(dx, dy);
        prev_x = current_x;
        prev_y = current_y;
    }
    else if ((current_x != -1) && (current_y != -1))
    {
        prev_x = current_x;
        prev_y = current_y;
    }
    else
    {
        prev_x = current_x = -1;
        prev_y = current_y = -1;
    }

    //joystickIO.put_value(convert_buttonmask(mouse_button_state));
}

void Win32Gui::mouse_warp(HWND w, int dx, int dy)
{
    if (warp_dx || warp_dy || dx || dy)
    {
        POINT point;

        point.x = warp_dx = dx;
        point.y = warp_dy = dy;
        // unfinished: on NT/2K/XP use SendInput instead
        // Don't know why coordinated have to be divided by 2
        //mouse_event(MOUSEEVENTF_MOVE, dx >> 1, dy >> 1, 0, nullptr);
        point.x = current_x + dx;
        point.y = current_y + dy;
        ClientToScreen(w, &point);
        SetCursorPos(point.x, point.y);
    }
}

const char *Win32Gui::get_title()
{
    if (cursor_type == FLX_DEFAULT_CURSOR)
    {
        return PROGRAMNAME " V" PROGRAM_VERSION
               " - Press CTRL F1 to capture mouse";
    }
    else
    {
        return PROGRAMNAME " V" PROGRAM_VERSION
               " - Press CTRL F1 to release mouse";
    }
}

void Win32Gui::toggle_mouse_capture(HWND w)
{
    cursor_type = (cursor_type == FLX_DEFAULT_CURSOR) ?
                  FLX_INVISIBLE_CURSOR : FLX_DEFAULT_CURSOR;

    const char *title = get_title();

    SetWindowText(w, title);

    if (cursor_type == FLX_DEFAULT_CURSOR)
    {
        ReleaseCapture();
    }
    else
    {
        SetCapture(e2screen);
    }

    set_cursor(cursor_type);
}

void Win32Gui::set_cursor(int type /* = FLX_DEFAULT_CURSOR */)
{
    ShowCursor(type == FLX_DEFAULT_CURSOR);
}

void Win32Gui::release_mouse_capture(HWND w)
{
    if (cursor_type == FLX_INVISIBLE_CURSOR)
    {
        toggle_mouse_capture(w);
    }
}

void Win32Gui::onSetFocus(HWND, HWND
    /* [[maybe_unused]] HWND hwnd, [[maybe_unused]] HWND oldHwnd */)
{
}

void Win32Gui::onKillFocus(HWND hwnd, HWND /* [[maybe_unused]] HWND newHwnd */)
{
    release_mouse_capture(hwnd);
}

void Win32Gui::onCommand(HWND hwndWindow, int cmd, HWND hwndControl)
{
    switch (cmd)
    {
        // Main Window menu
        case IDM_EXIT:
            CloseApp(hwndWindow, true);
            break;

        case IDM_RUN:
            request_new_state(CpuState::Run);
            break;

        case IDM_STOP:
            request_new_state(CpuState::Stop);
            break;

        case IDM_RESET:
            request_new_state(CpuState::ResetRun);
            break;

        case IDM_VIEW:
            toggle_cpu();
            break;

        case IDM_BREAKPOINTS:
            popup_bp(hwndWindow);
            break;

        case IDM_LOGFILE:
            popup_log(hwndWindow);
            break;

        case IDM_FREQUENCY0:
            toggle_freqency();
            break;

        case IDM_UNDOCUMENTED:
            toggle_undocumented();
            break;

        case IDM_ABOUT:
            popup_about(hwndWindow);
            break;

        case IDM_COPYRIGHT:
            popup_copyright(hwndWindow);
            break;

        case IDM_INTRODUCTION:
            popup_help(hwndWindow);
            break;

        case IDP_FLOPPY0:
        case IDP_FLOPPY1:
        case IDP_FLOPPY2:
        case IDP_FLOPPY3:
            popup_disk_info(hwndControl);
            break;

        case IDP_IRQ:
            popup_interrupt_info();
            break;
    }

    return;
} // onCommand

void Win32Gui::onChar(HWND /* [[maybe_unused]] HWND hwnd */,
    SWord ch,
    int /* [[maybe_unused]] int repeat */)
{
    SWord key;

    if ((key = translate_to_ascii(ch)) >= 0)
    {
        bool do_notify = false;

        keyboardIO.put_char_parallel((Byte)key, do_notify);
        if (do_notify)
        {
            auto command = BCommandPtr(
                    new CActiveTransition(pia1, Mc6821::ControlLine::CA1));
            scheduler.sync_exec(std::move(command));
        }
    }

} // onChar

void Win32Gui::onKeyDown(HWND /* [[maybe_unused]] HWND hwnd */,
    SWord ch,
    int /* [[maybe_unused]] int repeat */)
{
    SWord key;

    if ((key = translate_to_ascii1(ch)) >= 0)
    {
        bool do_notify = false;

        keyboardIO.put_char_parallel((Byte)key, do_notify);
        if (do_notify)
        {
            auto command = BCommandPtr(
                new CActiveTransition(pia1, Mc6821::ControlLine::CA1));
            scheduler.sync_exec(std::move(command));
        }
    }

} // onKeyDown

void Win32Gui::onPaletteChanged(HWND /* [[maybe_unused]] HWND hwnd */)
{
    HDC hdc;

    hdc = GetDC(e2screen);
    UpdateColors(hdc);
    ReleaseDC(e2screen, hdc);
} // onPaletteChanged

void Win32Gui::onMouseMove(HWND /* [[maybe_unused]] HWND hwnd */,
    Word newX, Word newY,
    Word /* [[maybe_unused]] Word fwKeys */)
{
    current_x = newX;
    current_y = newY;
}

void Win32Gui::update_disk_status(int floppyIndex, DiskStatus status)
{
    int     iconIdx = IDI_FLOPPY0;
    HICON       hIcon;

    switch (status)
    {
        case DiskStatus::EMPTY:
            iconIdx += 0;
            break;

        case DiskStatus::INACTIVE:
            iconIdx += 1;
            break;

        case DiskStatus::ACTIVE:
            iconIdx += 2;
            break;
    }

    hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(iconIdx),
                             IMAGE_ICON, 16, 16, LR_SHARED);

    if (hIcon != nullptr)
    {
        SendMessage(hButtonFloppy[floppyIndex], BM_SETIMAGE, IMAGE_ICON,
                    (LPARAM)hIcon);
    }
}

void Win32Gui::update_interrupt_status(tIrqType irqType, bool status)
{
    int     iconIdx = IDI_IRQ0;
    HICON       hIcon;

    if (status)
    {
        switch (irqType)
        {
            case INT_IRQ:
                iconIdx += 1;
                break;

            case INT_FIRQ:
                iconIdx += 2;
                break;

            case INT_NMI:
                iconIdx += 3;
                break;

            case INT_RESET:
                iconIdx += 4;
                break;
        }
    }

    hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(iconIdx),
                             IMAGE_ICON, 16, 16, LR_SHARED);

    if (hIcon != nullptr)
    {
        SendMessage(hButtonIrq, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
    }
}

void Win32Gui::onPaint(HWND hwnd)
{
    HDC         hdc;
    PAINTSTRUCT ps;

    // On Windows Request to update a part or the whole Client rect
    // Always update everything
    hdc = BeginPaint(hwnd, &ps);

    if (hwnd == e2screen)
    {
        int display_block;

        memory.init_blocks_to_update();

        for (display_block = 0; display_block < YBLOCKS; display_block++)
        {
            update_block(display_block, hdc);
        }
    }
    else if (hwnd == hwndStatus)
    {
        RECT rect;
        HBRUSH hPenNew, hPenOld;
        WORD i, count;

        i = 1;
        GetClientRect(hwnd, &rect);
        // draw background of status bar
        FillRect(hdc, &rect, GetSysColorBrush(COLOR_INACTIVEBORDER));
        // Draw sunken border
        hPenNew = (HBRUSH)CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
        hPenOld = (HBRUSH)SelectObject(hdc, hPenNew);
        MoveToEx(hdc, rect.left + i, rect.bottom - i, nullptr);
        LineTo(hdc, rect.left + i, rect.top + i);
        LineTo(hdc, rect.right - i, rect.top + i);
        SelectObject(hdc, hPenOld);
        DeleteObject(hPenNew);
        hPenNew = (HBRUSH)CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
        hPenOld = (HBRUSH)SelectObject(hdc, hPenNew);
        MoveToEx(hdc, rect.left + i, rect.bottom - i, nullptr);
        LineTo(hdc, rect.right - i - 16, rect.bottom - i);
        MoveToEx(hdc, rect.right - i, rect.bottom - i - 16, nullptr);
        LineTo(hdc, rect.right - i, rect.top + i);
        SelectObject(hdc, hPenOld);
        DeleteObject(hPenNew);

        // Draw the "resize corner"
        hPenNew = (HBRUSH)CreatePen(PS_SOLID, 2, GetSysColor(COLOR_3DSHADOW));
        hPenOld = (HBRUSH)SelectObject(hdc, hPenNew);
        i = 4;

        for (count = 0; count < 4; count++)
        {
            MoveToEx(hdc, rect.right - i, rect.bottom, nullptr);
            LineTo(hdc, rect.right, rect.bottom - i);
            i += 4;
        }

        SelectObject(hdc, hPenOld);
        DeleteObject(hPenNew);

        hPenNew = (HBRUSH)CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
        hPenOld = (HBRUSH)SelectObject(hdc, hPenNew);
        i = 5;

        for (count = 0; count < 4; count++)
        {
            MoveToEx(hdc, rect.right - i, rect.bottom, nullptr);
            LineTo(hdc, rect.right, rect.bottom - i);
            i += 4;
        }

        SelectObject(hdc, hPenOld);
        DeleteObject(hPenNew);
    }

    EndPaint(hwnd, &ps);
}

int Win32Gui::onSize(HWND hwnd, int sizeType, int width, int height)
{
    RECT            oldRect, rect;
    int         i;

    if (sizeType == SIZE_RESTORED && hwnd == e2screen)
    {
        if (width % WINDOWWIDTH != 0 || height % WINDOWHEIGHT != 0)
        {
            pixelSizeX = (width + (WINDOWWIDTH / 2)) /  WINDOWWIDTH;
            pixelSizeY = (height + (WINDOWHEIGHT / 2)) /  WINDOWHEIGHT;
            GetWindowRect(e2screen, &oldRect);
            rect.left       = (long)0;
            rect.top        = (long)0;
            rect.right      = (long)WINDOWWIDTH * pixelSizeX;
            rect.bottom     = (long)WINDOWHEIGHT * pixelSizeY + STATUSBAR_HEIGHT;

            if (AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, TRUE, 0))
            {
                width  = rect.right - rect.left;
                height = rect.bottom - rect.top;
            };

            MoveWindow(e2screen, oldRect.left, oldRect.top,
                       width, height, TRUE);

            MoveWindow(hwndStatus, 0, WINDOWHEIGHT * pixelSizeY,
                       WINDOWWIDTH * pixelSizeX, STATUSBAR_HEIGHT, TRUE);

            for (i = 0; i < 4; ++i)
            {
                MoveWindow(hButtonFloppy[i],
                           WINDOWWIDTH * pixelSizeX - ((6 - i) * SBAR_ICON_WIDTH),
                           4,
                           SBAR_ICON_WIDTH,
                           SBAR_ICON_HEIGHT,
                           TRUE);
            }

            MoveWindow(hButtonIrq,
                       WINDOWWIDTH * pixelSizeX - (2 * SBAR_ICON_WIDTH),
                       4,
                       SBAR_ICON_WIDTH,
                       SBAR_ICON_HEIGHT,
                       TRUE);
            warp_home_x = (pixelSizeX * WINDOWWIDTH)  >> 1;
            warp_home_y = (pixelSizeY * WINDOWHEIGHT) >> 1;
        }

        return 1;
    }

    return  0;
} // onSize

// Savely close the Application
bool Win32Gui::CloseApp(HWND hwnd, bool confirm /* = false */)
{
    if (confirm)
    {
        if (MessageBox(e2screen,
                       "Do you really want to close " PROGRAMNAME,
                       PROGRAMNAME, MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
        {
            return false;
        }
    }

    release_mouse_capture(hwnd);
    request_new_state(CpuState::Exit);
    KillTimer(hwnd, idTimer);
    idTimer = 0;

    if (menubar != nullptr)
    {
        DestroyMenu(menubar);
    }

    if (hwnd != nullptr)
    {
        DestroyWindow(hwnd);
    }

    return true;
} // CloseApp


void Win32Gui::onClose(HWND hwnd)
{
    CloseApp(hwnd, true);
} // onClose

void Win32Gui::onActivate(HWND /* [[maybe_unused]] HWND hwnd */,
    WORD what,
    HWND /* [[maybe_unused]] HWND hwnd1 */)
{
    if (what == WA_ACTIVE || what == WA_CLICKACTIVE)
    {
        memory.init_blocks_to_update();
    }
}

void Win32Gui::onDestroy(HWND)
{
    PostQuitMessage(0);
} // onDestroy

int Win32Gui::onMinMaxInfo(MINMAXINFO *lpmmi)
{
    RECT    rect;

    rect.left       = (long)0;
    rect.top        = (long)0;
    rect.right      = (long)WINDOWWIDTH;
    rect.bottom     = (long)WINDOWHEIGHT + STATUSBAR_HEIGHT;

    if (AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, TRUE, 0))
    {
        lpmmi->ptMinTrackSize.x = rect.right - rect.left;
        lpmmi->ptMinTrackSize.y = rect.bottom - rect.top;
    };

    rect.right      = (long)WINDOWWIDTH * MAX_PIXELSIZEX;

    rect.bottom     = (long)WINDOWHEIGHT * MAX_PIXELSIZEY + STATUSBAR_HEIGHT;

    if (AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, TRUE, 0))
    {
        lpmmi->ptMaxTrackSize.x = rect.right - rect.left;
        lpmmi->ptMaxTrackSize.y = rect.bottom - rect.top;
    };

    return  0;
} // onMinMaxInfo

SWord Win32Gui::translate_to_ascii(SWord key)
{
    if (!(key & 0xFF80))
    {
        return key;
    }

    return -1;
} // translate_to_ascii

SWord Win32Gui::translate_to_ascii1(SWord key)
{
    if ((GetKeyState(VK_CONTROL) < 0) &&
        (GetKeyState(VK_SHIFT) < 0))
    {
        switch (key)
        {
            case VK_PAUSE:
            case VK_CANCEL:
                request_new_state(CpuState::ResetRun);

            case VK_HOME:
                return 0xa1;

            case VK_UP:
                return 0xa8;

            case VK_PRIOR:
                return 0xa7;

            case VK_LEFT:
                return 0xa4;

            case VK_CLEAR:
                return 0xa5;

            case VK_RIGHT:
                return 0xa6;

            case VK_END:
                return 0xa9;

            case VK_DOWN:
                return 0xa2;

            case VK_NEXT:
                return 0xa3;
        }
    }

    if (GetKeyState(VK_CONTROL) < 0)
    {
        switch (key)
        {
            case VK_PAUSE:
            case VK_CANCEL:
                cpu.set_nmi();

                return -1;

            case VK_HOME:
                return 0xb1;

            case VK_UP:
                return 0xb8;

            case VK_PRIOR:
                return 0xb7;

            case VK_LEFT:
                return 0xb4;

            case VK_CLEAR:
                return 0xb5;

            case VK_RIGHT:
                return 0xb6;

            case VK_END:
                return 0xb9;

            case VK_DOWN:
                return 0xb2;

            case VK_NEXT:
                return 0xb3;

            case VK_F1:
                toggle_mouse_capture(e2screen);
                return -1;

            case VK_F11:
                return 0xfb; // PAT09: RIGHT MOST

            case VK_F12:
                return 0x92; // PAT09: LEFT  MOST

            case VK_DELETE:
                return 0x1f;

            default:
                return -1;
        }
    }

    if (GetKeyState(VK_SHIFT) < 0)
    {
        switch (key)
        {
            case VK_F1:
                return 0xca; // PAT09: F11

            case VK_F2:
                return 0xcb;

            case VK_F3:
                return 0xcc;

            case VK_F4:
                return 0xcd;

            case VK_F5:
                return 0xce;

            case VK_F6:
                return 0xcf;

            case VK_F7:
                return 0xd0;

            case VK_F8:
                return 0xd1;

            case VK_F9:
                return 0xd2; // PAT09: F19

            case VK_F11:
                return 0xea; // PAT09: RIGHT MOST

            case VK_F12:
                return 0x81; // PAT09: LEFT  MOST

            case VK_PAUSE:
            case VK_CANCEL:
                cpu.set_nmi();

                return -1;

            case VK_HOME:
                return 0xe1;

            case VK_UP:
                return 0xe8;

            case VK_PRIOR:
                return 0xe7;

            case VK_LEFT:
                return 0xe4;

            case VK_CLEAR:
                return 0xe5;

            case VK_RIGHT:
                return 0xe6;

            case VK_END:
                return 0xe9;

            case VK_DOWN:
                return 0xe2;

            case VK_NEXT:
                return 0xe3;

            case VK_DELETE:
                return 0x7f;

            default:
                return -1;
        }
    }

    switch (key)
    {
        case VK_F1:
            return 0xc0;

        case VK_F2:
            return 0xc1;

        case VK_F3:
            return 0xc2;

        case VK_F4:
            return 0xc3;

        case VK_F5:
            return 0xc4;

        case VK_F6:
            return 0xc5;

        case VK_F7:
            return 0xc6;

        case VK_F8:
            return 0xc7;

        case VK_F9:
            return 0xc8;

        case VK_F10:
            return 0xc9;

        case VK_F11:
            return 0xfa; // PAT09: RIGHT MOST

        case VK_F12:
            return 0x91; // PAT09: LEFT  MOST

        case VK_HOME:
            return 0xf1;

        case VK_UP:
            return 0xf8;

        case VK_PRIOR:
            return 0xf7;

        case VK_LEFT:
            return 0xf4;

        case VK_CLEAR:
            return 0xf5;

        case VK_RIGHT:
            return 0xf6;

        case VK_END:
            return 0xf9;

        case VK_DOWN:
            return 0xf2;

        case VK_NEXT:
            return 0xf3;

        case VK_DELETE:
            return 0x7f;

        default:
            return -1;
    }
}

void Win32Gui::initialize(struct sGuiOptions &guiOptions)
{
    AbstractGui::initialize(guiOptions);
    ggui        = this;
    palette     = nullptr; // needed for color display
    e2screen    = nullptr;
    withColorScale  = !stricmp(options.color.c_str(), "default");
    nColors     = options.nColors;
    bp_input[0] = 0;
    bp_input[1] = 0;
    lfs.reset();
    image_data = nullptr;
    frequency_control_on = false;
    cursor_type   = FLX_DEFAULT_CURSOR;
    warp_dx       = 0;
    warp_dy       = 0;
    warp_home_x   = 0;
    warp_home_y   = 0;
    prev_x        = -1;
    prev_y        = -1;
    current_x     = -1;
    current_y     = -1;
    initialize_conv_tables();
    initialize_e2window(guiOptions);
} // initialize

void Win32Gui::popup_disk_info(HWND hwnd)
{
    std::string message;
    Word drive_nr;

    for (drive_nr = 0; drive_nr < 4; ++drive_nr)
    {
        if (hwnd == hButtonFloppy[drive_nr])
        {
            message = inout.get_drive_info(drive_nr);
            MessageBox(e2screen, message.c_str(),
                       PROGRAMNAME " Disc status",
                       MB_OK | MB_ICONINFORMATION);
            return;
        }
    }
}

void Win32Gui::popup_interrupt_info()
{
    std::stringstream message;
    tInterruptStatus s;

    scheduler.get_interrupt_status(s);
    message << "IRQ:   " << s.count[INT_IRQ] << std::endl
            << "FIRQ:  " << s.count[INT_FIRQ] << std::endl
            << "NMI:   " << s.count[INT_NMI] << std::endl
            << "RESET: " << s.count[INT_RESET] << std::endl;
    MessageBox(e2screen, message.str().c_str(),
               PROGRAMNAME " Interrupt status",
               MB_OK | MB_ICONINFORMATION);
}

// return 0 on success
int Win32Gui::popup_help(HWND hwnd)
{
    char helpfile[PATH_MAX];
    char curdir[PATH_MAX];
    HINSTANCE res;

    GetCurrentDirectory(PATH_MAX, curdir);

    if (strlen(options.doc_dir.c_str()) != 0)
    {
        strcpy(helpfile, options.doc_dir.c_str());
    }
    else
    {
        strcpy(helpfile, curdir);
        strcat(helpfile, "\\doc\\");
    }

    if (helpfile[strlen(helpfile) - 1] != PATHSEPARATOR)
    {
        strcat(helpfile, PATHSEPARATORSTRING);
    }

    strcat(helpfile, "flexemu.htm");

    res = ShellExecute(
        hwnd,           // handle of parent window
        "open",         // open file
        helpfile,       // file to open
        nullptr,           // parameters
        curdir,         // directory
        SW_SHOWNORMAL   // openmode for new window
    );
    if ((INT_PTR)res > 32)
    {
        return 0;    // success
    }

    return -1; // no success
}

void Win32Gui::popup_message(char *pmessage)
{
    if (*pmessage)
        MessageBox(e2screen, pmessage, PROGRAMNAME " error",
                   MB_OK | MB_ICONERROR);
}

void Win32Gui::main_loop()
{
    MSG msg;
    BOOL bRet;

    while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            break;
        }

        if (!IsWindow(cpuform) || !IsDialogMessage(cpuform, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } // while
}

void Win32Gui::set_bell(int /* [[maybe_unused]] int percent */)
{
    // volume in percent is ignored
    Beep(400, 200);
} // set_bell

void Win32Gui::update_block(int block_number, HDC hdc)
{
    HDC   hMemoryDC;
    HBITMAP hBitmapOrig;
    int     firstpartHeight; // Height of first part of divided block
    int     startLine;       // start scanline of block
    HBITMAP img;
    HPALETTE oldPalette;
    size_t bmi_index;

    if (!memory.has_changed(block_number))
    {
        return;
    }

    memory.reset_changed(block_number);
    hMemoryDC = CreateCompatibleDC(nullptr);
    oldPalette = SelectPalette(hdc, palette, TRUE);
    RealizePalette(hdc);
    img = image[pixelSizeX - 1][pixelSizeY - 1];
    bmi_index = (pixelSizeX - 1) * MAX_PIXELSIZEY + (pixelSizeY - 1);

    if (memory.is_video_bank_valid(vico1.get_value()))
    {
        // copy block from video ram into device independant bitmap
        Byte const *src =
            memory.get_video_ram(vico1.get_value(), block_number);

        CopyToZPixmap(image_data.get(), src, 8);

        SetDIBits(
            hdc, img,
            0, BLOCKHEIGHT * pixelSizeY,
            image_data.get(),
            (const BITMAPINFO *)bmis[bmi_index].get(),
            DIB_PAL_COLORS);
        hBitmapOrig = SelectBitmap(hMemoryDC, img);
        startLine = ((WINDOWHEIGHT - vico2.get_value() +
                      block_number * BLOCKHEIGHT) % WINDOWHEIGHT) * pixelSizeY;

        auto divided_block = get_divided_block();
        if (divided_block >= 0 && (divided_block == block_number))
        {
            firstpartHeight = vico2.get_value() % BLOCKHEIGHT;
            // first half display on the bottom of the window
            BitBlt(hdc, 0, startLine,
                   BLOCKWIDTH * pixelSizeX,
                   firstpartHeight * pixelSizeY,
                   hMemoryDC, 0, 0,
                   SRCCOPY);
            // second half display on the top of window
            BitBlt(hdc, 0, 0,
                   BLOCKWIDTH * pixelSizeX,
                   (BLOCKHEIGHT - firstpartHeight) * pixelSizeY,
                   hMemoryDC, 0, firstpartHeight * pixelSizeY,
                   SRCCOPY);
            // first half display on the bottom of the window
        }
        else
        {
            BitBlt(hdc, 0, startLine,
                   BLOCKWIDTH * pixelSizeX, BLOCKHEIGHT * pixelSizeY,
                   hMemoryDC, 0, 0, SRCCOPY);
        }

        SelectBitmap(hMemoryDC, hBitmapOrig);
    }
    else
    {
        // Invalid video bank selected: Always display an empty screen.
        CopyToZPixmap(image_data.get(), nullptr, 8);
        SetDIBits(
            hdc, img,
            0, BLOCKHEIGHT * pixelSizeY,
            image_data.get(),
            (const BITMAPINFO *)bmis[bmi_index].get(),
            DIB_PAL_COLORS);
        hBitmapOrig = SelectBitmap(hMemoryDC, img);
        BitBlt(hdc, 0,
               (block_number * BLOCKHEIGHT) * pixelSizeY,
               BLOCKWIDTH * pixelSizeX,
               BLOCKHEIGHT * pixelSizeY,
               hMemoryDC, 0, 0, SRCCOPY);
        SelectBitmap(hMemoryDC, hBitmapOrig);
    } // else

    SelectPalette(hdc, oldPalette, FALSE);
    DeleteDC(hMemoryDC);
} // update_block

void Win32Gui::initialize_e2window(struct sGuiOptions &guiOptions)
{
    HWND w;

    if (!registerWindowClasses(winApiContext.hInstance))
    {
        MessageBox(nullptr, "RegisterClassEx failed\n" \
                   "Unable to create Mainwindow of " PROGRAMNAME,
                   PROGRAMNAME " error",
                   MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    if ((w = create_main_view()) == nullptr)
    {
        MessageBox(nullptr, "CreateWindow failed\n" \
                   "Unable to create Mainwindow of " PROGRAMNAME,
                   PROGRAMNAME " error",
                   MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    create_cpuview(w);
    initialize_after_create(w, guiOptions);
    manage_widget(w);
    initialize_after_open(w);
    e2screen = w;
} // initialize_e2window


BOOL Win32Gui::registerWindowClasses(HINSTANCE hinst)
{
    WNDCLASSEX      wcex ;

    // Fill in window class structure with parameters that describe
    // the Main window.
    wcex.cbSize        = sizeof(WNDCLASSEX) ;
    wcex.style         = CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW |
                         CS_DBLCLKS;
    wcex.lpfnWndProc   = e2windowWndProc ;
    wcex.cbClsExtra    = 0 ;
    wcex.cbWndExtra    = 0 ;
    wcex.hInstance     = hinst ;
    wcex.hIcon         = LoadIcon(hinst, MAKEINTRESOURCE(IDI_FLEXEMU)) ;
    wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    // wcex.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
    wcex.lpszMenuName  = nullptr;
    wcex.lpszClassName = "Flexemu";
    wcex.hIconSm       = nullptr ;

    if (!RegisterClassEx(&wcex))
    {
        return FALSE;
    }

    return TRUE;
}

HWND Win32Gui::create_main_view()
{
    HWND hwnd;
    RECT rect;
    int  width, height;
    int  count;
    DWORD style;
    char    label[10];

    MENUITEMINFO menuItem =
    {
        sizeof(MENUITEMINFO),           // size of structure
        MIIM_ID | MIIM_STATE | MIIM_TYPE, // set ID, state and type
        MFT_STRING,                     // menutype (String, Bitmap, Sep. ...)
        MFS_ENABLED,                    // menustate (enabled, checked ...)
        0,                              // ID of menuitem
        nullptr,                           // handle of submenu
        nullptr,                           // bitmap for checked
        nullptr,                           // bitmap for unchecked
        0,                              // user defined data
        "",                             // item string ...
        0
    };                             // stringlength if returned

    hInstance = winApiContext.hInstance;
    menubar = CreateMenu();

    if (menubar != nullptr)
    {
        menu1 = CreateMenu();
        AppendMenu(menubar, MF_POPUP | MF_STRING, (UINT_PTR)menu1, "&File");
        menu2 = CreateMenu();
        AppendMenu(menubar, MF_POPUP | MF_STRING, (UINT_PTR)menu2, "&Processor");
        menu3 = CreateMenu();
        AppendMenu(menubar, MF_POPUP | MF_STRING, (UINT_PTR)menu3, "&Help");

        menuItem.dwTypeData = "&Exit";
        menuItem.wID = IDM_EXIT;
        InsertMenuItem(menu1, 1, TRUE, &menuItem);

        count = 1;
        menuItem.dwTypeData = "&Run";
        menuItem.wID = IDM_RUN;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "&Stop";
        menuItem.wID = IDM_STOP;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "R&eset";
        menuItem.wID = IDM_RESET;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.fType = MFT_SEPARATOR;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "&View...";
        menuItem.fType = MFT_STRING;
        menuItem.wID = IDM_VIEW;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "&Breakpoints...";
        menuItem.fType = MFT_STRING;
        menuItem.wID = IDM_BREAKPOINTS;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "&Logging...";
        menuItem.fType = MFT_STRING;
        menuItem.wID = IDM_LOGFILE;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.fType = MFT_SEPARATOR;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "&Frequency 1.3396 MHz";
        menuItem.fType = MFT_STRING;
        menuItem.wID = IDM_FREQUENCY0;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "&Undocumented Instructions";
        menuItem.fType = MFT_STRING;
        menuItem.wID = IDM_UNDOCUMENTED;
        InsertMenuItem(menu2, count++, TRUE, &menuItem);

        count = 1;
        menuItem.dwTypeData = "&Documentation";
        menuItem.wID = IDM_INTRODUCTION;
        InsertMenuItem(menu3, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "&About " PROGRAMNAME;
        menuItem.wID = IDM_ABOUT;
        InsertMenuItem(menu3, count++, TRUE, &menuItem);
        menuItem.dwTypeData = "&Licence";
        menuItem.wID = IDM_COPYRIGHT;
        InsertMenuItem(menu3, count++, TRUE, &menuItem);
    }

    rect.left    = (long)0;
    rect.right   = (long)WINDOWWIDTH * pixelSizeX;
    rect.top     = (long)0;
    rect.bottom  = (long)WINDOWHEIGHT * pixelSizeY + STATUSBAR_HEIGHT;

    if (AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, TRUE, 0))
    {
        width  = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }
    else
    {
        width  = WINDOWWIDTH + 8;
        height = WINDOWHEIGHT + 24;
    }

    hwnd = CreateWindow(
               PROGRAMNAME,        // Address of registered class name
               PROGRAMNAME,          // Address of window name
               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
               WS_THICKFRAME | WS_MINIMIZEBOX,  // Window style
               CW_USEDEFAULT,      // Horizontal position of window
               CW_USEDEFAULT,      // Vertical position of window
               width,              // Window width
               height,             // Window height
               nullptr,               // Handle of parent or owner window
               menubar,            // Handle of menu for this window
               winApiContext.hInstance,// Handle of application instance
               nullptr) ;             // Address of window-creation data
#ifdef _MSC_VER
    _ASSERT(nullptr != hwnd) ;
#endif

    if (!hwnd)
    {
        return nullptr ;
    }

    width  = WINDOWWIDTH * pixelSizeX;
    height = STATUSBAR_HEIGHT;
    style  = WS_CHILD | WS_VISIBLE;

    hwndStatus = CreateWindow(
                     PROGRAMNAME,        // Address of registered class name
                     PROGRAMNAME,        // Address of window name
                     style,              // Window style
                     0,                  // Horizontal position of window
                     WINDOWHEIGHT * pixelSizeY,// Vertical position of window
                     width,              // Window width
                     height,             // Window height
                     hwnd,               // Handle of parent or owner window
                     nullptr,               // Handle of menu for this window
                     winApiContext.hInstance,// Handle of application instance
                     nullptr) ;             // Address of window-creation data

    rect.top  = 4;
    rect.left = width - (6 * (SBAR_ICON_WIDTH + 2));
    width     = SBAR_ICON_WIDTH;
    height    = SBAR_ICON_HEIGHT;
    style  = WS_CHILD | WS_VISIBLE | BS_ICON;

    for (count = 0; count < 4; count++)
    {
        sprintf(label, "BFLOPPY%d", count);
        UINT_PTR id = IDP_FLOPPY0 + count;
        hButtonFloppy[count] = CreateWindow("BUTTON", label, style,
                                            rect.left, rect.top, width, height,
                                            hwndStatus,
                                            reinterpret_cast<HMENU>(id),
                                            hInstance, nullptr);
        rect.left += SBAR_ICON_WIDTH + 2;
    }

    strcpy(label, "BIRQ");
    hButtonIrq = CreateWindow("BUTTON", label, style,
                              rect.left, rect.top, width, height, hwndStatus,
                              (HMENU)(IDP_IRQ), hInstance, nullptr);
    return hwnd ;
}  // create_main_view


INT_PTR CALLBACK aboutDialogProc(HWND hwnd, UINT message, WPARAM wParam,
                              LPARAM /* [[maybe_unused]] LPARAM lParam */)
{
    if (ggui == nullptr)
    {
        return FALSE;
    }

    switch (message)
    {
        case WM_INITDIALOG:
            return ggui->onAboutInit(hwnd);

        case WM_COMMAND:
            return ggui->onAboutCommand(hwnd, LOWORD(wParam));

        case WM_CLOSE:
            return ggui->onAboutClose(hwnd);
    } // switch

    return FALSE;
} // aboutDialogProc

void Win32Gui::toggle_undocumented()
{
    is_use_undocumented = !is_use_undocumented;
    UINT is_checked = is_use_undocumented ? MF_CHECKED : MF_UNCHECKED;

    cpu.set_use_undocumented(is_use_undocumented);
    CheckMenuItem(menu2, IDM_UNDOCUMENTED, MF_BYCOMMAND | is_checked);
}

void Win32Gui::toggle_freqency()
{
    float frequency;

    frequency_control_on = !frequency_control_on;
    frequency = frequency_control_on ? ORIGINAL_FREQUENCY : 0.0f;
    scheduler.sync_exec(BCommandPtr(new CSetFrequency(scheduler, frequency)));

    update_frequency_check();
}

void Win32Gui::update_frequency_check()
{
    UINT is_checked = frequency_control_on ? MF_CHECKED : MF_UNCHECKED;
    CheckMenuItem(menu2, IDM_FREQUENCY0, MF_BYCOMMAND | is_checked);
}

void Win32Gui::popup_about(HWND hwnd)
{
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_AB_DIALOG), hwnd,
              static_cast<DLGPROC>(aboutDialogProc));
} // popup_about

void Win32Gui::popdown_about(HWND hwnd)
{
    EndDialog(hwnd, 0);
}

void Win32Gui::stripBlanks(char *str)
{
    char *p1, *p2;

    p1 = str;
    p2 = str;

    while (*p2)
    {
        // skip all leading blanks
        while (*p2 == ' ')
        {
            p2++;
        }

        // copy line
        while (*p2 && *p2 != '\n')
        {
            *(p1++) = *(p2++);
        }

        *(p1++) = *p2;

        if (p2)
        {
            p2++;
        }
    } // while

    *p1 = '\0';
} // stripBlanks

BOOL Win32Gui::onAboutInit(HWND hwnd)
{
    char    aboutstring[512];


    memset(aboutstring, 0, 512);
    strcpy(aboutstring, "About ");
    strcat(aboutstring, PROGRAMNAME);
    SetWindowText(hwnd, aboutstring);
    strcpy(aboutstring, HEADER1);
    strcat(aboutstring, "V " PROGRAM_VERSION);
    strcat(aboutstring, HEADER2);
    stripBlanks(aboutstring);
    SetDlgItemText(hwnd, IDC_AB_TEXT, aboutstring);
    return TRUE;
}

BOOL Win32Gui::onAboutClose(HWND hwnd)
{
    popdown_about(hwnd);
    return TRUE;
}

BOOL Win32Gui::onAboutCommand(HWND hwnd, int cmd)
{
    switch (cmd)
    {
        // BP-Dialog button controls
        case IDC_AB_OK:
            popdown_about(hwnd);
            break;

        case IDC_AB_COPYRIGHT:
            popup_copyright(hwnd);
            popdown_about(hwnd);
            break;

        default:
            return FALSE;
    }

    return TRUE;
} // onAboutCommand

void Win32Gui::popup_copyright(HWND hwnd)
{
    char str[PATH_MAX];
    char *copyrightFile = "COPYING.TXT";

    GetCurrentDirectory(PATH_MAX, str);

    HINSTANCE res = ShellExecute(
            hwnd,           // handle of parent window
            "open",         // open file
            copyrightFile,  // file to open
            nullptr,           // parameters
            str,            // directory
            SW_SHOWNORMAL   // openmode for new window
    );
    if ((INT_PTR)res <= 32)
    {
        sprintf(str, "Unable to display file %s", copyrightFile);
        MessageBox(e2screen, str, PROGRAMNAME " error",
                   MB_OK | MB_ICONERROR);
    }
} // popup_copyright

bool Win32Gui::CheckDeviceSupport(HDC aHdc, bool isModifyValue, int *nrOfColors)
{
    int cNumColors, rasterCaps;

    rasterCaps = GetDeviceCaps(aHdc, RASTERCAPS);

    if (!(rasterCaps & RC_BITBLT) || !(rasterCaps & RC_DI_BITMAP))
    {
        if (!isModifyValue)
        {
            return false;    // fatal error, no BitBlt or SetDIBits available
        }

        popup_message("Device capabilities insufficient to start this "
                      "application\n(no BitBlt or SetDIBits available)");
        exit(1);
    }

    cNumColors = GetDeviceCaps(aHdc, NUMCOLORS);

    if (cNumColors == -1)
    {
        return true;    // success: The device has a color depth of more than 8 bit per pixel
    }

    if (cNumColors < *nrOfColors)
    {
        if (!isModifyValue)
        {
            return false;    // not enough colors
        }

        if (cNumColors >= 8)
        {
            *nrOfColors = 8;
        }
        else
        {
            *nrOfColors = 2;
        }

        return true;   // success by reducing nr of colors
    }

    return true;
}

void Win32Gui::SetColors(struct sGuiOptions &guiOptions)
{
    LOGPALETTE *pLog;
    int scale;
    Word    red   = 255;
    Word    green = 255;
    Word    blue  = 255;
    int     i, idx;

    getRGBForName(guiOptions.color.c_str(), &red, &green, &blue);

    pLog = (LOGPALETTE *)new char[sizeof(LOGPALETTE) + (MAX_COLORS - 1) *
                                  sizeof(PALETTEENTRY)];
    pLog->palVersion = 0x300;
    pLog->palNumEntries = MAX_COLORS;

    for (i = 0; i < MAX_COLORS; i++)
    {
        idx = i;

        if (guiOptions.isInverse)
        {
            idx = MAX_COLORS - idx - 1;
        }

        pLog->palPalEntry[idx].peFlags = PC_NOCOLLAPSE;

        if (withColorScale)
        {
            // Use same color values as Enhanced Graphics Adapter (EGA)
            // or Tandy Color Computer 3 RGB.
            // For details see:
            // https://en.wikipedia.org/wiki/Enhanced_Graphics_Adapter
            // https://exstructus.com/tags/coco/australia-colour-palette/
            static constexpr Byte colorValues[4] { 0x00, 0x55, 0xAA, 0xFF };

            // DEPENDANCIES:
            // the color plane masks used here depend on
            // the same masks used in CopyToZPixmap
            scale  = i & GREEN_HIGH ? 2 : 0;
            scale |= i & GREEN_LOW ? 1 : 0;
            pLog->palPalEntry[idx].peGreen = colorValues[scale];
            scale  = i & RED_HIGH ? 2 : 0;
            scale |= i & RED_LOW ? 1 : 0;
            pLog->palPalEntry[idx].peRed = colorValues[scale];
            scale  = i & BLUE_HIGH ? 2 : 0;
            scale |= i & BLUE_LOW ? 1 : 0;
            pLog->palPalEntry[idx].peBlue = colorValues[scale];
        }
        else
        {
            pLog->palPalEntry[idx].peBlue = (Byte)(blue  * sqrt((double)i / (
                    MAX_COLORS - 1)));
            pLog->palPalEntry[idx].peRed   = (Byte)(red   * sqrt((double)i / (
                    MAX_COLORS - 1)));
            pLog->palPalEntry[idx].peGreen = (Byte)(green * sqrt((double)i / (
                    MAX_COLORS - 1)));
        }
    }

    palette = CreatePalette(pLog);
    delete [] pLog;
}

void Win32Gui::initialize_after_create(HWND w, struct sGuiOptions &guiOptions)
{
    HDC             hdc;

    oldX          = 0;
    oldY          = 0;
    Word    red   = 255;
    Word    green = 255;
    Word    blue  = 255;
    int     foregroundIdx;
    int     backgroundIdx;
    int     i, j;
    BITMAPINFO *pbmi;

                        // screen depth on Win32 is 8
    const size_t size = WINDOWWIDTH * BLOCKHEIGHT *
                        MAX_PIXELSIZEX * MAX_PIXELSIZEY;
    image_data = std::unique_ptr<Byte[]>(new Byte[size]);
    memset(image_data.get(), 0, size);
    bmis.clear();

    getRGBForName(guiOptions.color.c_str(), &red, &green, &blue);

    if (!guiOptions.isInverse)
    {
        foregroundIdx = 1;
        backgroundIdx = 0;
    }
    else
    {
        foregroundIdx = 0;
        backgroundIdx = 1;
    }

    hdc = GetDC(w);
    CheckDeviceSupport(hdc, 1, &nColors);
    SetColors(guiOptions);

    for (i = 0; i < MAX_PIXELSIZEX; i++)
    {
        for (j = 0; j < MAX_PIXELSIZEY; j++)
        {
            pbmi = (BITMAPINFO *)new char[sizeof(BITMAPINFOHEADER) +
                                           MAX_COLORS * sizeof(Word)];

            pbmi->bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
            pbmi->bmiHeader.biPlanes         = 1;
            pbmi->bmiHeader.biWidth          = BLOCKWIDTH * (i + 1);
            pbmi->bmiHeader.biHeight         = -BLOCKHEIGHT * (j + 1);
            pbmi->bmiHeader.biBitCount       = 8;
            pbmi->bmiHeader.biCompression    = BI_RGB;
            pbmi->bmiHeader.biSizeImage      = 0;
            pbmi->bmiHeader.biXPelsPerMeter  = 0;
            pbmi->bmiHeader.biYPelsPerMeter  = 0;
            pbmi->bmiHeader.biClrUsed        = MAX_COLORS;
            pbmi->bmiHeader.biClrImportant   = 0;

            Word *wp = (Word *)((Byte *)pbmi + sizeof(BITMAPINFOHEADER));

            for (Word coloridx = 0; coloridx < MAX_COLORS; coloridx++)
            {
                *(wp++) = coloridx;
            }

            image[i][j]  = CreateDIBitmap(hdc, &pbmi->bmiHeader, 0, nullptr,
                                           pbmi, DIB_PAL_COLORS);
            bmis.push_back(std::unique_ptr<Byte[]>((Byte *)pbmi));
        } // for
    } // for

    // initialize Bitmap with all ones
    PatBlt(hdc, 0, 0, BLOCKWIDTH * pixelSizeX, BLOCKHEIGHT * pixelSizeY, WHITENESS);
    ReleaseDC(w, hdc);
    // Periodic 20 ms Timer for 50 Hz display update
    idTimer = SetTimer(w, GUI_TIMER_ID, TIMER_UPDATE, nullptr);
}

void Win32Gui::manage_widget(HWND w)
{
    ShowWindow(w, winApiContext.nCmdShow);
}  // manage widget


void Win32Gui::initialize_after_open(HWND w)
{
    const char *title = get_title();

    SetWindowText(w, title);
    warp_home_x = (pixelSizeX * WINDOWWIDTH)  >> 1;
    warp_home_y = (pixelSizeY * WINDOWHEIGHT) >> 1;
    release_mouse_capture(w);

    if (scheduler.get_target_frequency() == ORIGINAL_FREQUENCY)
    {
        frequency_control_on = true;
        update_frequency_check();
    }
}

GuiType Win32Gui::gui_type()
{
    return GuiType::WINDOWS;
}

Win32Gui::Win32Gui(
    Mc6809 &x_cpu,
    Memory &x_memory,
    Scheduler &x_scheduler,
    Inout  &x_inout,
    VideoControl1 &x_vico1,
    VideoControl2 &x_vico2,
    JoystickIO &x_joystickIO,
    KeyboardIO &x_keyboardIO,
    TerminalIO &x_terminalIO,
    Pia1 &x_pia1,
    struct sGuiOptions &x_options)
        : AbstractGui(
               x_cpu
             , x_memory
             , x_scheduler
             , x_inout
             , x_vico1
             , x_vico2
             , x_joystickIO
             , x_keyboardIO
             , x_terminalIO
             , x_options)
        , pia1(x_pia1)
        , cpu_popped_up(false)
        , oldX(0)
        , oldY(0)
        , idTimer(0)
        , is_use_undocumented(false)
        , cpu_stat(nullptr)
{
    initialize(options);
}

Win32Gui::~Win32Gui()
{
    int i, j;


    // release all bitmaps
    for (i = 0; i < MAX_PIXELSIZEX; i++)
    {
        for (j = 0; j < MAX_PIXELSIZEY; j++)
        {
            DeleteObject(image[i][j]);
        } // for
    } // for

    if (hFontFixed != nullptr)
    {
        DeleteObject(hFontFixed);
    }

    if (palette != nullptr)
    {
        DeleteObject(palette);
    }

    if (IsMenu(menubar))
    {
        DestroyMenu(menubar);
    }

    delete cpu_stat;
    cpu_stat = nullptr;
    ggui = nullptr;
}

/***************************/
/* CPU-view implemantation */
/***************************/

void Win32Gui::popup_cpu()
{
    static bool firstTime = true;

    CheckMenuItem(menu2, IDM_VIEW, MF_BYCOMMAND | MF_CHECKED);
    ShowWindow(cpuform, SW_SHOW); // Display modeless dialog

    if (firstTime)
    {
        // Create Font, calculate min size and
        // Resize/Move CPU dialog
        RECT rect;
        HDC hdc;
        HWND hwndControl;
        TEXTMETRIC tmi;
        int left, top;
        int minCpuHeightAlt;

        hwndControl = GetDlgItem(cpuform, IDC_CPU_TEXT);
#ifdef _MSC_VER
        _ASSERT(hwndControl != nullptr);
#endif
        hdc = GetDC(hwndControl);
        hFontFixed = CreateFontIndirect(getLogFontStruct(hdc, 8));
        SelectFont(hdc, hFontFixed);
        GetTextMetrics(hdc, &tmi);
        ReleaseDC(hwndControl, hdc);
        SetWindowFont(hwndControl, hFontFixed, TRUE);

        hwndControl = GetDlgItem(cpuform, IDP_RUN);
#ifdef _MSC_VER
        _ASSERT(hwndControl != nullptr);
#endif
        GetWindowRect(hwndControl, &rect);

        minCpuWidth  = 4 + ((CPU_LINE_SIZE + 1) * tmi.tmAveCharWidth) +
                       rect.right - rect.left;
        minCpuHeight = ((CPU_LINES - 1)     * tmi.tmHeight);
        minCpuHeightAlt = 8 * (rect.bottom - rect.top);

        if (minCpuHeight < minCpuHeightAlt)
        {
            minCpuHeight = minCpuHeightAlt;
        }

        rect.top = 0;
        rect.left = 0;
        rect.bottom = minCpuHeight;
        rect.right  = minCpuWidth;

        if (AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                               WS_THICKFRAME | WS_MINIMIZEBOX, TRUE, 0))
        {
            minCpuWidth  = rect.right - rect.left;
            minCpuHeight = rect.bottom - rect.top;
        };

        GetWindowRect(e2screen, &rect);

        left = rect.left + ((rect.right - rect.left) >> 1) - (minCpuWidth >> 1);

        top  = rect.top  - minCpuHeight;

        if (top < 0)
        {
            top = rect.bottom;
        }

        MoveWindow(cpuform, left, top, minCpuWidth, minCpuHeight, TRUE);
        firstTime = false;
    }

    cpu_popped_up = true;
}

void Win32Gui::popdown_cpu()
{
    CheckMenuItem(menu2, IDM_VIEW, MF_BYCOMMAND | MF_UNCHECKED);
    ShowWindow(cpuform, SW_HIDE);
    cpu_popped_up = false;
}

void Win32Gui::toggle_cpu()
{
    if (cpu_popped_up)
    {
        popdown_cpu();
    }
    else
    {
        popup_cpu();
    }
}

LOGFONT *Win32Gui::getLogFontStruct(HDC hdc, int pointSize)
{
    static LOGFONT lf;

    lf.lfHeight     = -MulDiv(pointSize,
                              GetDeviceCaps(hdc, LOGPIXELSY), 72);
    lf.lfWidth      = 0;
    lf.lfEscapement     = 0;
    lf.lfOrientation    = 0;
    lf.lfWeight     = FW_DONTCARE;
    lf.lfItalic     = 0;
    lf.lfUnderline      = 0;
    lf.lfStrikeOut      = 0;
    lf.lfCharSet        = ANSI_CHARSET;
    lf.lfOutPrecision   = OUT_TT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
    lf.lfFaceName[0]    = '\0';
    return &lf;
} // getLogFontStruct

void Win32Gui::create_cpuview(HWND parent)
{
    cpu_popped_up = false;
    set_line_delim("\r\n");
    cpuform = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CPU_DIALOG),
                           parent, cpuWindowWndProc);
#ifdef _MSC_VER
    _ASSERT(cpuform != nullptr);
#endif
    LONG_PTR hIcon = reinterpret_cast<LONG_PTR>(
        LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FLEXCPU)));
    SetClassLongPtr(cpuform, GCLP_HICON, hIcon);
}

BOOL Win32Gui::onCpuInit(HWND hwnd)
{
#ifndef FASTFLEX
    const char *title = "Mc6809";
#else
    const char *title = "Mc6809 (L.C. Benschop)";
#endif

    SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)title);

    return TRUE;
}

BOOL Win32Gui::onCpuCommand(HWND hwnd, int cmd)
{
    switch (cmd)
    {
        // cpuview button controls
        case IDP_NEXT:
            request_new_state(CpuState::Next);
            break;

        case IDP_STEP:
            request_new_state(CpuState::Step);
            break;

        case IDP_STOP:
            request_new_state(CpuState::Stop);
            break;

        case IDP_RUN:
            request_new_state(CpuState::Run);
            break;

        case IDP_RESET:
            request_new_state(CpuState::Reset);
            break;

        case IDP_BP:
            popup_bp(hwnd);
            break;

        case IDP_LOG:
            popup_log(hwnd);
            break;

        case IDP_CPU:
            popdown_cpu();
            break;

        default:
            return FALSE;
    }

    return TRUE;
} // onCpuCommand

BOOL Win32Gui::onCpuClose(HWND /* [[maybe_unused]] HWND hwnd */)
{
    popdown_cpu();
    return TRUE;
}

INT_PTR CALLBACK cpuWindowWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (ggui == nullptr)
    {
        return FALSE;
    }

    switch (message)
    {
        case WM_INITDIALOG:
            return ggui->onCpuInit(hwnd);

        case WM_COMMAND:
            return ggui->onCpuCommand(hwnd, LOWORD(wParam));

        case WM_CLOSE:
            return ggui->onCpuClose(hwnd);

        case WM_SIZE:
            return ggui->onCpuSize(hwnd, (int)wParam,
                                   (int)LOWORD(lParam), (int)HIWORD(lParam));

        case WM_SIZING:
            return ggui->onCpuSizing(hwnd, (int)wParam, (LPRECT)lParam);
    } // switch

    return FALSE;
} // cpuWindowWndProc

void Win32Gui::redraw_cpuview_impl(const Mc6809CpuStatus &stat)
{
    int         i;
    HWND        hwnd;

    hwnd = cpuform;
    i = stat.s & 7;
    text(5 + 3 * i, 10, "[");
    text(8 + 3 * i, 10, "]");
    SetDlgItemText(cpuform, IDC_CPU_TEXT, cpustring);
}

void Win32Gui::update_cpuview(const Mc6809CpuStatus &s)
{
    redraw_cpuview(s);
}

BOOL Win32Gui::onBpCommand(HWND hwnd, int cmd)
{
    switch (cmd)
    {
        // BP-Dialog button controls
        case IDC_BP_OK:
            popdown_bp(cmd, hwnd);
            break;

        case IDC_BP_CANCEL:
            popdown_bp(cmd, hwnd);
            break;

        case IDC_BP_CLEAR:
            clear_bp(hwnd);
            break;

        default:
            return FALSE;
    }

    return TRUE;
} // onBpCommand

BOOL Win32Gui::onBpInit(HWND hwnd)
{
    char bpstring[16];
    HWND hEdit;
    int which;

    sprintf(bpstring, "Breakpoints");
    SetWindowText(hwnd, bpstring);

    for (which = 0; which <= 1; which++)
    {
        strcpy(bpstring, "");

        if (cpu.is_bp_set(which))
        {
            sprintf(bpstring, "%04x", cpu.get_bp(which));
        }

        SetDlgItemText(hwnd, IDC_BP_ADDR1 + which, bpstring);
    }

    hEdit = GetDlgItem(hwnd, IDC_BP_ADDR1);
#ifdef _MSC_VER
    _ASSERT(hEdit != nullptr);
#endif
    // Select contents of edit control
    SendMessage(hEdit, EM_SETSEL, 0, -1);

    return TRUE;
}

BOOL Win32Gui::onBpClose(HWND hwnd)
{
    EndDialog(hwnd, 0);
    return TRUE;
}

BOOL Win32Gui::onCpuSizing(HWND /* [[maybe_unused]] HWND hwnd */,
    int edge, LPRECT pRect)
{
    int what = 0;

    switch (edge)
    {
        case WMSZ_RIGHT:
            what = CHECK_RIGHT;
            break;

        case WMSZ_LEFT:
            what = CHECK_LEFT;
            break;

        case WMSZ_TOP:
            what = CHECK_TOP;
            break;

        case WMSZ_BOTTOM:
            what = CHECK_BOTTOM;
            break;

        case WMSZ_BOTTOMLEFT:
            what = CHECK_BOTTOM | CHECK_LEFT;
            break;

        case WMSZ_BOTTOMRIGHT:
            what = CHECK_BOTTOM | CHECK_RIGHT;
            break;

        case WMSZ_TOPLEFT:
            what = CHECK_TOP    | CHECK_LEFT;
            break;

        case WMSZ_TOPRIGHT:
            what = CHECK_TOP    | CHECK_RIGHT;
            break;
    }

    if ((what & CHECK_LEFT) && pRect->right - pRect->left < minCpuWidth)
    {
        pRect->left = pRect->right - minCpuWidth;
    }

    if ((what & CHECK_RIGHT) && pRect->right - pRect->left < minCpuWidth)
    {
        pRect->right = pRect->left + minCpuWidth;
    }

    if ((what & CHECK_TOP) && pRect->bottom - pRect->top < minCpuHeight)
    {
        pRect->top = pRect->bottom - minCpuHeight;
    }

    if ((what & CHECK_BOTTOM) && pRect->bottom - pRect->top < minCpuHeight)
    {
        pRect->bottom = pRect->top + minCpuHeight;
    }

    return TRUE;
}

BOOL Win32Gui::onCpuSize(HWND hwnd, int sizeType, int width, int height)
{
    if (sizeType != SIZE_MINIMIZED && hwnd == cpuform)
    {
        HWND hwndControl;
        RECT rect;
        int width_button, height_button, top_button;
        int width_edit, height_edit;
        int i;

        hwndControl = GetDlgItem(hwnd, IDP_RUN);
#ifdef _MSC_VER
        _ASSERT(hwndControl != nullptr);
#endif
        GetClientRect(hwndControl, &rect);
        width_button  = rect.right  - rect.left;
        height_button = rect.bottom - rect.top;
        top_button    = 2;
        hwndControl = GetDlgItem(hwnd, IDC_CPU_TEXT);
#ifdef _MSC_VER
        _ASSERT(hwndControl != nullptr);
#endif
        GetClientRect(hwndControl, &rect);
        width_edit = width - width_button - 6;
        height_edit = height - 2;
        MoveWindow(hwndControl, 2, 2,
                   width_edit, height_edit, TRUE);

        for (i = 0; i < 8; i++)
        {
            hwndControl = GetDlgItem(hwnd, IDP_RUN + i);
#ifdef _MSC_VER
            _ASSERT(hwndControl != nullptr);
#endif
            MoveWindow(hwndControl, width_edit + 4, top_button,
                       width_button, height_button, TRUE);
            top_button += height_button + 2;
        }

        return TRUE;
    }

    return FALSE;
}

INT_PTR CALLBACK bpDialogProc(HWND hwnd, UINT message, WPARAM wParam,
                           LPARAM /* [[maybe_unused]] LPARAM lParam */)
{
    if (ggui == nullptr)
    {
        return FALSE;
    }

    switch (message)
    {
        case WM_INITDIALOG:
            return ggui->onBpInit(hwnd);

        case WM_COMMAND:
            return ggui->onBpCommand(hwnd, LOWORD(wParam));

        case WM_CLOSE:
            return ggui->onBpClose(hwnd);
    } // switch

    return FALSE;
} // bpDialogProc

void Win32Gui::popup_bp(HWND hwnd)
{
    UINT_PTR res;

    res = DialogBox(hInstance, MAKEINTRESOURCE(IDD_BP_DIALOG), hwnd,
        static_cast<DLGPROC>(bpDialogProc));
}

void Win32Gui::popdown_bp(int cmd, HWND hwnd)
{
    char        bpstring[16];
    unsigned int    addr, which;

    if (cmd == IDC_BP_OK)
        for (which = 0; which <= 1; which++)
        {
            GetDlgItemText(hwnd, IDC_BP_ADDR1 + which, bpstring, 16);

            if (sscanf(bpstring, "%x", &addr) == 1 && addr <= 0xffff)
            {
                cpu.set_bp(which, (Word)addr);
            }
            else if (strlen(bpstring) == 0)
            {
                cpu.reset_bp(which);
            }
        }

    EndDialog(hwnd, 0);
}

void Win32Gui::clear_bp(HWND hwnd)
{
    SetDlgItemText(hwnd, IDC_BP_ADDR1, "");
    SetDlgItemText(hwnd, IDC_BP_ADDR2, "");
}

BOOL Win32Gui::onLogCommand(HWND hwnd, int cmd)
{
    switch (cmd)
    {
        // Log-Dialog button controls
        case IDC_LOG_OK:
            popdown_log(cmd, hwnd);
            break;

        case IDC_LOG_CANCEL:
            popdown_log(cmd, hwnd);
            break;

        case IDC_LOG_CLEAR:
            clear_log(hwnd);
            break;

        case IDC_LOG_FPROMPT:
            prompt_logfile(hwnd);
            break;

        default:
            return FALSE;
    }

    return TRUE;
} // onLogCommand

BOOL Win32Gui::onLogInit(HWND hwnd)
{
    HWND hEdit;
    char tmpstring[32];

    sprintf(tmpstring, "Logging");
    SetWindowText(hwnd, tmpstring);

    // Min Address
    tmpstring[0] = '\0';

    if (lfs.minAddr < 0x10000)
    {
        sprintf(tmpstring, "%04X", lfs.minAddr);
    }

    SetDlgItemText(hwnd, IDC_LOG_MINADDR, tmpstring);
    // Max Address
    tmpstring[0] = '\0';

    if (lfs.maxAddr < 0x10000)
    {
        sprintf(tmpstring, "%04X", lfs.maxAddr);
    }

    SetDlgItemText(hwnd, IDC_LOG_MAXADDR, tmpstring);
    // Start Address
    tmpstring[0] = '\0';

    if (lfs.startAddr < 0x10000)
    {
        sprintf(tmpstring, "%04X", lfs.startAddr);
    }

    SetDlgItemText(hwnd, IDC_LOG_STARTADDR, tmpstring);
    // Stop Address
    tmpstring[0] = '\0';

    if (lfs.stopAddr < 0x10000)
    {
        sprintf(tmpstring, "%04X", lfs.stopAddr);
    }

    SetDlgItemText(hwnd, IDC_LOG_STOPADDR, tmpstring);
    SetDlgItemText(hwnd, IDC_LOG_FILENAME, lfs.logFileName.c_str());

    hEdit = GetDlgItem(hwnd, IDC_LOG_MINADDR);
#ifdef _MSC_VER
    _ASSERT(hEdit != nullptr);
#endif
    // Select contents of edit control
    SendMessage(hEdit, EM_SETSEL, 0, -1);

    return TRUE;
}

BOOL Win32Gui::onLogClose(HWND hwnd)
{
    EndDialog(hwnd, 0);
    return TRUE;
}

INT_PTR CALLBACK logDialogProc(HWND hwnd, UINT message, WPARAM wParam,
                            LPARAM /* [[maybe_unused]] LPARAM lParam */)
{
    if (ggui == nullptr)
    {
        return FALSE;
    }

    switch (message)
    {
        case WM_INITDIALOG:
            return ggui->onLogInit(hwnd);

        case WM_COMMAND:
            return ggui->onLogCommand(hwnd, LOWORD(wParam));

        case WM_CLOSE:
            return ggui->onLogClose(hwnd);
    } // switch

    return FALSE;
} // logDialogProc

void Win32Gui::popup_log(HWND hwnd)
{
    UINT_PTR res;

    res = DialogBox(hInstance, MAKEINTRESOURCE(IDD_LOG_DIALOG), hwnd,
              static_cast<DLGPROC>(logDialogProc));
}

void Win32Gui::popdown_log(int cmd, HWND hwnd)
{
    char tmpstring[PATH_MAX];
    unsigned int addr;

    if (cmd == IDC_LOG_OK)
    {
        // Min Address
        GetDlgItemText(hwnd, IDC_LOG_MINADDR, tmpstring, 16);
        lfs.minAddr = 0x0000;

        if (sscanf(tmpstring, "%x", &addr) == 1 && addr <= 0xffff)
        {
            lfs.minAddr = addr;
        }

        // Max Address
        GetDlgItemText(hwnd, IDC_LOG_MAXADDR, tmpstring, 16);
        lfs.maxAddr = 0xFFFF;

        if (sscanf(tmpstring, "%x", &addr) == 1 && addr <= 0xffff)
        {
            lfs.maxAddr = addr;
        }

        // Start Address
        GetDlgItemText(hwnd, IDC_LOG_STARTADDR, tmpstring, 16);
        lfs.startAddr = 0x10000;

        if (sscanf(tmpstring, "%x", &addr) == 1 && addr <= 0xffff)
        {
            lfs.startAddr = addr;
        }

        // Stop Address
        GetDlgItemText(hwnd, IDC_LOG_STOPADDR, tmpstring, 16);
        lfs.stopAddr = 0x10000;

        if (sscanf(tmpstring, "%x", &addr) == 1 && addr <= 0xffff)
        {
            lfs.stopAddr = addr;
        }

        // Log Filename
        GetDlgItemText(hwnd, IDC_LOG_FILENAME, tmpstring, PATH_MAX);
        lfs.logFileName = tmpstring;

        scheduler.sync_exec(BCommandPtr(new CSetLogFile(cpu, lfs)));
    }

    EndDialog(hwnd, 0);
}

void Win32Gui::clear_log(HWND hwnd)
{
    SetDlgItemText(hwnd, IDC_LOG_MINADDR,   "");
    SetDlgItemText(hwnd, IDC_LOG_MAXADDR,   "");
    SetDlgItemText(hwnd, IDC_LOG_STARTADDR, "");
    SetDlgItemText(hwnd, IDC_LOG_STOPADDR,  "");
    SetDlgItemText(hwnd, IDC_LOG_FILENAME,  "");
}

void Win32Gui::prompt_logfile(HWND hwnd)
{
    OPENFILENAME of;
    char filename[PATH_MAX];

    GetDlgItemText(hwnd, IDC_LOG_FILENAME, filename, PATH_MAX);
    memset(&of, 0, sizeof(OPENFILENAME));
    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner   = hwnd;
    of.hInstance   = hInstance;
    of.lpstrFilter = "*.LOG";
    of.lpstrFile   = filename;
    of.nMaxFile    = PATH_MAX;
    of.lpstrTitle  = "Choose Logging Filename";
    of.Flags       = OFN_HIDEREADONLY;
    of.lpstrDefExt = "LOG";

    if (GetSaveFileName(&of))
    {
        SetDlgItemText(hwnd, IDC_LOG_FILENAME, of.lpstrFile);
    }
}
#endif // _WIN32

