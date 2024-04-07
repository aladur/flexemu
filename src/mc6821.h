/*
    mc6821.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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



#ifndef MC6821_INCLUDED
#define MC6821_INCLUDED

#include <cstdint>
#include <type_traits>
#include "iodevice.h"


class Mc6821 : public IoDevice
{
public:
    enum class ControlLine : uint8_t
    {
        NONE = 0,
        CA1 = 1,
        CA2 = 2,
        CB1 = 4,
        CB2 = 8,
    };

protected:

    // Internal registers:
    //
    // cra, crb control register A, B
    // ddra, ddrb data direction register A, B
    // ora, orb output register A, B

    Byte cra{0};
    Byte ora{0};
    Byte ddra{0};
    Byte crb{0};
    Byte orb{0};
    Byte ddrb{0};
    Mc6821::ControlLine cls{ControlLine::NONE};

public:

    // IoDevice interface
    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte value) override;
    void resetIo() override;
    const char *getName() override
    {
        return "mc6821";
    }
    Word sizeOfIo() override
    {
        return 4;
    }


public:

    // generate an active transition on CA1, CA2, CB1 or CB2
    void activeTransition(Mc6821::ControlLine control_line);

    // test contol line
    bool testControlLine(Mc6821::ControlLine control_line);


protected:

    // read non strobed data
    virtual Byte readInputA();
    virtual Byte readInputB();
    virtual void set_irq_A();
    virtual void set_irq_B();

    // read strobed data
    virtual void requestInputA();
    virtual void requestInputB();

    // write data to port-pins
    virtual void writeOutputA(Byte val);
    virtual void writeOutputB(Byte val);

public:

    Mc6821() = default;
    ~Mc6821() override = default;
};

inline Mc6821::ControlLine operator| (Mc6821::ControlLine lhs,
                                      Mc6821::ControlLine rhs)
{
    using T = std::underlying_type<Mc6821::ControlLine>::type;

    return static_cast<Mc6821::ControlLine>(static_cast<T>(lhs) |
                                            static_cast<T>(rhs));
}

inline Mc6821::ControlLine operator& (Mc6821::ControlLine lhs,
                                      Mc6821::ControlLine rhs)
{
    using T = std::underlying_type<Mc6821::ControlLine>::type;

    return static_cast<Mc6821::ControlLine>(static_cast<T>(lhs) &
                                            static_cast<T>(rhs));
}

inline Mc6821::ControlLine operator|= (Mc6821::ControlLine &lhs,
                                       Mc6821::ControlLine rhs)
{
    return lhs = lhs | rhs;
}

inline Mc6821::ControlLine operator&= (Mc6821::ControlLine &lhs,
                                       Mc6821::ControlLine rhs)
{
    return lhs = lhs & rhs;
}

inline Mc6821::ControlLine operator~ (const Mc6821::ControlLine lhs)
{
    using T = std::underlying_type<Mc6821::ControlLine>::type;

    return static_cast<Mc6821::ControlLine>(~static_cast<T>(lhs));
}

#endif // MC6821_INCLUDED
