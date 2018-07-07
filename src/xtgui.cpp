/*
    xtgui.cpp  user interface for XToolkit


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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

#ifdef HAVE_XTK

#include <sstream>
#include <stdio.h>
#include <ctype.h>
#include "xtgui.h"
#include <X11/IntrinsicP.h>
#include "benv.h"
#include "bfileptr.h"
#include "csetfreq.h"
#include "clogfile.h"
#include "vico1.h"
#include "vico2.h"
#include "mc6809.h"
#include "mc6809st.h"
#include "inout.h"
#include "schedule.h"
#include "mc6809st.h"
#include "joystick.h"
#include "keyboard.h"
#include "pia1.h"
#include "cacttrns.h"

#include "bitmaps/ok.xbm"
#include "bitmaps/flexmain.xbm"

#ifdef HAVE_XPM
    #include "flexemu.xpm"
    #include "flexcpu.xpm"
    #include "bitmaps/author.xpm"
    #include "bitmaps/floppy0.xpm"
    #include "bitmaps/floppy1.xpm"
    #include "bitmaps/floppy2.xpm"
    #include "bitmaps/irq0.xpm"
    #include "bitmaps/irq1.xpm"
    #include "bitmaps/irq2.xpm"
    #include "bitmaps/irq3.xpm"
    #include "bitmaps/irq4.xpm"
#endif
#include "bitmaps/flexcpu.xbm"

// global variables
static XtGui *ggui = nullptr;
const char **XtGui::pixmapname[8] = { floppy0, floppy1, floppy2,
                                      irq0, irq1, irq2, irq3, irq4
                                    };

int XtGui::radio_data[static_cast<Byte>(CpuState::_count)] =
{
};

void expose(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_expose(pevent);
}

#ifdef HAVE_XPM
void author_expose(Widget w, XEvent *pevent, String *, Cardinal *)
{
    ggui->a_expose(w, pevent);
}
#endif

void keyPress(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_keyPress(pevent);
}

void buttonPress(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_buttonPress(pevent);
}

void buttonRelease(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_buttonRelease(pevent);
}

void enter(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_enter(pevent);
}

void leave(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_leave(pevent);
}

void motion(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_motion(pevent);
}

void focusInMain(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_focusIn(pevent);
}

void focusOutMain(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_focusOut(pevent);
}

void popup_no_resources(Widget, XEvent *, String *, Cardinal *)
{
    ggui->popup_message("");
    ggui->set_exit();
}

void processResize(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_process_resize(pevent);
}

void wm_protocols(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_wm_protocols(pevent);
}

void closeMenuMode(Widget, XEvent *pevent, String *, Cardinal *)
{
    ggui->c_close_menu_mode(pevent);
}

void highlightChild(Widget w, XEvent *pevent, String *params, Cardinal *)
{
    ggui->c_highlight_child(w, pevent, params);
}

void XtGui::timerCallbackProc(XtPointer p, XtIntervalId *pId)
{
    if (p != nullptr)
    {
        XtGui *pGui = (XtGui *)p;
        pGui->timerCallback(*pId);
    }
}

void XtGui::initialize(struct sGuiOptions &options)
{
    int i;

    XAbstractGui::initialize(options);
    lfs.reset();
    ggui = this;        // global instance pointer needed for callbacks
    warp_x = 0;
    warp_y = 0;
    warp_home_x = 0;
    warp_home_y = 0;
    prev_x = -1;
    prev_y = -1;
    current_x = -1;
    current_y = -1;
    mouse_button_state = 0;
    image_data = nullptr;
#ifdef HAVE_XPM

    for (i = PM_FLOPPY0; i <= PM_IRQ4; ++i)
    {
        pixmap[i] = None;
    }

#endif
    initialize_e2window(options);
} // initialize

void XtGui::setCpuExitCallback(Widget,
                               XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
        ((XtGui *)client_data)->popup_confirmation(
            "Do you really want to close " PROGRAMNAME);
}

void XtGui::setCpuRunCallback(Widget,
                              XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->request_new_state(CpuState::Run);
    }
}

void XtGui::setCpuStopCallback(Widget,
                               XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->request_new_state(CpuState::Stop);
    }
}

void XtGui::toggleCpuRunCallback(Widget,
                                 XtPointer client_data, XtPointer call_data)
{
    if (client_data != nullptr && call_data != nullptr)
    {
        ((XtGui *)client_data)->request_new_state(CpuState::Run);
    }
}

void XtGui::toggleCpuStopCallback(Widget,
                                  XtPointer client_data, XtPointer call_data)
{
    if (client_data != nullptr && call_data != nullptr)
    {
        ((XtGui *)client_data)->request_new_state(CpuState::Stop);
    }
}

void XtGui::toggleCpuStepCallback(Widget,
                                  XtPointer client_data, XtPointer call_data)
{
    if (client_data != nullptr && call_data != nullptr)
    {
        ((XtGui *)client_data)->request_new_state(CpuState::Step);
    }
}

void XtGui::toggleCpuNextCallback(Widget,
                                  XtPointer client_data, XtPointer call_data)
{
    if (client_data != nullptr && call_data != nullptr)
    {
        ((XtGui *)client_data)->request_new_state(CpuState::Next);
    }
}

void XtGui::toggleCpuResetCallback(Widget,
                                   XtPointer client_data, XtPointer call_data)
{
    if (client_data != nullptr && call_data != nullptr)
    {
        ((XtGui *)client_data)->request_new_state(CpuState::Reset);
    }
}

void XtGui::setCpuResetRunCallback(Widget,
                                   XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->request_new_state(CpuState::ResetRun);
    }
}

void XtGui::toggleCpuCallback(Widget,
                              XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->toggle_cpu();
    }
}

void XtGui::toggleUndocumentedCallback(Widget,
                                       XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->toggle_undocumented();
    }
}

void XtGui::toggleFrequencyCallback(Widget,
                                    XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->toggle_frequency();
    }
}

void XtGui::popdownCpuCallback(Widget,
                               XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popdown_cpu();
    }
}

void XtGui::popupHelpCallback(Widget,
                              XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popup_help();
    }
}

void XtGui::popupAboutCallback(Widget,
                               XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popup_about();
    }
}

void XtGui::popdownAboutCallback(Widget,
                                 XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popdown_about();
    }
}

#ifdef HAVE_XPM
void XtGui::popupAuthorCallback(Widget,
                                XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popdown_about();
        ((XtGui *)client_data)->popup_author();
    }
}

void XtGui::popdownAuthorCallback(Widget,
                                  XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popdown_author();
    }
}
#endif // iddef HAVE_XPM

void XtGui::popupBpCallback(Widget,
                            XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popup_bp();
    }
}

void XtGui::popupLogfileCallback(Widget,
                                 XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popup_log();
    }
}

void XtGui::popdownLogCallback(Widget w,
                               XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popdown_log(w);
    }
}

void XtGui::clearLogCallback(Widget,
                             XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->clear_log();
    }
}

void XtGui::clearBpCallback(Widget,
                            XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->clear_bp();
    }
}

void XtGui::popdownBpCallback(Widget w,
                              XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popdown_bp(w);
    }
}

void XtGui::popdownMessageCallback(Widget w,
                                   XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popdown_message(w);
    }
}

void XtGui::popupFloppyCallback(Widget w,
                                XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popup_disk_info(w);
    }
}

void XtGui::popupInterruptCallback(Widget w,
                                   XtPointer client_data, XtPointer)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->popup_interrupt_info(w);
    }
}

void XtGui::c_process_resize(XEvent *pevent)
{
    pixelSizeX = (pevent->xconfigure.width + 10) / WINDOWWIDTH;
    pixelSizeX = pixelSizeX < 1 ? 1 : pixelSizeX;
    pixelSizeY = (pevent->xconfigure.height + 10) / WINDOWHEIGHT;
    pixelSizeY = pixelSizeY < 1 ? 1 : pixelSizeY;
    warp_home_x = (pixelSizeX * WINDOWWIDTH)  >> 1;
    warp_home_y = (pixelSizeY * WINDOWHEIGHT) >> 1;
    memory.init_blocks_to_update();
}

#ifdef HAVE_XPM
// color is the color used instead of transparency
void XtGui::create_pixmaps(Widget w, Pixel color)
{
    int errorStatus = XpmSuccess;
    XpmAttributes xpmattr;
    XpmColorSymbol symbol;
    const char *none = "none";
    int i;
    char **data;

    for (i = PM_FLOPPY0; i <= PM_IRQ4; ++i)
    {
        data = const_cast<char **>(pixmapname[i]);
        symbol.name = nullptr;
        symbol.value = const_cast<char *>(none);
        symbol.pixel = color;
        xpmattr.colorsymbols = &symbol;
        xpmattr.numsymbols = 1;
        xpmattr.colormap = DefaultColormapOfScreen(XtScreen(w));
        xpmattr.valuemask = XpmColormap | XpmColorSymbols;
        errorStatus = XpmCreatePixmapFromData(XtDisplay(w), XtWindow(w),
                                              data, &pixmap[i], nullptr, &xpmattr);
        XpmFreeAttributes(&xpmattr);

        if (errorStatus != XpmSuccess)
        {
            pixmap[i] = None;
        }
    }
}

void XtGui::popup_author()
{
    center_dialog(authorframe);
    XtPopup(authorframe, XtGrabExclusive);
    XtSetSensitive(e2toplevel, False);
    XtSetSensitive(cpuframe, False);
}

void XtGui::a_expose(Widget w, XEvent *pevent)
{
    static int errorStatus = XpmSuccess;
    XpmAttributes xpmattr;
    char **data;

    if (authorpixmap == None && errorStatus == XpmSuccess)
    {
        data = const_cast<char **>(author);
        xpmattr.colormap  = DefaultColormapOfScreen(XtScreen(w));
        xpmattr.valuemask = XpmColormap;
        errorStatus = XpmCreatePixmapFromData(XtDisplay(w), XtWindow(w),
                                              data, &authorpixmap, nullptr,
                                              &xpmattr);
        XpmFreeAttributes(&xpmattr);

        if (errorStatus != XpmSuccess)
            popup_message("\
sorry, unable to create image\n\
of author. (image need 62\n\
color entries)");
    }

    if (errorStatus == XpmSuccess)
        XCopyArea(XtDisplay(w), authorpixmap, XtWindow(w), e2gc,
                  pevent->xexpose.x,     pevent->xexpose.y,
                  pevent->xexpose.width, pevent->xexpose.height,
                  pevent->xexpose.x,     pevent->xexpose.y);
}

void XtGui::popdown_author()
{
    XtPopdown(authorframe);

    if (authorpixmap != None)
    {
        XFreePixmap(XtDisplay(authorframe), authorpixmap);
        authorpixmap = None;
    }

    XtSetSensitive(e2toplevel, True);
    XtSetSensitive(cpuframe, True);
}
#endif // #ifdef HAVE_XPM

void XtGui::popup_about()
{
    center_dialog(aboutframe);
    XtPopup(aboutframe, XtGrabExclusive);
    XtSetSensitive(e2toplevel, False);
    XtSetSensitive(cpuframe, False);
    XtSetKeyboardFocus(aboutform, aboutbutton);
}

void XtGui::popdown_about()
{
    XtPopdown(aboutframe);
    XtSetSensitive(e2toplevel, True);
    XtSetSensitive(cpuframe, True);
}

void XtGui::center_dialog(Widget shell)
{
    Position x, y;
    Dimension width, height;

    XtVaGetValues(e2toplevel, XtNwidth, &width, XtNheight, &height,
                  XtNx, &x, XtNy, &y, nullptr);
    x += width >> 1;
    y += height >> 1;
    XtVaGetValues(shell, XtNwidth, &width, XtNheight, &height, nullptr);
    x -= width >> 1;
    y -= height >> 1;
    XtMoveWidget(shell, x, y);
}

void XtGui::popup_message(const char *pmessage, const char *ptitle,
                          int width, int height)
{
    const char *p = ptitle;

    set_exit(false);

    if (*pmessage)
    {
        XtVaSetValues(messagetext, XtNstring, pmessage, nullptr);
    }

    if (p == nullptr)
    {
        p = PROGRAMNAME " Warning";
    }

    XtVaSetValues(messageframe, XtNtitle, p, nullptr);
    XtResizeWidget(messageframe, width, height, 0);
    center_dialog(messageframe);
    XtVaSetValues(messagebutton1, XtNlabel, "Ok", nullptr);
    XtUnmanageChild(messagebutton2);
    XtPopupSpringLoaded(messageframe);
    XtSetSensitive(e2toplevel, False);
    XtSetSensitive(cpuframe, False);
}

void XtGui::popup_confirmation(const char *pmessage, const char *ptitle,
                               int width, int height)
{
    const char *p = ptitle;

    set_exit(false);

    if (*pmessage)
    {
        XtVaSetValues(messagetext, XtNstring, pmessage, nullptr);
    }

    if (p == nullptr)
    {
        p = PROGRAMNAME " Warning";
    }

    XtVaSetValues(messageframe, XtNtitle, p, nullptr);
    XtManageChild(messagebutton2);
    XtResizeWidget(messageframe, width, height, 0);
    center_dialog(messageframe);
    XtVaSetValues(messagebutton1, XtNlabel, "Yes", nullptr);
    XtVaSetValues(messagebutton2, XtNlabel, "No", nullptr);
    XtPopupSpringLoaded(messageframe);
    XtSetSensitive(e2toplevel, False);
    XtSetSensitive(cpuframe, False);
}

void XtGui::popdown_message(Widget w)
{
    XtPopdown(messageframe);
    XtSetSensitive(e2toplevel, True);
    XtSetSensitive(cpuframe, True);

    // Check if it was the confirmation dialog
    if (XtIsManaged(messagebutton2) && (w == messagebutton1))
    {
        set_exit();
    }

    if (exit_flag)
    {
        request_new_state(CpuState::Exit);
    }
}

void XtGui::popup_disk_info(Widget w)
{
    if (is_menu_mode)
    {
        close_menu_mode();
        return;
    }

    std::string message;
    int i;

    for (i = 0; i < 4; ++i)
        if (w == floppybutton[i])
        {
            message = inout.get_drive_info(i);
            popup_message(message.c_str(), PROGRAMNAME " Disc status",
                          480, 160);
            return;
        }
}

void XtGui::popup_interrupt_info(Widget)
{
    if (is_menu_mode)
    {
        close_menu_mode();
        return;
    }

    std::stringstream message;
    tInterruptStatus s;

    scheduler.get_interrupt_status(s);
    message << "IRQ:   " << s.count[INT_IRQ] << std::endl
            << "FIRQ:  " << s.count[INT_FIRQ] << std::endl
            << "NMI:   " << s.count[INT_NMI] << std::endl
            << "RESET: " << s.count[INT_RESET] << std::endl;
    popup_message(message.str().c_str(),
                  PROGRAMNAME " Interrupt status", 480, 160);
}

void XtGui::toggle_frequency()
{
    float frequency;

    frequency_control_on = !frequency_control_on;
    frequency = frequency_control_on ? 1.3396 : 0.0;
    scheduler.sync_exec(BCommandPtr(new CSetFrequency(scheduler, frequency)));

    if (okpixmap != None)
        XtVaSetValues(entry27, XtNleftBitmap,
                      frequency_control_on ? okpixmap : None, nullptr);
}

void XtGui::toggle_undocumented()
{
    is_use_undocumented = !is_use_undocumented;
    cpu.set_use_undocumented(is_use_undocumented);

    if (okpixmap != None)
        XtVaSetValues(entry28, XtNleftBitmap,
                      is_use_undocumented ? okpixmap : None, nullptr);
}

void XtGui::update_disk_status(int floppyIndex, DiskStatus status)
{
#ifdef HAVE_XPM
    int iconIdx = PM_FLOPPY0;

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

    XtVaSetValues(floppybutton[floppyIndex], XtNbitmap,
                  pixmap[iconIdx], nullptr);
#else
    char *label = "";

    switch (status)
    {
        case DiskStatus::EMPTY:
            label = " ";
            break;

        case DiskStatus::INACTIVE:
            label = "O";
            break;

        case DiskStatus::ACTIVE:
            label = "X";
            break;
    }

    XtVaSetValues(floppybutton[floppyIndex], XtNlabel,
                  label, nullptr);
#endif
}

void XtGui::update_interrupt_status(tIrqType irqType, bool status)
{
#ifdef HAVE_XPM
    int iconIdx = PM_IRQ0;

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

    XtVaSetValues(interruptbutton, XtNbitmap, pixmap[iconIdx], nullptr);
#else
    char *label = " ";

    if (status)
    {
        switch (irqType)
        {
            case INT_IRQ:
                label = "I";
                break;

            case INT_FIRQ:
                label = "F";
                break;

            case INT_NMI:
                label = "N";
                break;

            case INT_RESET:
                label = "R";
                break;
        }
    }

    XtVaSetValues(interruptbutton, XtNlabel, label, nullptr);
#endif
}

void XtGui::timerCallback(XtIntervalId)
{
    // check if program can be savely shut down
    // Just send a dummy event here to let the
    // main_loop getting closed
    if (scheduler.is_finished())
    {
        XEvent event;

        memset(&event, 0, sizeof(XEvent));
        event.type = Expose;
        XSendEvent(getDisplay(), getWindow(FLX_E2SCREEN), False, 0,  &event);
    }

    static int count = 0;
    // check if floppy bitmap has to be updated
    count++;

    if (count % (100 / timebase) == 0)
    {
        static DiskStatus status[4];
        DiskStatus newStatus[4];
        static tInterruptStatus irqStat;
        tInterruptStatus newIrqStat;
        static bool firstTime = true;
        static bool lastState[INT_RESET + 1];
        bool bState;
        Word t;

        count = 0;
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
        update_cpuview(*status);

        if (okpixmap != None)
        {
            bool is_running = (status->state == CpuState::Run ||
                               status->state == CpuState::Next);
            XtVaSetValues(entry21, XtNleftBitmap,
                          is_running ? okpixmap : None, nullptr);
            XtVaSetValues(entry22, XtNleftBitmap,
                          !is_running ? okpixmap : None, nullptr);
        }

        if (status->state == CpuState::Invalid)
        {
            char err_msg[128];

            sprintf((char *)&err_msg, "\
Got invalid instruction pc=%04x instr=%02x %02x %02x %02x \
Processor stopped. To continue press Reset button",
                    status->pc,
                    status->instruction[0],
                    status->instruction[1],
                    status->instruction[2],
                    status->instruction[3]);
            popup_message(err_msg);
        }
    }

    // update graphic display
    int display_block;

    for (display_block = 0; display_block < YBLOCKS; display_block++)
    {
        update_block(display_block);
    }

    //XGetInputFocus(XtDisplay(e2toplevel), &child, &revert);
    mouse_update();

    // re-register timer callback
    XtAppAddTimeOut(context, timebase, timerCallbackProc, this);
}

void XtGui::mouse_update()
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
            mouse_warp(warp_home_x - current_x,
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

    joystickIO.put_value(convert_buttonmask(mouse_button_state));
    keyboardIO.put_value(convert_keymask(mouse_button_state));
}

void XtGui::mouse_warp(int dx, int dy)
{
    if (warp_dx || warp_dy || dx || dy)
    {
        warp_dx = dx;
        warp_dy = dy;
        XWarpPointer(getDisplay(), None, None, 0, 0, 0, 0, dx, dy);
    }
}

void XtGui::main_loop()
{
    //XtAppMainLoop(context);
    // Create own main loop to be able to exit
    // this function
    XEvent event;

    memset(&event, 0, sizeof(XEvent));

    while (!scheduler.is_finished())
    {
        XtAppNextEvent(context, &event);
        XtDispatchEvent(&event);
    }
}

void XtGui::c_expose(XEvent *pevent)
{
    // Event may be sent by a XSentEvent request
    // This is used as a hack to immediately finish the main_loop
    // In this case the event can be ignored
    if (pevent->xexpose.send_event)
    {
        return;
    }

    if (pevent->xexpose.count == 0 && XtIsRealized(e2screen))
    {
        memory.init_blocks_to_update();
    }
} // c_expose

void XtGui::c_keyPress(XEvent *pevent)
{
    SWord key;

    if (is_menu_mode)
    {
        char buffer[16];
        KeySym keysym;

        XLookupString((XKeyEvent *)pevent, buffer, sizeof(buffer) - 1, &keysym,
                      nullptr);
        if (keysym == XK_Escape)
        {
            close_menu_mode();
        }
        return;
    }

    if ((key = translate_to_ascii(&pevent->xkey)) >= 0)
    {
        bool do_notify = false;

        keyboardIO.put_char_parallel(key, do_notify);
        if (do_notify)
        {
            auto command = BCommandPtr(
                    new CActiveTransition(pia1, Mc6821::ControlLine::CA1));
            scheduler.sync_exec(std::move(command));
        }
    }
} // c_keyPress

void XtGui::c_motion(XEvent *pevent)
{
    XPointerMovedEvent *pm_event = (XPointerMovedEvent *) pevent;
    current_x = pm_event->x;
    current_y = pm_event->y;
    mouse_button_state = pm_event->state;
} // c_motion

void XtGui::c_enter(XEvent *pevent)
{
    XEnterWindowEvent *enter_event = (XEnterWindowEvent *)pevent;

    current_x = prev_x = enter_event->x;
    current_y = prev_y = enter_event->y;
    mouse_button_state = enter_event->state;
} // c_enter

void XtGui::c_leave(XEvent *pevent)
{
    XLeaveWindowEvent *leave_event = (XEnterWindowEvent *)pevent;

    current_x = prev_x = -1;
    current_y = prev_y = -1;
    mouse_button_state = leave_event->state;
} // c_leave

void XtGui::c_buttonPress(XEvent *pevent)
{
    if (is_menu_mode)
    {
        close_menu_mode();
        return;
    }

    // just catch event to prevent to get cached by the
    // window manager
    XButtonEvent *button_event = (XButtonEvent *)pevent;
    current_x = button_event->x;
    current_y = button_event->y;
    mouse_button_state = button_event->state;

    switch (button_event->button)
    {
        case Button1:
            mouse_button_state |= Button1Mask;
            break;

        case Button2:
            mouse_button_state |= Button2Mask;
            break;

        case Button3:
            mouse_button_state |= Button3Mask;
            break;

        case Button4:
            mouse_button_state |= Button4Mask;
            break;

        case Button5:
            mouse_button_state |= Button5Mask;
            break;
    }
} // c_buttonPress

void XtGui::c_buttonRelease(XEvent *pevent)
{
    if (is_menu_mode)
    {
        close_menu_mode();
        return;
    }

    XButtonEvent *button_event = (XButtonEvent *)pevent;

    current_x = button_event->x;
    current_y = button_event->y;
    mouse_button_state = button_event->state;

    switch (button_event->button)
    {
        case Button1:
            mouse_button_state &= ~Button1Mask;
            break;

        case Button2:
            mouse_button_state &= ~Button2Mask;
            break;

        case Button3:
            mouse_button_state &= ~Button3Mask;
            break;

        case Button4:
            mouse_button_state &= ~Button4Mask;
            break;

        case Button5:
            mouse_button_state &= ~Button5Mask;
            break;
    }
} // c_buttonRelease

void XtGui::c_focusIn(XEvent *pevent)
{
    XAbstractGui::c_focusIn(pevent);
    // set SERPAR-Flag of Monitor to parallel
    // sorry, it's very dirty but it's implemented for
    // the most convenience of the user and it's ONLY
    // enabled if the default monitor file "neumon54.hex"
    // is used
    // output_to_graphic();
} // c_focusIn

void XtGui::c_focusOut(XEvent *pevent)
{
    XAbstractGui::c_focusOut(pevent);
} // c_focusOut

void XtGui::c_wm_protocols(XEvent *pevent)
{
    // Only do a propper shut down of the application
    // when clicking on main window
    if (pevent->type == ClientMessage &&
        ((Atom)pevent->xclient.data.l[0] == wm_delete_window) &&
        (pevent->xclient.window == XtWindow(e2toplevel)))
    {
        popup_confirmation("Do you really want to close " PROGRAMNAME);
    }
} // c_wm_protocols

void XtGui::c_close_menu_mode(XEvent *)
{
    close_menu_mode();
} // c_close_menu_mode

void XtGui::c_highlight_child(Widget w, XEvent *pevent, String *params)
{
    Position x, y;
    Dimension wi, he;

    x = y = 0;
    XtVaGetValues(w, XtNx, &x, XtNy, &y, XtNwidth, &wi, XtNheight, &he, nullptr);
    x = pevent->xbutton.x_root - x;
    y = pevent->xbutton.y_root - y;

    if (pevent->type == ButtonPress || pevent->type == MotionNotify)
    {
        if (x >= 0 && x <= wi && y >= 0 && y <= he)
        {
            XtCallActionProc(w, "highlight", pevent, params, 0);
        }
        else
        {
            XtCallActionProc(w, "unhighlight", pevent, params, 0);
        }
    }
} // c_highlight_child

void XtGui::initialize_e2window(struct sGuiOptions &options)
{
    Widget w;

    frequency_control_on = false;
    is_use_undocumented  = false;
    is_menu_mode = false;
    menu_popped_up = None;
    initialize_conv_tables();
    w = create_main_view(options.argc, options.argv,
                         options.isSynchronized);
    create_message_dialog(w);
    create_about_dialog(w);
#ifdef HAVE_XPM
    create_author_dialog(w);
#endif
    create_cpuview(w);
    create_bp_dialog(w);
    create_logfile_dialog(w);
    initialize_after_create(w, options.isInverse, options.color.c_str());
    manage_widget(w);
    initialize_after_open(w, get_title());
    e2toplevel = w;
} // initialize_e2window

// The menus are kept open when releasing the mouse button.
// This is realized by adding a menu handler.
void XtGui::add_menu_handler(Widget button, Widget /*menu*/)
{
    XtAddEventHandler(button,
                      ButtonPressMask | KeyPressMask | KeyReleaseMask |
                      EnterWindowMask | LeaveWindowMask,
                      False, (XtEventHandler)menuHandlerCallback,
                      (XtPointer)this);
}

Widget XtGui::create_main_view(int argc, char *const argv[],
                               bool isSynchronized)
{
    int i;
    Widget mainview;

    static XtActionsRec actions[] =
    {
#ifdef HAVE_XPM
        {const_cast<String>("author_expose"), (XtActionProc)author_expose},
#endif
        {const_cast<String>("focusInMain"), (XtActionProc)focusInMain},
        {const_cast<String>("focusOutMain"), (XtActionProc)focusOutMain},
        {const_cast<String>("expose"), (XtActionProc)expose},
        {const_cast<String>("keyPress"), (XtActionProc)keyPress},
        {const_cast<String>("buttonPress"), (XtActionProc)buttonPress},
        {const_cast<String>("buttonRelease"), (XtActionProc)buttonRelease},
        {const_cast<String>("motion"), (XtActionProc)motion},
        {const_cast<String>("enter"), (XtActionProc)enter},
        {const_cast<String>("leave"), (XtActionProc)leave},
        {const_cast<String>("convert"), (XtActionProc)keyPress},
        {
            const_cast<String>("popup_no_resources"),
            (XtActionProc)popup_no_resources
        },
        {const_cast<String>("resize"), (XtActionProc)processResize},
        {const_cast<String>("wm_protocols"), (XtActionProc)wm_protocols},
        {const_cast<String>("highlightChild"), (XtActionProc)highlightChild},
        {const_cast<String>("closeMenuMode"), (XtActionProc)closeMenuMode},
    };

    XtSetLanguageProc(nullptr, nullptr, nullptr); // set to default language

    // create widget tree:
    mainview = XtVaAppInitialize(&context, "Flexemu",
                                 opts, 0, &argc, const_cast<char **>(argv),
                                 fallback_resources, nullptr);

    if (isSynchronized)
    {
        XSynchronize(XtDisplay(mainview), True);
    }

    XtAppAddConverter(context, XtRString, XtROrientation,
                      XmuCvtStringToOrientation, nullptr, 0);
    XtAppAddActions(context, actions, XtNumber(actions));

    // use a Paned widget because it doesn't resize the menuBar
    form = XtVaCreateManagedWidget("form", formWidgetClass, mainview,
                                   nullptr);
    menubar = XtVaCreateManagedWidget("menuBar", boxWidgetClass, form,
                                      nullptr);
    button[0] = XtVaCreateManagedWidget("menuButton1", menuButtonWidgetClass,
                                      menubar, nullptr);
    button[1] = XtVaCreateManagedWidget("menuButton2", menuButtonWidgetClass,
                                      menubar, nullptr);
    button[2] = XtVaCreateManagedWidget("menuButton3", menuButtonWidgetClass,
                                      menubar, nullptr);
    e2screen = XtVaCreateManagedWidget("screen", coreWidgetClass, form,
                                       XtNwidth,
                                       (XtArgVal)WINDOWWIDTH * pixelSizeX,
                                       XtNheight,
                                       (XtArgVal)WINDOWHEIGHT * pixelSizeY, nullptr);
    statusbuttons = XtVaCreateManagedWidget("statusButtons",
                                            boxWidgetClass, form, nullptr);
    menu[0] = XtVaCreatePopupShell("menu1", simpleMenuWidgetClass, button[0],
                                 nullptr);
    add_menu_handler(button[0], menu[0]);
    menu[1] = XtVaCreatePopupShell("menu2", simpleMenuWidgetClass, button[1],
                                 nullptr);
    add_menu_handler(button[1], menu[1]);
    menu[2] = XtVaCreatePopupShell("menu3", simpleMenuWidgetClass, button[2],
                                 nullptr);
    add_menu_handler(button[2], menu[2]);
    entry12 = XtVaCreateManagedWidget("menuEntry12", smeBSBObjectClass,
                                      menu[0], nullptr);
    entry21 = XtVaCreateManagedWidget("menuEntry21", smeBSBObjectClass,
                                      menu[1], nullptr);
    entry22 = XtVaCreateManagedWidget("menuEntry22", smeBSBObjectClass,
                                      menu[1], nullptr);
    entry23 = XtVaCreateManagedWidget("menuEntry23", smeBSBObjectClass,
                                      menu[1], nullptr);
    line21  = XtVaCreateManagedWidget("line21", smeLineObjectClass,
                                      menu[1], nullptr);
    entry24 = XtVaCreateManagedWidget("menuEntry24", smeBSBObjectClass,
                                      menu[1], nullptr);
    entry25 = XtVaCreateManagedWidget("menuEntry25", smeBSBObjectClass,
                                      menu[1], nullptr);
    entry26 = XtVaCreateManagedWidget("menuEntry26", smeBSBObjectClass,
                                      menu[1], nullptr);
    line22 = XtVaCreateManagedWidget("line22", smeLineObjectClass,
                                     menu[1], nullptr);
    entry27 = XtVaCreateManagedWidget("menuEntry27", smeBSBObjectClass,
                                      menu[1], nullptr);
    entry28 = XtVaCreateManagedWidget("menuEntry28", smeBSBObjectClass,
                                      menu[1], nullptr);
    entry31 = XtVaCreateManagedWidget("menuEntry31", smeBSBObjectClass,
                                      menu[2], nullptr);
    entry32 = XtVaCreateManagedWidget("menuEntry32", smeBSBObjectClass,
                                      menu[2], nullptr);
    floppybutton[0] = XtVaCreateManagedWidget("floppyButton0",
                      commandWidgetClass, statusbuttons, nullptr);
    floppybutton[1] = XtVaCreateManagedWidget("floppyButton1",
                      commandWidgetClass, statusbuttons, nullptr);
    floppybutton[2] = XtVaCreateManagedWidget("floppyButton2",
                      commandWidgetClass, statusbuttons, nullptr);
    floppybutton[3] = XtVaCreateManagedWidget("floppyButton3",
                      commandWidgetClass, statusbuttons, nullptr);
    interruptbutton = XtVaCreateManagedWidget("interruptButton",
                      commandWidgetClass, statusbuttons, nullptr);
    // must realize it now
    // to get valid contents of win
    //XtSetMappedWhenManaged(mainview, False);
    XtRealizeWidget(mainview);
    wm_delete_window = XInternAtom(getDisplay(), "WM_DELETE_WINDOW", False);
    XSetWMProtocols(getDisplay(), XtWindow(mainview), &wm_delete_window, 1);
    SetColors(XtDisplay(mainview));
    Window win = XtWindow(mainview);
    XSetWindowColormap(XtDisplay(mainview), win, cmap);

    // add some menu callbacks:
    XtAddCallback(entry12, XtNcallback, setCpuExitCallback,
                  (XtPointer)this);
    XtAddCallback(entry21, XtNcallback, setCpuRunCallback,
                  (XtPointer)this);
    XtAddCallback(entry22, XtNcallback, setCpuStopCallback,
                  (XtPointer)this);
    XtAddCallback(entry23, XtNcallback, setCpuResetRunCallback,
                  (XtPointer)this);
    XtAddCallback(entry24, XtNcallback, toggleCpuCallback,
                  (XtPointer)this);
    XtAddCallback(entry25, XtNcallback, popupBpCallback,
                  (XtPointer)this);
    XtAddCallback(entry26, XtNcallback, popupLogfileCallback,
                  (XtPointer)this);
    XtAddCallback(entry27, XtNcallback, toggleFrequencyCallback,
                  (XtPointer)this);
    XtAddCallback(entry28, XtNcallback, toggleUndocumentedCallback,
                  (XtPointer)this);
    XtAddCallback(entry31, XtNcallback, popupHelpCallback,
                  (XtPointer)this);
    XtAddCallback(entry32, XtNcallback, popupAboutCallback,
                  (XtPointer)this);

    for (i = 0; i < 4; ++i)
        XtAddCallback(floppybutton[i], XtNcallback,
                      popupFloppyCallback, (XtPointer)this);

    XtAddCallback(interruptbutton, XtNcallback,
                  popupInterruptCallback, (XtPointer)this);
    return mainview;
}  // create_main_view


void XtGui::create_about_dialog(Widget parent)
{
    char            *allocstring;
    const char      *aboutstring;
    allocstring = XtMalloc(sizeof(PROGRAM_VERSION) + 2 + sizeof(HEADER1) +
                           sizeof(HEADER2));

    if (allocstring != nullptr)
    {
        strcpy(allocstring, HEADER1);
        strcat(allocstring, "V " PROGRAM_VERSION);
        strcat(allocstring, HEADER2);
        aboutstring = allocstring;
    }
    else
    {
        aboutstring = "not enough memory";
    }

    aboutframe = XtVaCreatePopupShell("about",
                                      transientShellWidgetClass, parent, nullptr);
    aboutform = XtVaCreateManagedWidget("aboutForm",
                                        formWidgetClass, aboutframe, nullptr);
    abouttext = XtVaCreateManagedWidget("aboutText", asciiTextWidgetClass,
                                        aboutform,
                                        XtNstring, (XtArgVal)aboutstring,
                                        XtNlength,
                                        (XtArgVal)(strlen(aboutstring) + 1),
                                        nullptr);
    aboutbuttons = XtVaCreateManagedWidget("aboutButtons",
                                           boxWidgetClass, aboutform, nullptr);
    aboutbutton = XtVaCreateManagedWidget("aboutButton", commandWidgetClass,
                                          aboutbuttons, nullptr);
#ifdef HAVE_XPM
    morebutton = XtVaCreateManagedWidget("moreButton", commandWidgetClass,
                                         aboutbuttons, nullptr);
#endif

    // define some button callbacks:
    XtAddCallback(aboutbutton, XtNcallback, popdownAboutCallback,
                  (XtPointer)this);
#ifdef HAVE_XPM
    XtAddCallback(morebutton, XtNcallback, popupAuthorCallback,
                  (XtPointer)this);
#endif
    XtFree(allocstring);
    XtRealizeWidget(aboutframe);
    XSetWMProtocols(getDisplay(), XtWindow(aboutframe), &wm_delete_window, 1);
} // create_about_dialog

#ifdef HAVE_XPM
void XtGui::create_author_dialog(Widget parent)
{
    authorpixmap = None;
    authorframe = XtVaCreatePopupShell("author", transientShellWidgetClass,
                                       parent, nullptr);
    authorform = XtVaCreateManagedWidget("authorForm", formWidgetClass,
                                         authorframe, nullptr);
    authorwidget = XtVaCreateManagedWidget("authorWidget", coreWidgetClass,
                                           authorform, nullptr);
    authorbutton = XtVaCreateManagedWidget("authorButton", commandWidgetClass,
                                           authorform, nullptr);

    // define some button callbacks:
    XtAddCallback(authorbutton, XtNcallback, popdownAuthorCallback,
                  (XtPointer)this);

    XtRealizeWidget(authorframe);
    XSetWMProtocols(getDisplay(), XtWindow(authorframe), &wm_delete_window, 1);
} // create_author_dialog
#endif

void XtGui::create_message_dialog(Widget parent)
{
    messageframe = XtVaCreatePopupShell("message", transientShellWidgetClass,
                                        parent, nullptr);
    messageform = XtVaCreateManagedWidget("messageForm", formWidgetClass,
                                          messageframe, nullptr);
    messagetext = XtVaCreateManagedWidget("messageText", asciiTextWidgetClass,
                                          messageform, nullptr);
    messagebuttons = XtVaCreateManagedWidget("messageButtons", boxWidgetClass,
                     messageform, nullptr);
    messagebutton1 = XtVaCreateManagedWidget("messageButton1",
                     commandWidgetClass,
                     messagebuttons, nullptr);
    messagebutton2 = XtVaCreateManagedWidget("messageButton2",
                     commandWidgetClass,
                     messagebuttons, nullptr);

    XtVaSetValues(messageframe, XtCTransientFor, parent, nullptr);
    XtVaSetValues(messagetext, XtNwrap, XawtextWrapWord, nullptr);

    // define some button callbacks:
    XtAddCallback(messagebutton1, XtNcallback, popdownMessageCallback,
                  (XtPointer)this);
    XtAddCallback(messagebutton2, XtNcallback, popdownMessageCallback,
                  (XtPointer)this);

    XtRealizeWidget(messageframe);
    XSetWMProtocols(getDisplay(), XtWindow(messageframe), &wm_delete_window, 1);
} // create_message_dialog

void XtGui::initialize_after_create(Widget w, bool isInverse, const char *color)
{
    unsigned int i, j;
    Display *display;
    Screen *screen;
    Visual *visual;
    XGCValues gcv;
    XWMHints *wm_hints;
    XColor xcolor, exact_color;

    oldX = 0;
    oldY = 0;

    display = XtDisplay(w);
    screen = XtScreen(w);
    visual = DefaultVisualOfScreen(screen);

    mainpixmap = None;
#ifdef HAVE_XPM
    XpmAttributes xpmattr;
    int errorStatus;
    char **data;

    data = const_cast<char **>(flexemu);
    xpmattr.colormap = DefaultColormapOfScreen(XtScreen(w));
    xpmattr.valuemask = XpmColormap;
    errorStatus = XpmCreatePixmapFromData(XtDisplay(w),
                                          XtWindow(w), data, &mainpixmap,
                                          nullptr, &xpmattr);
    XpmFreeAttributes(&xpmattr);

    if (errorStatus != XpmSuccess)
#endif
        mainpixmap = XCreateBitmapFromData(XtDisplay(w),
                                           RootWindowOfScreen(screen),
                                           (char *)flexmain_bits,
                                           flexmain_width, flexmain_height);

    if (mainpixmap != None)
    {
        XtVaSetValues(w, XtNiconPixmap, mainpixmap, nullptr);
    }

    if (!*color || !stricmp(color, "default"))
    {
        color = "green";
    }

    if (!XAllocNamedColor(display, DefaultColormapOfScreen(screen),
                          color, &xcolor, &exact_color))
    {
        xcolor.pixel = WhitePixelOfScreen(screen);
    }

    if (isInverse)
    {
        gcv.foreground = BlackPixelOfScreen(screen);
        gcv.background = xcolor.pixel;
    }
    else
    {
        gcv.foreground = xcolor.pixel;
        gcv.background = BlackPixelOfScreen(screen);
    }

#ifdef HAVE_XPM
    authorgc = XtGetGC(e2screen, GCForeground | GCBackground, &gcv);
#endif
    e2gc = XtGetGC(e2screen, GCForeground | GCBackground, &gcv);
    XtVaSetValues(e2screen, XtNbackground, gcv.background, nullptr);

    const size_t size = WINDOWWIDTH * BLOCKHEIGHT *
                        MAX_PIXELSIZEX * MAX_PIXELSIZEY *
                        32 / 8; // max. screen depth
    image_data = new Byte[size];
    memset(image_data, 0, size);

    // initialize different images used for different window sizes:
    for (i = 0; i < MAX_PIXELSIZEX; i++)
    {
        for (j = 0; j < MAX_PIXELSIZEY; j++)
        {
            image[i][j] = XCreateImage(display, visual, depth,
                                       ZPixmap, 0, (char *)image_data,
                                       BLOCKWIDTH * (i + 1),
                                       BLOCKHEIGHT * (j + 1), 32, 0);

            switch (depth)
            {
                case  1:
                case  8:
                    image[i][j]->bitmap_unit = 8;
                    break;

                case 15:
                case 16:
                    image[i][j]->bitmap_unit = 16;
                    break;

                case 24:
                case 32:
                    image[i][j]->bitmap_unit = 32;
                    break;
            }

            image[i][j]->bitmap_bit_order = MSBFirst;
#ifdef WORDS_BIGENDIAN
            image[i][j]->byte_order = MSBFirst;
#else
            image[i][j]->byte_order = LSBFirst;
#endif
            _XInitImageFuncPtrs(image[i][j]);
        } // for
    } // for

    wm_hints = XAllocWMHints();
    wm_hints->initial_state = NormalState;
    wm_hints->input         = True;
    wm_hints->icon_pixmap   = mainpixmap;
    wm_hints->flags = StateHint | IconPixmapHint | InputHint;
    XSetWMHints(XtDisplay(w), XtWindow(w), wm_hints);
    XFree(wm_hints);
    XtAppAddTimeOut(context, timebase, timerCallbackProc, this);

    // needed for propper expose:
    XtInstallAllAccelerators(e2screen, form);
#ifdef HAVE_XPM
    XtInstallAllAccelerators(authorwidget, authorframe);
#endif
}

void XtGui::manage_widget(Widget w)
{
    Mc6809CpuStatus stat;
    stat.mnemonic[0] = '\0';
    XtMapWidget(w);
    memory.init_blocks_to_update();    // update main view
    update_cpuview(stat);
}  // manage widget


void XtGui::initialize_after_open(Widget w, const char *title)
{
    Dimension   width, height;
    XSizeHints  *pxsh;

    warp_home_x = (pixelSizeX * WINDOWWIDTH)  >> 1;
    warp_home_y = (pixelSizeY * WINDOWHEIGHT) >> 1;
#ifdef HAVE_XPM
    Pixel bg_color;

    XtVaGetValues(interruptbutton, XtNbackground, &bg_color, nullptr);
    create_pixmaps(w, bg_color);
    XtVaSetValues(floppybutton[0], XtNbitmap, pixmap[PM_FLOPPY0], nullptr);
    XtVaSetValues(floppybutton[1], XtNbitmap, pixmap[PM_FLOPPY0], nullptr);
    XtVaSetValues(floppybutton[2], XtNbitmap, pixmap[PM_FLOPPY0], nullptr);
    XtVaSetValues(floppybutton[3], XtNbitmap, pixmap[PM_FLOPPY0], nullptr);
    XtVaSetValues(interruptbutton, XtNbitmap, pixmap[PM_IRQ0],    nullptr);
#endif
    okpixmap = XCreateBitmapFromData(XtDisplay(w),
                                     RootWindowOfScreen(XtScreen(w)),
                                     (char *)ok_bits, ok_width, ok_height);

    if (okpixmap != None && cpu.is_use_undocumented())
    {
        XtVaSetValues(entry28, XtNleftBitmap, okpixmap, nullptr);
    }

    XtVaGetValues(w, XtNwidth, &width, XtNheight, &height, nullptr);
    pxsh = XAllocSizeHints();

    if (pxsh != nullptr)
    {
        pxsh->min_width = WINDOWWIDTH + (width % WINDOWWIDTH);
        pxsh->max_width = WINDOWWIDTH * MAX_PIXELSIZEX +
                           (width % WINDOWWIDTH);
        pxsh->min_height = WINDOWHEIGHT + (height % WINDOWHEIGHT);;
        pxsh->max_height = WINDOWHEIGHT * MAX_PIXELSIZEY +
                           (height % WINDOWHEIGHT);
        pxsh->width_inc = WINDOWWIDTH;
        pxsh->height_inc = WINDOWHEIGHT;
        pxsh->win_gravity = NorthWestGravity;
        pxsh->flags = PMaxSize | PMinSize | PResizeInc | PWinGravity;
        XSetStandardProperties(XtDisplay(w), XtWindow(w),
                               title, title, None, nullptr, 0, pxsh);
        XFree(pxsh);
    }
}

Display *XtGui::getDisplay()
{
    return XtDisplay(e2screen);
}

Window XtGui::getWindow(tWindowType t /* = FLX_E2SCREEN */)
{
    switch (t)
    {
        case FLX_MAIN:
            return XtWindow(e2toplevel);

        case FLX_E2SCREEN:
        default:
            return XtWindow(e2screen);
    }
}

GuiType XtGui::gui_type()
{
    return GuiType::XTOOLKIT;
}

XtGui::XtGui(
    Mc6809 &x_cpu,
    Memory &x_memory,
    Scheduler &x_scheduler,
    Inout &x_inout,
    VideoControl1 &x_vico1,
    VideoControl2 &x_vico2,
    JoystickIO &x_joystickIO,
    KeyboardIO &x_keyboardIO,
    TerminalIO &x_terminalIO,
    Pia1 &x_pia1,
    struct sGuiOptions &x_options)
        : XAbstractGui(
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
{
    initialize(options);
}

XtGui::~XtGui()
{
    int i, j;

    if (okpixmap != None)
    {
        XFreePixmap(XtDisplay(e2toplevel), okpixmap);
    }

    if (mainpixmap != None)
    {
        XFreePixmap(XtDisplay(e2toplevel), mainpixmap);
    }

    if (cpupixmap != None)
    {
        XFreePixmap(XtDisplay(e2toplevel), cpupixmap);
    }

    // remove data pointer otherwise it would be freed
    for (i = 0; i < MAX_PIXELSIZEX; i++)
    {
        for (j = 0; j < MAX_PIXELSIZEY; j++)
        {
            image[i][j]->data = nullptr;
            XDestroyImage(image[i][j]);
        } // for
    } // for

#ifdef HAVE_XPM

    if (authorpixmap != None)
    {
        XFreePixmap(XtDisplay(e2toplevel), authorpixmap);
    }

    for (i = PM_FLOPPY0; i <= PM_IRQ4; i++)
        if (pixmap[i] != None)
        {
            XFreePixmap(XtDisplay(e2toplevel), pixmap[i]);
        }

    XtReleaseGC(e2screen, authorgc);
#endif
    XtReleaseGC(e2screen, e2gc);

    if (own_cmap)
    {
        XFreeColormap(getDisplay(), cmap);
    }

    XtDestroyWidget(e2toplevel); /* Avoid memory leak */
    XtDestroyApplicationContext(context);
    delete [] image_data;
    image_data = nullptr;
    ggui = nullptr;
}


/******************************/
/* CPU-view implementation    */
/******************************/

void XtGui::popup_cpu()
{
    Position x, y;
    Dimension width, topheight, height;
#ifndef FASTFLEX
    const char *title = "Mc6809";
#else
    const char *title = "Mc6809 (L.C. Benschop)";
#endif

    XtVaGetValues(e2toplevel, XtNwidth, &width, XtNheight, &topheight,
                  XtNx, &x, XtNy, &y, nullptr);
    x += width >> 1;
    XtVaGetValues(cpuframe, XtNwidth, &width, XtNheight, &height, nullptr);
    x -= width >> 1;
    y -= height;

    if (y < 0)
    {
        y += height + topheight;
    }

    XtVaSetValues(cpuframe, XtNx, x, XtNy, y, XtNtitle, title, nullptr);
    XtPopup(cpuframe, XtGrabNone);

    if (okpixmap != None)
    {
        XtVaSetValues(entry24, XtNleftBitmap, okpixmap, nullptr);
    }

    cpu_popped_up = true;
}

void XtGui::popdown_cpu()
{
    XtVaSetValues(entry24, XtNleftBitmap, None, nullptr);
    XtPopdown(cpuframe);
    cpu_popped_up = false;
}

void XtGui::toggle_cpu()
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

void XtGui::popup_bp()
{
    String bpstring;
    unsigned int which;

    bpstring = (String)XtMalloc(16);

    for (which = 0; which <= 1; which++)
    {
        if (!cpu.is_bp_set(which))
        {
            strcpy(bpstring, "");
        }
        else
        {
            sprintf(bpstring, "%04x", cpu.get_bp(which));
        }

        XtVaSetValues(bptext[which], XtNlength, 6,
                      XtNstring, (XtArgVal)bpstring, nullptr);
    }

    center_dialog(bpframe);
    XtPopup(bpframe, XtGrabExclusive);
    XtSetSensitive(e2toplevel, False);
    XtSetSensitive(cpuframe, False);
    XtFree(bpstring);
}

void XtGui::popdown_bp(Widget w)
{
    String bpstring;
    unsigned int addr, which;

    if (w != bpokbutton)
    {
        XtPopdown(bpframe);
        XtSetSensitive(e2toplevel, True);
        XtSetSensitive(cpuframe, True);
        return;
    }

    for (which = 0; which <= 1; which++)
    {
        XtVaGetValues(bptext[which], XtNstring, &bpstring, nullptr);

        if (sscanf(bpstring, "%x", &addr) == 1 && addr <= 0xffff)
        {
            cpu.set_bp(which, (Word)addr);
        }
        else if (strlen(bpstring) == 0)
        {
            cpu.reset_bp(which);
        }
    }

    XtPopdown(bpframe);
    XtSetSensitive(e2toplevel, True);
    XtSetSensitive(cpuframe, True);
}

void XtGui::clear_bp()
{
    unsigned int which;

    for (which = 0; which <= 1; which++)
    {
        XtVaSetValues(bptext[which], XtNstring, (XtArgVal)"", nullptr);
    }
}

void XtGui::popup_log()
{
    String tmpstring = (String)XtMalloc(PATH_MAX);

    int which = 0;
    // Min Address
    tmpstring[0] = '\0';

    if (lfs.minAddr < 0x10000)
    {
        sprintf(tmpstring, "%04x", lfs.minAddr);
    }

    XtVaSetValues(logtext[which++], XtNlength, 6,
                  XtNstring, (XtArgVal)tmpstring, nullptr);
    // Max Address
    tmpstring[0] = '\0';

    if (lfs.maxAddr < 0x10000)
    {
        sprintf(tmpstring, "%04x", lfs.maxAddr);
    }

    XtVaSetValues(logtext[which++], XtNlength, 6,
                  XtNstring, (XtArgVal)tmpstring, nullptr);
    // Start Address
    tmpstring[0] = '\0';

    if (lfs.startAddr < 0x10000)
    {
        sprintf(tmpstring, "%04x", lfs.startAddr);
    }

    XtVaSetValues(logtext[which++], XtNlength, 6,
                  XtNstring, (XtArgVal)tmpstring, nullptr);
    // Stop Address
    tmpstring[0] = '\0';

    if (lfs.stopAddr < 0x10000)
    {
        sprintf(tmpstring, "%04x", lfs.stopAddr);
    }

    XtVaSetValues(logtext[which++], XtNlength, 6,
                  XtNstring, (XtArgVal)tmpstring, nullptr);
    // Log Filename
    strcpy(tmpstring, lfs.logFileName.c_str());
    XtVaSetValues(logfilename, XtNlength, 6,
                  XtNstring, (XtArgVal)tmpstring, nullptr);

    center_dialog(logframe);
    XtPopup(logframe, XtGrabExclusive);
    XtSetSensitive(e2toplevel, False);
    XtSetSensitive(cpuframe, False);
    XtFree(tmpstring);
}

void XtGui::popdown_log(Widget w)
{
    String tmpstring;
    unsigned int addr, which;

    if (w != logokbutton)
    {
        XtPopdown(logframe);
        XtSetSensitive(e2toplevel, True);
        XtSetSensitive(cpuframe, True);
        return;
    }

    which = 0;
    // Min Address
    XtVaGetValues(logtext[which++], XtNstring, &tmpstring, nullptr);
    lfs.minAddr = 0x0000;

    if (sscanf(tmpstring, "%x", &addr) == 1 && addr <= 0xffff)
    {
        lfs.minAddr = addr;
    }

    // Max Address
    XtVaGetValues(logtext[which++], XtNstring, &tmpstring, nullptr);
    lfs.maxAddr = 0xFFFF;

    if (sscanf(tmpstring, "%x", &addr) == 1 && addr <= 0xffff)
    {
        lfs.maxAddr = addr;
    }

    // Start Address
    XtVaGetValues(logtext[which++], XtNstring, &tmpstring, nullptr);
    lfs.startAddr = 0x10000;

    if (sscanf(tmpstring, "%x", &addr) == 1 && addr <= 0xffff)
    {
        lfs.startAddr = addr;
    }

    // Stop Address
    XtVaGetValues(logtext[which++], XtNstring, &tmpstring, nullptr);
    lfs.stopAddr = 0x10000;

    if (sscanf(tmpstring, "%x", &addr) == 1 && addr <= 0xffff)
    {
        lfs.stopAddr = addr;
    }

    // Log Filename
    XtVaGetValues(logfilename, XtNstring, &tmpstring, nullptr);
    lfs.logFileName = tmpstring;

    scheduler.sync_exec(BCommandPtr(new CSetLogFile(cpu, lfs)));

    XtPopdown(logframe);
    XtSetSensitive(e2toplevel, True);
    XtSetSensitive(cpuframe, True);
}

void XtGui::clear_log()
{
    unsigned int which;

    for (which = 0; which <= 3; which++)
    {
        XtVaSetValues(logtext[which], XtNstring, (XtArgVal)"", nullptr);
    }

    XtVaSetValues(logfilename, XtNstring, (XtArgVal)"", nullptr);
}

void XtGui::create_cpuview(Widget parent)
{
    XtArgVal argVal;

    cpu_popped_up = false;
    cpuframe = XtVaCreatePopupShell("MC6809", topLevelShellWidgetClass,
                                    parent, nullptr);
    cpuform = XtVaCreateManagedWidget("cpuForm", formWidgetClass, cpuframe,
                                      nullptr);
    cputext = XtVaCreateManagedWidget("cpuText", asciiTextWidgetClass, cpuform,
                                      nullptr);
    cpubuttons = XtVaCreateManagedWidget("cpuButtons", boxWidgetClass, cpuform,
                                         nullptr);
    argVal = (XtArgVal)&radio_data[static_cast<Byte>(CpuState::Run)];
    runbutton = XtVaCreateManagedWidget("runButton", toggleWidgetClass,
                                        cpubuttons, XtNradioData,
                                        argVal,
                                        XtNstate, (XtArgVal)1, nullptr);
    argVal = (XtArgVal)&radio_data[static_cast<Byte>(CpuState::Stop)];
    stopbutton = XtVaCreateManagedWidget("stopButton", toggleWidgetClass,
                                         cpubuttons, XtNradioGroup,
                                         (XtArgVal)runbutton, XtNradioData,
                                         argVal,
                                         nullptr);
    argVal = (XtArgVal)&radio_data[static_cast<Byte>(CpuState::Step)];
    stepbutton = XtVaCreateManagedWidget("stepButton", toggleWidgetClass,
                                         cpubuttons, XtNradioGroup,
                                         (XtArgVal)runbutton, XtNradioData,
                                         argVal,
                                         nullptr);
    argVal = (XtArgVal)&radio_data[static_cast<Byte>(CpuState::Next)];
    nextbutton = XtVaCreateManagedWidget("nextButton", toggleWidgetClass,
                                         cpubuttons, XtNradioGroup,
                                         (XtArgVal)runbutton, XtNradioData,
                                         argVal,
                                         nullptr);
    argVal = (XtArgVal)&radio_data[static_cast<Byte>(CpuState::Reset)];
    resetbutton = XtVaCreateManagedWidget("resetButton", toggleWidgetClass,
                                          cpubuttons, XtNradioGroup,
                                          (XtArgVal)runbutton, XtNradioData,
                                          argVal,
                                          nullptr);
    bpbutton = XtVaCreateManagedWidget("bpButton", commandWidgetClass,
                                       cpubuttons, nullptr);
    logbutton = XtVaCreateManagedWidget("logButton", commandWidgetClass,
                                        cpubuttons, nullptr);
    cpubutton = XtVaCreateManagedWidget("cpuButton", commandWidgetClass,
                                        cpubuttons, nullptr);

    XtAddCallback(runbutton,   XtNcallback, toggleCpuRunCallback,
                  (XtPointer)this);
    XtAddCallback(stopbutton,  XtNcallback, toggleCpuStopCallback,
                  (XtPointer)this);
    XtAddCallback(stepbutton,  XtNcallback, toggleCpuStepCallback,
                  (XtPointer)this);
    XtAddCallback(nextbutton,  XtNcallback, toggleCpuNextCallback,
                  (XtPointer)this);
    XtAddCallback(resetbutton, XtNcallback, toggleCpuResetCallback,
                  (XtPointer)this);
    XtAddCallback(bpbutton,    XtNcallback, popupBpCallback,
                  (XtPointer)this);
    XtAddCallback(logbutton,   XtNcallback, popupLogfileCallback,
                  (XtPointer)this);
    XtAddCallback(cpubutton,   XtNcallback, popdownCpuCallback,
                  (XtPointer)this);

    XtRealizeWidget(cpuframe);
    XSetWMProtocols(getDisplay(), XtWindow(cpuframe), &wm_delete_window, 1);
    cpupixmap = None;
#ifdef HAVE_XPM
    XpmAttributes xpmattr;
    int errorStatus;
    char **data;

    xpmattr.colormap = DefaultColormapOfScreen(XtScreen(cpuframe));
    xpmattr.valuemask = XpmColormap;
    data = const_cast<char **>(flexcpu);
    errorStatus = XpmCreatePixmapFromData(XtDisplay(cpuframe),
                                          XtWindow(cpuframe), data, &cpupixmap,
                                          nullptr, &xpmattr);
    XpmFreeAttributes(&xpmattr);

    if (errorStatus != XpmSuccess)
#endif
        cpupixmap = XCreateBitmapFromData(XtDisplay(cpuframe),
                                          RootWindowOfScreen(
                                              XtScreen(cpuframe)),
                                          (char *)flexcpu_bits, flexcpu_width,
                                          flexcpu_height);

    if (cpupixmap != None)
    {
        XtVaSetValues(cpuframe, XtNiconPixmap, cpupixmap, nullptr);
    }
} // create_cpuview

void XtGui::create_bp_dialog(Widget parent)
{
    bpframe = XtVaCreatePopupShell("Breakpoints", transientShellWidgetClass,
                                   parent, nullptr);
    bpform = XtVaCreateManagedWidget("bpForm", formWidgetClass, bpframe, nullptr);
    bplabel[0] = XtVaCreateManagedWidget("bpLabel1", labelWidgetClass, bpform,
                                         nullptr);
    bplabel[1] = XtVaCreateManagedWidget("bpLabel2", labelWidgetClass, bpform,
                                         nullptr);
    bptext[0] = XtVaCreateManagedWidget("bpText1", asciiTextWidgetClass, bpform,
                                        XtNeditType, (XtArgVal)XawtextEdit,
                                        nullptr);
    bptext[1] = XtVaCreateManagedWidget("bpText2", asciiTextWidgetClass, bpform,
                                        XtNeditType, (XtArgVal)XawtextEdit,
                                        nullptr);
    bpokbutton = XtVaCreateManagedWidget("bpOkButton", commandWidgetClass,
                                         bpform, nullptr);
    bpcancelbutton = XtVaCreateManagedWidget("bpCancelButton",
                     commandWidgetClass, bpform, nullptr);
    bpclearbutton = XtVaCreateManagedWidget("bpClearButton", commandWidgetClass,
                                            bpform, nullptr);

    XtAddCallback(bpokbutton, XtNcallback, popdownBpCallback,
                  (XtPointer)this);
    XtAddCallback(bpcancelbutton, XtNcallback, popdownBpCallback,
                  (XtPointer)this);
    XtAddCallback(bpclearbutton, XtNcallback, clearBpCallback,
                  (XtPointer)this);

    XtInstallAllAccelerators(bpform, bptext[0]);

    XtRealizeWidget(bpframe);
    XSetWMProtocols(getDisplay(), XtWindow(bpframe), &wm_delete_window, 1);
} // create_bp_dialog

void XtGui::create_logfile_dialog(Widget parent)
{
    logframe = XtVaCreatePopupShell("Logging", transientShellWidgetClass,
                                    parent, nullptr);
    logform = XtVaCreateManagedWidget("logForm", formWidgetClass, logframe,
                                      nullptr);
    loglabel[0] = XtVaCreateManagedWidget("logLabel1", labelWidgetClass,
                                          logform, nullptr);
    loglabel[1] = XtVaCreateManagedWidget("logLabel2", labelWidgetClass,
                                          logform, nullptr);
    loglabel[2] = XtVaCreateManagedWidget("logLabel3", labelWidgetClass,
                                          logform, nullptr);
    loglabel[3] = XtVaCreateManagedWidget("logLabel4", labelWidgetClass,
                                          logform, nullptr);
    loglabel[4] = XtVaCreateManagedWidget("logLabel5", labelWidgetClass,
                                          logform, nullptr);
    logtext[0] = XtVaCreateManagedWidget("logText1", asciiTextWidgetClass,
                                         logform, XtNeditType,
                                         (XtArgVal)XawtextEdit, nullptr);
    logtext[1] = XtVaCreateManagedWidget("logText2", asciiTextWidgetClass,
                                         logform, XtNeditType,
                                         (XtArgVal)XawtextEdit, nullptr);
    logtext[2] = XtVaCreateManagedWidget("logText3", asciiTextWidgetClass,
                                         logform, XtNeditType,
                                         (XtArgVal)XawtextEdit, nullptr);
    logtext[3] = XtVaCreateManagedWidget("logText4", asciiTextWidgetClass,
                                         logform, XtNeditType,
                                         (XtArgVal)XawtextEdit, nullptr);
    logfilename = XtVaCreateManagedWidget("logFileName", asciiTextWidgetClass,
                                          logform, XtNeditType,
                                          (XtArgVal)XawtextEdit, nullptr);
    logokbutton = XtVaCreateManagedWidget("logOkButton", commandWidgetClass,
                                          logform, nullptr);
    logcancelbutton = XtVaCreateManagedWidget("logCancelButton",
                      commandWidgetClass, logform,
                      nullptr);
    logclearbutton = XtVaCreateManagedWidget("logClearButton",
                     commandWidgetClass, logform,
                     nullptr);

    XtAddCallback(logokbutton, XtNcallback, popdownLogCallback,
                  (XtPointer)this);
    XtAddCallback(logcancelbutton, XtNcallback, popdownLogCallback,
                  (XtPointer)this);
    XtAddCallback(logclearbutton, XtNcallback, clearLogCallback,
                  (XtPointer)this);

    XtInstallAllAccelerators(logform, logtext[0]);

    XtRealizeWidget(logframe);
    XSetWMProtocols(getDisplay(), XtWindow(logframe), &wm_delete_window, 1);
} // create_logfile_dialog

void XtGui::redraw_cpuview_impl(const Mc6809CpuStatus &stat)
{
    int i;
    CpuState current_state;

    i = stat.s & 7;
    text(5 + 3 * i, 10, "[");
    text(8 + 3 * i, 10, "]");
    XtVaSetValues(cputext, XtNlength, strlen(cpustring) + 1,
                  XtNstring, cpustring, nullptr);

    if (stat.state == CpuState::ResetRun)
    {
        current_state = CpuState::Reset;
    }
    else if (stat.state == CpuState::Invalid)
    {
        current_state = CpuState::Stop;
    }
    else
    {
        current_state = stat.state;
    }

    if (XawToggleGetCurrent(runbutton) !=
        (XtPointer)&radio_data[static_cast<Byte>(current_state)])
    {
        XtPointer ptr = &radio_data[static_cast<Byte>(stat.state)];
        XawToggleSetCurrent(runbutton, ptr);
    }

    if (stat.state == CpuState::Run)
    {
        XtVaSetValues(stepbutton, XtNsensitive, 0, nullptr);
        XtVaSetValues(nextbutton, XtNsensitive, 0, nullptr);
        XtVaSetValues(resetbutton, XtNsensitive, 0, nullptr);
    }
    else
    {
        XtVaSetValues(stepbutton, XtNsensitive, 1, nullptr);
        XtVaSetValues(nextbutton, XtNsensitive, 1, nullptr);
        XtVaSetValues(resetbutton, XtNsensitive, 1, nullptr);
    }
}

void XtGui::menuHandlerCallback(Widget button, XtPointer client_data,
                 XEvent *event, Boolean *flag)
{
    if (client_data != nullptr)
    {
        ((XtGui *)client_data)->menuHandler(button, event, flag);
    }
}

void XtGui::close_menu_mode()
{
    if (menu_popped_up != None)
    {
        if (XtIsRealized(menu_popped_up))
        {
            XtPopdown(menu_popped_up);
        }
        menu_popped_up = None;
    }
    is_menu_mode = false;
}

void XtGui::menuHandler(Widget pbutton, XEvent *event, Boolean * /*flag*/)
{
    size_t i;
    Widget pmenu = None;
    int x, y, width, height;
    char buffer[16];
    KeySym keysym;

    if (!pbutton || !XtIsRealized(pbutton))
    {
        return;
    }

    for (i = 0; i < sizeof(button)/sizeof(Widget); ++i)
    {
        if (button[i] == pbutton)
        {
            pmenu = menu[i];
        }
    }

    if (pmenu == None)
    {
        return;
    }

    XtVaGetValues(pbutton, XtNx, &x, XtNy, &y,
                  XtNwidth, &width, XtNheight, &height, nullptr);

    switch(event->type)
    {
    case KeyPress:
    case KeyRelease:
        XLookupString((XKeyEvent *)event, buffer, sizeof(buffer) - 1, &keysym,
                      nullptr);
        if (keysym == XK_Escape)
        {
            close_menu_mode();
        }
        break;

    case ButtonPress:
        if (menu_popped_up != None && (pmenu != menu_popped_up))
        {
            XtPopdown(menu_popped_up);
        }

        XtPopup(pmenu, XtGrabNone);
        menu_popped_up = pmenu;
        is_menu_mode = true;
        break;

    case EnterNotify:
        if (is_menu_mode)
        {
            if (menu_popped_up != None && (pmenu != menu_popped_up))
            {
                XtPopdown(menu_popped_up);
            }

            // Before popping up the menu it evtl. has to be moved
            // just beyond the menu button.
            XtMoveWidget(pmenu, event->xcrossing.x_root - event->xcrossing.x,
                         event->xcrossing.y_root - event->xcrossing.y + height);
            XtPopup(pmenu, XtGrabNone);
            menu_popped_up = pmenu;
            return;
        }
        break;

    case LeaveNotify:
        if (event->xcrossing.x < 0 || event->xcrossing.x >= width ||
         event->xcrossing.y < 0)
        {
            if (menu_popped_up != None && XtIsRealized(menu_popped_up))
            {
                XtPopdown(menu_popped_up);
                menu_popped_up = None;
            }
        }
        break;
    }
}
#endif // #ifdef HAVE_XTK

