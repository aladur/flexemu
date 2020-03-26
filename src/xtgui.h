/*
    xtgui.h  userinterface for XToolkit


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



#ifndef XTGUI_INCLUDED
#define XTGUI_INCLUDED

#include <misc1.h>

#ifdef HAVE_XTK

#ifdef HAVE_X11_XPM_H
    #include <X11/xpm.h>
#endif

#include "flexemu.h"
#include "schedcpu.h"
#include "scpulog.h"
#include "xabsgui.h"
#include <memory>

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

class Pia1;

class XtGui : public XAbstractGui
{

protected:
    Pia1 &pia1;

    static const char **pixmapname[8];
    static int radio_data[];
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
    std::unique_ptr<Byte[]> image_data;

    Display *getDisplay() override;
    Window getWindow(tWindowType windowType = FLX_E2SCREEN) override;

protected:
    void initialize(struct sGuiOptions &options) override;

    Widget create_main_view(int argc, char *const argv[], bool isSynchronized);
    void create_message_dialog(Widget parent) override;
    void create_about_dialog(Widget parent);
    void create_cpuview(Widget parent);
    void create_bp_dialog(Widget parent);
    void create_logfile_dialog(Widget parent);
    void manage_widget(Widget w);
    void initialize_e2window(struct sGuiOptions &options);
    void initialize_after_create(Widget w, bool isInverse);
    void initialize_after_open(Widget w, const char *title);
    void update_disk_status(int floppyIndex, DiskStatus status);
    void update_interrupt_status(tIrqType irqType, bool status);
#ifdef HAVE_XPM
    void create_pixmaps(Widget parent, Pixel bg_color);
    void create_author_dialog(Widget parent);
#endif
    void add_menu_handler(Widget button, Widget menu);
    void close_menu_mode();
    void update_frequency_check();

    // Internal registers

protected:

    bool        cpu_popped_up;
    bool        frequency_control_on;
    bool        is_use_undocumented;
    s_cpu_logfile lfs;

    // public interface
public:

    void main_loop() override;

    void center_dialog(Widget w);
    void popup_about();
    void popup_cpu();
    void popup_bp();
    void popup_log();
    void popdown_message(Widget w);
    void popdown_about();
    void popdown_cpu();
    void popdown_bp(Widget w);
    void popdown_log(Widget w);
    void popup_disk_info(Widget w);
    void popup_interrupt_info(Widget w);
    void clear_bp();
    void clear_log();
    void toggle_cpu();
    void toggle_frequency();
    void toggle_undocumented();
    void mouse_update();
    void mouse_warp(int dx, int dy);
#ifdef HAVE_XPM
    void a_expose(Widget w, XEvent *pevent);
    void popup_author();
    void popdown_author();
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
    void c_focusIn(XEvent *pevent) override;
    void c_focusOut(XEvent *pevent) override;

    void c_expose(XEvent *pevent);
    void c_keyPress(XEvent *pevent);
    void c_buttonPress(XEvent *pevent);
    void c_buttonRelease(XEvent *pevent);
    void c_motion(XEvent *pevent);
    void c_enter(XEvent *pevent);
    void c_leave(XEvent *pevent);
    void c_wm_protocols(XEvent *pevent);
    void c_process_resize(XEvent *pevent);
    void c_close_menu_mode(XEvent *pevent);
    void c_highlight_child(Widget w, XEvent *pevent, String *params);

public:
    GuiType gui_type() override;

    void redraw_cpuview_impl(const Mc6809CpuStatus &sstat) override;
    void popup_message(const char *pmessage,
                                  const char *ptitle = nullptr,
                                  int width = 270, int height = 120);
    void popup_confirmation(const char *pmessage,
                                       const char *ptitle = nullptr,
                                       int width = 270, int height = 120);
    void            timerCallback(XtIntervalId pId);
    void menuHandler(Widget button, XEvent *event, Boolean *flag);

    // Public constructor and destructor
public:
    XtGui(
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
        struct sGuiOptions &options);
    virtual ~XtGui();
};

#endif // ifdef HAVE_XTK
#endif // XTGUI_INCLUDED

