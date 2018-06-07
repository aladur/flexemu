/*
    xtgui.h  userinterface for XToolkit


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



#ifndef __xtgui_h__
#define __xtgui_h__

#include <misc1.h>

#ifdef HAVE_XTK

#ifdef HAVE_X11_XPM_H
    #include <X11/xpm.h>
#endif

#include "flexemu.h"
#include "schedcpu.h"
#include "scpulog.h"
#include "xabsgui.h"

// include Athena Widget headers AFTER xabsgui.h

#include <X11/Xaw/Simple.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Toggle.h>
#include <X11/RectObj.h>
#include <X11/Xmu/Converters.h>
#include <X11/Shell.h>

enum
{
    PM_FLOPPY0 = 0,
    PM_FLOPPY1,
    PM_FLOPPY2,
    PM_IRQ0,
    PM_IRQ1,
    PM_IRQ2,
    PM_IRQ3,
    PM_IRQ4
};

#define NR_OF_HELPBUTTONS   (9)

extern String fallback_resources[];


class XtGui : public XAbstractGui
{

protected:
    static const char **pixmapname[8];
    static int radio_data[7];
    // Xtoolkit stuff
    Widget      e2toplevel, form, menubar, e2screen;
    Widget      button[3];
    Widget      menu[3];
    Widget      menu_popped_up;
    Widget      entry12;
    Widget      entry21, entry22, entry23, entry24, entry25;
    Widget      entry26, entry27, entry28;
    Widget      entry31, entry32;
    Widget      line21, line22;
    Widget      statusbuttons, floppybutton[4], interruptbutton;
    Widget      messageframe, messageform, messagetext;
    Widget      messagebuttons, messagebutton1, messagebutton2;
    Widget      aboutframe, aboutform, abouttext, aboutbutton;
    Widget      aboutbuttons;
    Widget      cpuframe, cpuform, cpubuttons, cputext;
    Widget      nextbutton, stepbutton, runbutton, stopbutton;
    Widget      bpbutton, logbutton, cpubutton, resetbutton;
    Widget      bpframe, bpform, bpokbutton, bpcancelbutton;
    Widget      bplabel[2], bptext[2], bpclearbutton;
    Widget      logframe, logform;
    Widget      logokbutton, logcancelbutton, logclearbutton;
    Widget      loglabel[5], logtext[4], logfilename;
    Pixmap      okpixmap, mainpixmap, cpupixmap;
    XtAppContext    context;
    XrmOptionDescList opts;

    // necessary for WM Protocol support
    Atom         wm_delete_window;

    // Pointer grabbing
    int      prev_x, current_x;
    int      prev_y, current_y;
    int      warp_x, warp_home_x, warp_dx;
    int      warp_y, warp_home_y, warp_dy;
    int      mouse_button_state;
#ifdef HAVE_XPM
    Widget       morebutton;
    Widget       authorframe, authorform, authorwidget, authorbutton;
    Pixmap       authorpixmap;
    Pixmap       pixmap[8];
    GC       authorgc;
#endif
    bool        is_menu_mode;
    bool        isSynchronized;

    virtual Display  *getDisplay();
    virtual Window   getWindow(tWindowType t = FLX_E2SCREEN);

protected:
    virtual Widget      create_main_view(int argc, char *const argv[],
                                         bool isSynchronized);
    virtual void        create_message_dialog(Widget parent);
    virtual void        create_about_dialog(Widget parent);
    virtual void        create_cpuview(Widget parent);
    virtual void        create_bp_dialog(Widget parent);
    virtual void        create_logfile_dialog(Widget parent);
    virtual void        manage_widget(Widget w);
    virtual void        initialize_after_create(Widget w, bool isInverse,
            const char *color);
    virtual void        initialize_after_open(Widget w, const char *title);
    virtual void        initialize(struct sGuiOptions *pOptions);
    virtual void        update_disk_status(int floppyIndex,
                                           tDiskStatus status);
    virtual void        update_interrupt_status(tIrqType t,
            bool status);
#ifdef HAVE_XPM
    virtual void        create_pixmaps(Widget parent, Pixel bg_color);
    virtual void        create_author_dialog(Widget parent);
#endif
    virtual void        add_menu_handler(Widget button, Widget menu);
    virtual void        close_menu_mode();

    // Internal registers

protected:

    bool        cpu_popped_up;
    bool        frequency_control_on;
    bool        is_use_undocumented;
    struct s_cpu_logfile lfs;

    // Initialisation functions

protected:

    virtual void    initialize_e2window(struct sGuiOptions *pOptions);

    // public interface
public:

    virtual void    center_dialog(Widget w);
    virtual void    popup_about();
    virtual void    popup_cpu();
    virtual void    popup_bp();
    virtual void    popup_log();
    virtual void    popdown_message(Widget w);
    virtual void    popdown_about();
    virtual void    popdown_cpu();
    virtual void    popdown_bp(Widget w);
    virtual void    popdown_log(Widget w);
    virtual void    popup_disk_info(Widget w);
    virtual void    popup_interrupt_info(Widget w);
    virtual void    clear_bp();
    virtual void    clear_log();
    virtual void    toggle_cpu();
    virtual void    toggle_frequency();
    virtual void    toggle_undocumented();
    virtual void    main_loop();
    virtual void    mouse_update();
    virtual void    mouse_warp(int dx, int dy);
#ifdef HAVE_XPM
    virtual void    a_expose(Widget w, XEvent *pevent);
    virtual void    popup_author();
    virtual void    popdown_author();
#endif

    // callbacks:
private:
    static void     updateNafsCallback(Widget w, XtPointer client_data,
                                       XtPointer call_data);
    static void     setCpuExitCallback(Widget w, XtPointer client_data,
                                       XtPointer call_data);
    static void     setCpuRunCallback(Widget w, XtPointer client_data,
                                      XtPointer call_data);
    static void     setCpuStopCallback(Widget w, XtPointer client_data,
                                       XtPointer call_data);
    static void     toggleCpuRunCallback(Widget w, XtPointer client_data,
                                         XtPointer call_data);
    static void     toggleCpuStopCallback(Widget w, XtPointer client_data,
                                          XtPointer call_data);
    static void     toggleCpuStepCallback(Widget w, XtPointer client_data,
                                          XtPointer call_data);
    static void     toggleCpuNextCallback(Widget w, XtPointer client_data,
                                          XtPointer call_data);
    static void     toggleCpuResetCallback(Widget w, XtPointer client_data,
                                           XtPointer call_data);
    static void     setCpuResetRunCallback(Widget w, XtPointer client_data,
                                           XtPointer call_data);
    static void     toggleCpuCallback(Widget w, XtPointer client_data,
                                      XtPointer call_data);
    static void     toggleFrequencyCallback(Widget w, XtPointer client_data,
                                            XtPointer call_data);
    static void     toggleUndocumentedCallback(Widget w, XtPointer client_data,
            XtPointer call_data);
    static void     popdownCpuCallback(Widget w, XtPointer client_data,
                                       XtPointer call_data);
    static void     popupHelpCallback(Widget w, XtPointer client_data,
                                      XtPointer call_data);
    static void     popupAboutCallback(Widget w, XtPointer client_data,
                                       XtPointer call_data);
    static void     popdownAboutCallback(Widget w, XtPointer client_data,
                                         XtPointer call_data);
    static void     popupBpCallback(Widget w, XtPointer client_data,
                                    XtPointer call_data);
    static void     popupLogfileCallback(Widget w, XtPointer client_data,
                                         XtPointer call_data);
    static void     clearBpCallback(Widget w, XtPointer client_data,
                                    XtPointer call_data);
    static void     clearLogCallback(Widget w, XtPointer client_data,
                                     XtPointer call_data);
    static void     popdownBpCallback(Widget w, XtPointer client_data,
                                      XtPointer call_data);
    static void     popdownLogCallback(Widget w, XtPointer client_data,
                                       XtPointer call_data);
    static void     popupAuthorCallback(Widget w, XtPointer client_data,
                                        XtPointer call_data);
    static void     popdownAuthorCallback(Widget w, XtPointer client_data,
                                          XtPointer call_data);
    static void     popdownMessageCallback(Widget w, XtPointer client_data,
                                           XtPointer call_data);
    static void     popupFloppyCallback(Widget w, XtPointer client_data,
                                        XtPointer call_data);
    static void     popupInterruptCallback(Widget w, XtPointer client_data,
                                           XtPointer call_data);
    static void     menuHandlerCallback(Widget button, XtPointer client_data,
                                XEvent *event, Boolean *flag);
    void    updateNafs();
    static void     timerCallbackProc(XtPointer client_data, XtIntervalId *pId);

    // accelerators:
public:
    virtual void    c_expose(XEvent *pevent);
    virtual void    c_keyPress(XEvent *pevent);
    virtual void    c_buttonPress(XEvent *pevent);
    virtual void    c_buttonRelease(XEvent *pevent);
    virtual void    c_motion(XEvent *pevent);
    virtual void    c_enter(XEvent *pevent);
    virtual void    c_leave(XEvent *pevent);
    virtual void    c_focusIn(XEvent *pevent);
    virtual void    c_focusOut(XEvent *pevent);
    virtual void    c_wm_protocols(XEvent *pevent);
    virtual void    c_process_resize(XEvent *pevent);
    virtual void    c_close_menu_mode(XEvent *pevent);
    virtual void    c_highlight_child(Widget w, XEvent *pevent, String *params);

public:
    virtual void    redraw_cpuview_impl(const Mc6809CpuStatus &sstat);
    virtual void    popup_message(const char *pmessage,
                                  const char *ptitle = NULL,
                                  int width = 270, int height = 120);
    virtual void    popup_confirmation(const char *pmessage,
                                       const char *ptitle = NULL,
                                       int width = 270, int height = 120);
    virtual int     gui_type();
    void            timerCallback(XtIntervalId pId);
    virtual void    menuHandler(Widget button, XEvent *event, Boolean *flag);

    // Public constructor and destructor
public:
    XtGui(
        Mc6809 *x_cpu,
        Memory *x_memory,
        Scheduler *x_sched,
        Inout *x_io,
        E2video *x_video,
        struct sGuiOptions *options);
    virtual ~XtGui();
};


#endif // ifdef HAVE_XTK
#endif // __xtgui_h__

