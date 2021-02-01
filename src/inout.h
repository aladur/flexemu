/*
    inout.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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



#ifndef INOUT_INCLUDED
#define INOUT_INCLUDED

#include "misc1.h"
#include "flexemu.h"
#include "bobserv.h"
#include <string>
#include <memory>


class Memory;
class Mc146818;
class AbstractGui;

class Inout : public BObserver
{
private:
    Memory &memory;
    const struct sOptions &options;
    Mc146818 *rtc; // RTC is an optional device
    AbstractGui *gui;
    int local_serpar_address;

public:
    void    update_1_second();
    void    set_rtc(Mc146818 *x_rtc);
    void    set_gui(AbstractGui *x_gui);

    // Communication with GUI
public:
    bool    output_to_terminal();
    bool    output_to_graphic();
    bool    is_gui_present();

    // local interface
public:
    bool is_serpar_address_valid() const;
    Word serpar_address() const;
    void serpar_address(int value);

    // BObserver interface
public:
    void UpdateFrom(NotifyId id, void *param = nullptr) override;

public:
    Inout(const struct sOptions &x_options, Memory &x_memory);
    ~Inout();
};

#endif // INOUT_INCLUDED

