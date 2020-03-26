/*
    win32gui.h  userinterface for Windows


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

#ifndef WIN32GUI_INCLUDED
#define WIN32GUI_INCLUDED

#ifdef _WIN32
#include "misc1.h"
#include "flexemu.h"
#include "schedcpu.h"
#include "scpulog.h"
#ifdef _MSC_VER
    #include <commctrl.h>
#endif
#include <vector>
#include <memory>

class Mc6809;
class Memory;
class Scheduler;
class Inout;
class VideoControl1;
class VideoControl2;
class Mc6809CpuStatus;
class JoystickIO;
class KeyboardIO;
class TerminalIO;
class Pia1;

// maybe someone has forgotten to define this:

#ifndef SelectBitmap
#define SelectBitmap(hdc, hbm)\
    ((HBITMAP)SelectObject((hdc), (HGDIOBJ)(HBITMAP)(hbm)))
#endif
#ifndef SelectFont
#define SelectFont(hdc, hfont)\
    ((HFONT)SelectObject((hdc),(HGDIOBJ)(HFONT)(hfont)))
#endif

#ifndef FORWARD_WM_SETFONT
#define FORWARD_WM_SETFONT(hwnd, hfont, fRedraw, fn) \
    (void)(fn)((hwnd), WM_SETFONT, (WPARAM)(HFONT)(hfont), \
               (LPARAM)(BOOL)(fRedraw))
#endif

#ifndef SetWindowFont
#define SetWindowFont(hwnd, hfont, fRedraw) \
    FORWARD_WM_SETFONT((hwnd), (hfont), (fRedraw), SendMessage)
#endif

#ifndef IsLButtonDown
    #define     IsLButtonDown()  (GetKeyState(VK_LBUTTON) < 0)
#endif
#ifndef IsRButtonDown
    #define     IsRButtonDown()  (GetKeyState(VK_RBUTTON) < 0)
#endif
#ifndef IsMButtonDown
    #define     IsMButtonDown()  (GetKeyState(VK_MBUTTON) < 0)
#endif

// casting only needed for MSVC = V5.0
#ifdef _MSC_VER
    #if (_MSC_VER == 1100) /* MSVC V5.0 */
        #define CALLBACKCAST (int (CALLBACK *)())
    #endif
    #if (_MSC_VER == 1200) /* MSVC V6.0 */
        #define CALLBACKCAST
    #endif
#else
    #define CALLBACKCAST
#endif

#include "absgui.h"

#define NR_OF_HELPBUTTONS       (9)

// IDs for menu items

#define IDM_UPDATE_FS         (101)
#define IDM_EXIT              (102)
#define IDM_RUN               (201)
#define IDM_STOP              (202)
#define IDM_RESET             (203)
#define IDM_VIEW              (204)
#define IDM_BREAKPOINTS         (205)
#define IDM_LOGFILE             (206)
#define IDM_FREQUENCY0        (207)
#define IDM_UNDOCUMENTED      (208)
#define IDM_ABOUT             (301)
#define IDM_INTRODUCTION      (302)
#define IDM_COPYRIGHT         (303)

#define GUI_TIMER_ID          (556)

class Win32Gui : public AbstractGui
{
protected:
    enum
    {
        FLX_INVISIBLE_CURSOR = 10,
        FLX_DEFAULT_CURSOR   = 11
    };
    HINSTANCE   hInstance;
    HWND        e2screen; // handle to E2 graphic display
    HWND        hwndStatus; // handle to status bar
    HWND        hButtonFloppy[4];
    HWND        hButtonIrq;
    HMENU       menubar;  // menu bar of Main Window
    // bitmap info for following bitmaps
    std::vector<std::unique_ptr<Byte[]> > bmis;
    // default bitmap for graphic display
    HBITMAP     image[MAX_PIXELSIZEX][MAX_PIXELSIZEY];
    HPALETTE    palette;
    HMENU       menu1, menu2, menu3; // menu handles

    HWND        cpuform;  // CPU Window
    HFONT       hFontFixed; // Fixed font for CPU Window
    // memory block for calculating bitmaps for display
    std::unique_ptr<Byte[]> image_data;
    int     minCpuWidth, minCpuHeight;

public:
    void    onCommand(HWND hwndWindow, int cmd, HWND hwndControl);
    void    onPaint(HWND hwnd);
    void    onChar(HWND hwnd, SWord ch, int repeat);
    void    onKeyDown(HWND hwnd, SWord ch, int repeat);
    void    onPaletteChanged(HWND hwnd);
    void    onMouseMove(HWND hwnd, Word newX, Word newY,
                        Word fwKeys);
    void    onTimer(HWND hwnd, UINT id);
    int     onSize(HWND hwnd, int sizeType,
                   int width, int height);
    void    onDestroy(HWND hwnd);
    void    onClose(HWND hwnd);
    void    onSetFocus(HWND hwnd, HWND oldHwnd);
    void    onKillFocus(HWND hwnd, HWND newHwnd);
    void    onActivate(HWND hwnd, WORD what, HWND hwnd1);
    int     onMinMaxInfo(MINMAXINFO *lpmmi);
    BOOL    onCpuInit(HWND hwnd);
    BOOL    onCpuCommand(HWND hwnd, int cmd);
    BOOL    onCpuClose(HWND hwnd);
    BOOL    onCpuSize(HWND hwnd, int sizeType,
                      int width, int height);
    BOOL    onCpuSizing(HWND hwnd, int edge, LPRECT pRect);
    BOOL    onBpCommand(HWND hwnd, int cmd);
    BOOL    onBpInit(HWND hwnd);
    BOOL    onBpClose(HWND hwnd);
    BOOL    onAboutInit(HWND hwnd);
    BOOL    onAboutCommand(HWND hwnd, int cmd);
    BOOL    onAboutClose(HWND hwnd);
    BOOL    onLogCommand(HWND hwnd, int cmd);
    BOOL    onLogInit(HWND hwnd);
    BOOL    onLogClose(HWND hwnd);
    void    resetMouseMoveCoords();
    bool    CloseApp(HWND hwnd, bool confirm = false);

protected:
    BOOL    registerWindowClasses(HINSTANCE hinst);
    HWND    create_main_view();
    void set_bell(int percent) override;
    void    update_disk_status(int floppyIndex, DiskStatus status);
    void    update_interrupt_status(tIrqType irqType, bool status);
    void    update_block(int block_number, HDC hdc);
    SWord   translate_to_ascii(SWord key);
    SWord   translate_to_ascii1(SWord key);
    void    SetColors(struct sGuiOptions &options);
    bool    CheckDeviceSupport(HDC aHdc, bool isModifyValue,
                               int *nrOfColors);
    void    mouse_update(HWND w);
    void    mouse_warp(HWND w, int dx, int dy);
    void    set_cursor(int type = FLX_DEFAULT_CURSOR);
    void    toggle_mouse_capture(HWND w);
    void    release_mouse_capture(HWND w);
    const char *get_title();
    void    update_frequency_check();

    // Pointer grabbing
    int      cursor_type;
    int      prev_x, current_x;
    int      prev_y, current_y;
    int      warp_x, warp_home_x, warp_dx;
    int      warp_y, warp_home_y, warp_dy;

    // CPU View
    void    create_cpuview(HWND parent);
    void    popup_cpu();
    void    toggle_freqency();
    void    toggle_undocumented();
    void    popdown_cpu();
    void    toggle_cpu();
    void redraw_cpuview_impl(const Mc6809CpuStatus &stat) override;
    LOGFONT *getLogFontStruct(HDC hdc, int pointSize);

    void            popup_copyright(HWND hwnd);
    void    manage_widget(HWND w);
    void    initialize_after_create(HWND w,
                                    struct sGuiOptions &options);
    void    initialize_after_open(HWND w);
    void initialize(struct sGuiOptions &options) override;
    void    initialize_e2window(struct sGuiOptions &options);
    void    stripBlanks(char *str);

    // Internal registers

protected:

    Pia1 &pia1;
    bool            cpu_popped_up;
    int             oldX, oldY;
    Word            mouseMoved;
    UINT_PTR        idTimer;
    bool            frequency_control_on;
    bool            is_use_undocumented;
    s_cpu_logfile lfs;
    Mc6809CpuStatus *cpu_stat;

public:
    void    update_cpuview(const Mc6809CpuStatus &stat) override;
    void    popup_message(char *pmessage);
    void    popup_disk_info(HWND hwnd);
    void    popup_interrupt_info();
    int     popup_help(HWND hwnd);
    void    popup_about(HWND hwnd);
    void    popdown_about(HWND hwnd);
    void    popup_bp(HWND hwnd);
    void    popdown_bp(int cmd, HWND hwnd);
    void    clear_bp(HWND hwnd);
    void    popup_log(HWND hwnd);
    void    popdown_log(int cmd, HWND hwnd);
    void    clear_log(HWND hwnd);
    void    prompt_logfile(HWND hwnd);
    void    main_loop();
    GuiType gui_type();

public:
    Win32Gui(
        Mc6809 &,
        Memory &,
        Scheduler &,
        Inout &,
        VideoControl1 &,
        VideoControl2 &,
        JoystickIO &,
        KeyboardIO &,
        TerminalIO &,
        Pia1 &,
        struct sGuiOptions &);
    virtual ~Win32Gui();
};

#endif // _WIN32
#endif // WIN32GUI_INCLUDED
