/*
    csetfreq.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2026  W. Schwotzer

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

#ifndef CSETFREQ_INCLUDED
#define CSETFREQ_INCLUDED

#include "bcommand.h"

class Scheduler;

class CSetFrequency : public BCommand
{

public:
    CSetFrequency(Scheduler &p_sched, float p_frequency);
    ~CSetFrequency() override = default;
    CSetFrequency(const CSetFrequency &src) = delete;
    CSetFrequency(CSetFrequency &&src) = delete;
    CSetFrequency &operator=(const CSetFrequency &src) = delete;
    CSetFrequency &operator=(CSetFrequency &&src) = delete;
    void Execute() override;

protected:
    Scheduler &scheduler;
    float frequency;
};

#endif
