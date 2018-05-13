/*
    mc6821.cpp


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


#include "misc1.h"
#include <stdio.h>

#include "mc6821.h"

Mc6821::Mc6821()
{
}

Mc6821::~Mc6821()
{
}

void Mc6821::resetIo()
{
    cra = 0;    // control register A
    ora = 0;    // output register A
    ddra = 0;   // data direction register A
    crb = 0;    // control register B
    orb = 0;    // output register B
    ddrb = 0;   // data direction register B

    cls = 0;    // control lines CA1, CA2, CB1, CB2

}

Byte Mc6821::readIo(Word offset)
{
    switch (offset & 0x03)
    {
        case 0:
            if (cra & 0x04)         // check data direction bit
            {
                ora = readInputA(); // get data from HW-input
                cra &= 0xbf;    // clear IRQA2 flag
                cra &= 0x7f;    // clear IRQA1 flag

                if ((cra & 0x38) == 0x20)
                {
                    cls &= ~CA2;
                }

                return ora; // read output register A
            }
            else
            {
                return ddra;
            }

        case 1:
            requestInputA();        // request for input
            return cra;

        case 2:
            if (crb & 0x04)         // check data direction bit
            {
                orb = readInputB(); // get data from HW-input
                crb &= 0xbf;    // clear IRQB2 flag
                crb &= 0x7f;    // clear IRQB1 flag
                return orb; // read output register B
            }
            else
            {
                return ddrb;
            }

        case 3:
            requestInputB();        // request for input
            return cra;
    }

    return 0;
}

// read data from port-Pins (should be overwritten by subclass)
// if non strobed data input should be read

Byte Mc6821::readInputA(void)
{
    return ora;
}

Byte Mc6821::readInputB(void)
{
    return orb;
}

// request for data from port-Pins (should be implemented by subclass)
// if strobed data input should be read

void Mc6821::requestInputA(void)
{
}

void Mc6821::requestInputB(void)
{
}


void Mc6821::writeIo(Word offset, Byte val)
{
    switch (offset & 0x03)
    {
        case 0:
            if (BTST2(cra))     // check data direction bit
            {
                ora = val;
                writeOutputA(ora); // write output to port-Pins
            }
            else
            {
                ddra = val;
            }

            break;

        case 1:
            cra = val;

            if ((cra & 0x30) == 0x30)
            {
                if (BTST3(cra))
                {
                    cls |= CA2;
                }
                else
                {
                    cls &= ~CA2;
                }
            }

            break;

        case 2:
            if (BTST2(crb))     // check data direction bit
            {
                orb = val;
                writeOutputB(orb); // write output to port-Pins

                if ((crb & 0x38) == 0x20)
                {
                    cls &= CB2;
                }
            }
            else
            {
                ddrb = val;
            }

            break;

        case 3:
            crb = val;

            if ((crb & 0x30) == 0x30)
            {
                if (BTST3(crb))
                {
                    cls |= CB2;
                }
                else
                {
                    cls &= ~CB2;
                }
            }

            break;
    }
}

// write output to port-Pins (should be overwritten by subclass)

void Mc6821::writeOutputA(Byte)
{
}

void Mc6821::set_irq_A(void)
{
}

void Mc6821::set_irq_B(void)
{
}


void Mc6821::writeOutputB(Byte)
{
}

// generate an active transition on CA1, CA2, CB1 or CB2

void Mc6821::activeTransition(Byte control_line)
{
    switch (control_line)
    {
        case CA1:
            BSET7(cra);     // set IRQA1 flag

            if (BTST0(cra))
            {
                set_irq_A();    // send an interrupt to CPU
            }

            if ((cra & 0x38) == 0x20)
            {
                cls |= CA2;
            }

            break;

        case CA2:
            if (BTST5(cra))
            {
                BSET6(cra); // set IRQA2 flag

                if (BTST3(cra))
                {
                    // send an interrupt to CPU
                }
            }

            break;

        case CB1:
            BSET7(crb);     // set IRQB1 flag

            if (BTST0(crb))
            {
                set_irq_B();    // send an interrupt to CPU
            }

            if ((crb & 0x38) == 0x20)
            {
                cls |= CB2;
            }

            break;

        case CB2:
            if (BTST5(crb))
            {
                BSET6(crb); // set IRQB2 flag

                if (BTST3(crb))
                {
                    // send an interrupt to CPU
                }
            }

            break;
    }
}


// test contol lines CB1 or CB2
// contents of control lines only valid, if used as outputs

Byte Mc6821::testControlLine(Byte control_line)
{
    return (cls & control_line);
}
