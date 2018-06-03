/* engine.cc

   flexemu, an MC6809 emulator running FLEX
   Copyright (C) 1997-2018  W. Schwotzer

   Copyright 1994,1995 L.C. Benschop, Eidnhoven The Netherlands.

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

#include <stdio.h>
#include "engine.h"


static unsigned char haspostbyte[] =
{
    /*0*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*1*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*2*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*3*/      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*4*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*5*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*6*/      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /*7*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*8*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*9*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*A*/      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /*B*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*C*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*D*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*E*/      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /*F*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

iflag = 0;
flaginstr:  /* $10 and $11 instructions return here */
        ireg = GETBYTE_PI(ipcreg);

        if (haspostbyte[ireg])
        {
            Byte postbyte = GETBYTE_PI(ipcreg);

            switch (postbyte)
            {
                case 0x00:
                    eaddr = ixreg;
                    break;

                case 0x01:
                    eaddr = ixreg + 1;
                    break;

                case 0x02:
                    eaddr = ixreg + 2;
                    break;

                case 0x03:
                    eaddr = ixreg + 3;
                    break;

                case 0x04:
                    eaddr = ixreg + 4;
                    break;

                case 0x05:
                    eaddr = ixreg + 5;
                    break;

                case 0x06:
                    eaddr = ixreg + 6;
                    break;

                case 0x07:
                    eaddr = ixreg + 7;
                    break;

                case 0x08:
                    eaddr = ixreg + 8;
                    break;

                case 0x09:
                    eaddr = ixreg + 9;
                    break;

                case 0x0A:
                    eaddr = ixreg + 10;
                    break;

                case 0x0B:
                    eaddr = ixreg + 11;
                    break;

                case 0x0C:
                    eaddr = ixreg + 12;
                    break;

                case 0x0D:
                    eaddr = ixreg + 13;
                    break;

                case 0x0E:
                    eaddr = ixreg + 14;
                    break;

                case 0x0F:
                    eaddr = ixreg + 15;
                    break;

                case 0x10:
                    eaddr = ixreg - 16;
                    break;

                case 0x11:
                    eaddr = ixreg - 15;
                    break;

                case 0x12:
                    eaddr = ixreg - 14;
                    break;

                case 0x13:
                    eaddr = ixreg - 13;
                    break;

                case 0x14:
                    eaddr = ixreg - 12;
                    break;

                case 0x15:
                    eaddr = ixreg - 11;
                    break;

                case 0x16:
                    eaddr = ixreg - 10;
                    break;

                case 0x17:
                    eaddr = ixreg - 9;
                    break;

                case 0x18:
                    eaddr = ixreg - 8;
                    break;

                case 0x19:
                    eaddr = ixreg - 7;
                    break;

                case 0x1A:
                    eaddr = ixreg - 6;
                    break;

                case 0x1B:
                    eaddr = ixreg - 5;
                    break;

                case 0x1C:
                    eaddr = ixreg - 4;
                    break;

                case 0x1D:
                    eaddr = ixreg - 3;
                    break;

                case 0x1E:
                    eaddr = ixreg - 2;
                    break;

                case 0x1F:
                    eaddr = ixreg - 1;
                    break;

                case 0x20:
                    eaddr = iyreg;
                    break;

                case 0x21:
                    eaddr = iyreg + 1;
                    break;

                case 0x22:
                    eaddr = iyreg + 2;
                    break;

                case 0x23:
                    eaddr = iyreg + 3;
                    break;

                case 0x24:
                    eaddr = iyreg + 4;
                    break;

                case 0x25:
                    eaddr = iyreg + 5;
                    break;

                case 0x26:
                    eaddr = iyreg + 6;
                    break;

                case 0x27:
                    eaddr = iyreg + 7;
                    break;

                case 0x28:
                    eaddr = iyreg + 8;
                    break;

                case 0x29:
                    eaddr = iyreg + 9;
                    break;

                case 0x2A:
                    eaddr = iyreg + 10;
                    break;

                case 0x2B:
                    eaddr = iyreg + 11;
                    break;

                case 0x2C:
                    eaddr = iyreg + 12;
                    break;

                case 0x2D:
                    eaddr = iyreg + 13;
                    break;

                case 0x2E:
                    eaddr = iyreg + 14;
                    break;

                case 0x2F:
                    eaddr = iyreg + 15;
                    break;

                case 0x30:
                    eaddr = iyreg - 16;
                    break;

                case 0x31:
                    eaddr = iyreg - 15;
                    break;

                case 0x32:
                    eaddr = iyreg - 14;
                    break;

                case 0x33:
                    eaddr = iyreg - 13;
                    break;

                case 0x34:
                    eaddr = iyreg - 12;
                    break;

                case 0x35:
                    eaddr = iyreg - 11;
                    break;

                case 0x36:
                    eaddr = iyreg - 10;
                    break;

                case 0x37:
                    eaddr = iyreg - 9;
                    break;

                case 0x38:
                    eaddr = iyreg - 8;
                    break;

                case 0x39:
                    eaddr = iyreg - 7;
                    break;

                case 0x3A:
                    eaddr = iyreg - 6;
                    break;

                case 0x3B:
                    eaddr = iyreg - 5;
                    break;

                case 0x3C:
                    eaddr = iyreg - 4;
                    break;

                case 0x3D:
                    eaddr = iyreg - 3;
                    break;

                case 0x3E:
                    eaddr = iyreg - 2;
                    break;

                case 0x3F:
                    eaddr = iyreg - 1;
                    break;

                case 0x40:
                    eaddr = iureg;
                    break;

                case 0x41:
                    eaddr = iureg + 1;
                    break;

                case 0x42:
                    eaddr = iureg + 2;
                    break;

                case 0x43:
                    eaddr = iureg + 3;
                    break;

                case 0x44:
                    eaddr = iureg + 4;
                    break;

                case 0x45:
                    eaddr = iureg + 5;
                    break;

                case 0x46:
                    eaddr = iureg + 6;
                    break;

                case 0x47:
                    eaddr = iureg + 7;
                    break;

                case 0x48:
                    eaddr = iureg + 8;
                    break;

                case 0x49:
                    eaddr = iureg + 9;
                    break;

                case 0x4A:
                    eaddr = iureg + 10;
                    break;

                case 0x4B:
                    eaddr = iureg + 11;
                    break;

                case 0x4C:
                    eaddr = iureg + 12;
                    break;

                case 0x4D:
                    eaddr = iureg + 13;
                    break;

                case 0x4E:
                    eaddr = iureg + 14;
                    break;

                case 0x4F:
                    eaddr = iureg + 15;
                    break;

                case 0x50:
                    eaddr = iureg - 16;
                    break;

                case 0x51:
                    eaddr = iureg - 15;
                    break;

                case 0x52:
                    eaddr = iureg - 14;
                    break;

                case 0x53:
                    eaddr = iureg - 13;
                    break;

                case 0x54:
                    eaddr = iureg - 12;
                    break;

                case 0x55:
                    eaddr = iureg - 11;
                    break;

                case 0x56:
                    eaddr = iureg - 10;
                    break;

                case 0x57:
                    eaddr = iureg - 9;
                    break;

                case 0x58:
                    eaddr = iureg - 8;
                    break;

                case 0x59:
                    eaddr = iureg - 7;
                    break;

                case 0x5A:
                    eaddr = iureg - 6;
                    break;

                case 0x5B:
                    eaddr = iureg - 5;
                    break;

                case 0x5C:
                    eaddr = iureg - 4;
                    break;

                case 0x5D:
                    eaddr = iureg - 3;
                    break;

                case 0x5E:
                    eaddr = iureg - 2;
                    break;

                case 0x5F:
                    eaddr = iureg - 1;
                    break;

                case 0x60:
                    eaddr = isreg;
                    break;

                case 0x61:
                    eaddr = isreg + 1;
                    break;

                case 0x62:
                    eaddr = isreg + 2;
                    break;

                case 0x63:
                    eaddr = isreg + 3;
                    break;

                case 0x64:
                    eaddr = isreg + 4;
                    break;

                case 0x65:
                    eaddr = isreg + 5;
                    break;

                case 0x66:
                    eaddr = isreg + 6;
                    break;

                case 0x67:
                    eaddr = isreg + 7;
                    break;

                case 0x68:
                    eaddr = isreg + 8;
                    break;

                case 0x69:
                    eaddr = isreg + 9;
                    break;

                case 0x6A:
                    eaddr = isreg + 10;
                    break;

                case 0x6B:
                    eaddr = isreg + 11;
                    break;

                case 0x6C:
                    eaddr = isreg + 12;
                    break;

                case 0x6D:
                    eaddr = isreg + 13;
                    break;

                case 0x6E:
                    eaddr = isreg + 14;
                    break;

                case 0x6F:
                    eaddr = isreg + 15;
                    break;

                case 0x70:
                    eaddr = isreg - 16;
                    break;

                case 0x71:
                    eaddr = isreg - 15;
                    break;

                case 0x72:
                    eaddr = isreg - 14;
                    break;

                case 0x73:
                    eaddr = isreg - 13;
                    break;

                case 0x74:
                    eaddr = isreg - 12;
                    break;

                case 0x75:
                    eaddr = isreg - 11;
                    break;

                case 0x76:
                    eaddr = isreg - 10;
                    break;

                case 0x77:
                    eaddr = isreg - 9;
                    break;

                case 0x78:
                    eaddr = isreg - 8;
                    break;

                case 0x79:
                    eaddr = isreg - 7;
                    break;

                case 0x7A:
                    eaddr = isreg - 6;
                    break;

                case 0x7B:
                    eaddr = isreg - 5;
                    break;

                case 0x7C:
                    eaddr = isreg - 4;
                    break;

                case 0x7D:
                    eaddr = isreg - 3;
                    break;

                case 0x7E:
                    eaddr = isreg - 2;
                    break;

                case 0x7F:
                    eaddr = isreg - 1;
                    break;

                case 0x80:
                    eaddr = ixreg;
                    ixreg++;
                    break;

                case 0x81:
                    eaddr = ixreg;
                    ixreg += 2;
                    break;

                case 0x82:
                    ixreg--;
                    eaddr = ixreg;
                    break;

                case 0x83:
                    ixreg -= 2;
                    eaddr = ixreg;
                    break;

                case 0x84:
                    eaddr = ixreg;
                    break;

                case 0x85:
                    eaddr = ixreg + SIGNED(ibreg);
                    break;

                case 0x86:
                    eaddr = ixreg + SIGNED(iareg);
                    break;

                case 0x87:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILELGAL*/

                case 0x88:
                    IMMBYTE(eaddr);
                    eaddr = ixreg + SIGNED(eaddr);
                    break;

                case 0x89:
                    IMMWORD(eaddr);
                    eaddr += ixreg;
                    break;

                case 0x8A:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0x8B:
                    eaddr = ixreg + GETDREG;
                    break;

                case 0x8C:
                    IMMBYTE(eaddr);
                    eaddr = ipcreg + SIGNED(eaddr);
                    break;

                case 0x8D:
                    IMMWORD(eaddr);
                    eaddr += ipcreg;
                    break;

                case 0x8E:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0x8F:
                    IMMWORD(eaddr);
                    break;

                case 0x90:
                    eaddr = ixreg;
                    ixreg++;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x91:
                    eaddr = ixreg;
                    ixreg += 2;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x92:
                    ixreg--;
                    eaddr = ixreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x93:
                    ixreg -= 2;
                    eaddr = ixreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x94:
                    eaddr = ixreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x95:
                    eaddr = ixreg + SIGNED(ibreg);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x96:
                    eaddr = ixreg + SIGNED(iareg);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x97:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILELGAL*/

                case 0x98:
                    IMMBYTE(eaddr);
                    eaddr = ixreg + SIGNED(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x99:
                    IMMWORD(eaddr);
                    eaddr += ixreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x9A:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0x9B:
                    eaddr = ixreg + GETDREG;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x9C:
                    IMMBYTE(eaddr);
                    eaddr = ipcreg + SIGNED(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x9D:
                    IMMWORD(eaddr);
                    eaddr += ipcreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0x9E:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0x9F:
                    IMMWORD(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xA0:
                    eaddr = iyreg;
                    iyreg++;
                    break;

                case 0xA1:
                    eaddr = iyreg;
                    iyreg += 2;
                    break;

                case 0xA2:
                    iyreg--;
                    eaddr = iyreg;
                    break;

                case 0xA3:
                    iyreg -= 2;
                    eaddr = iyreg;
                    break;

                case 0xA4:
                    eaddr = iyreg;
                    break;

                case 0xA5:
                    eaddr = iyreg + SIGNED(ibreg);
                    break;

                case 0xA6:
                    eaddr = iyreg + SIGNED(iareg);
                    break;

                case 0xA7:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILELGAL*/

                case 0xA8:
                    IMMBYTE(eaddr);
                    eaddr = iyreg + SIGNED(eaddr);
                    break;

                case 0xA9:
                    IMMWORD(eaddr);
                    eaddr += iyreg;
                    break;

                case 0xAA:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xAB:
                    eaddr = iyreg + GETDREG;
                    break;

                case 0xAC:
                    IMMBYTE(eaddr);
                    eaddr = ipcreg + SIGNED(eaddr);
                    break;

                case 0xAD:
                    IMMWORD(eaddr);
                    eaddr += ipcreg;
                    break;

                case 0xAE:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xAF:
                    IMMWORD(eaddr);
                    break;

                case 0xB0:
                    eaddr = iyreg;
                    iyreg++;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xB1:
                    eaddr = iyreg;
                    iyreg += 2;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xB2:
                    iyreg--;
                    eaddr = iyreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xB3:
                    iyreg -= 2;
                    eaddr = iyreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xB4:
                    eaddr = iyreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xB5:
                    eaddr = iyreg + SIGNED(ibreg);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xB6:
                    eaddr = iyreg + SIGNED(iareg);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xB7:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILELGAL*/

                case 0xB8:
                    IMMBYTE(eaddr);
                    eaddr = iyreg + SIGNED(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xB9:
                    IMMWORD(eaddr);
                    eaddr += iyreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xBA:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xBB:
                    eaddr = iyreg + GETDREG;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xBC:
                    IMMBYTE(eaddr);
                    eaddr = ipcreg + SIGNED(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xBD:
                    IMMWORD(eaddr);
                    eaddr += ipcreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xBE:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xBF:
                    IMMWORD(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xC0:
                    eaddr = iureg;
                    iureg++;
                    break;

                case 0xC1:
                    eaddr = iureg;
                    iureg += 2;
                    break;

                case 0xC2:
                    iureg--;
                    eaddr = iureg;
                    break;

                case 0xC3:
                    iureg -= 2;
                    eaddr = iureg;
                    break;

                case 0xC4:
                    eaddr = iureg;
                    break;

                case 0xC5:
                    eaddr = iureg + SIGNED(ibreg);
                    break;

                case 0xC6:
                    eaddr = iureg + SIGNED(iareg);
                    break;

                case 0xC7:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILELGAL*/

                case 0xC8:
                    IMMBYTE(eaddr);
                    eaddr = iureg + SIGNED(eaddr);
                    break;

                case 0xC9:
                    IMMWORD(eaddr);
                    eaddr += iureg;
                    break;

                case 0xCA:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xCB:
                    eaddr = iureg + GETDREG;
                    break;

                case 0xCC:
                    IMMBYTE(eaddr);
                    eaddr = ipcreg + SIGNED(eaddr);
                    break;

                case 0xCD:
                    IMMWORD(eaddr);
                    eaddr += ipcreg;
                    break;

                case 0xCE:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xCF:
                    IMMWORD(eaddr);
                    break;

                case 0xD0:
                    eaddr = iureg;
                    iureg++;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xD1:
                    eaddr = iureg;
                    iureg += 2;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xD2:
                    iureg--;
                    eaddr = iureg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xD3:
                    iureg -= 2;
                    eaddr = iureg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xD4:
                    eaddr = iureg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xD5:
                    eaddr = iureg + SIGNED(ibreg);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xD6:
                    eaddr = iureg + SIGNED(iareg);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xD7:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILELGAL*/

                case 0xD8:
                    IMMBYTE(eaddr);
                    eaddr = iureg + SIGNED(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xD9:
                    IMMWORD(eaddr);
                    eaddr += iureg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xDA:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xDB:
                    eaddr = iureg + GETDREG;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xDC:
                    IMMBYTE(eaddr);
                    eaddr = ipcreg + SIGNED(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xDD:
                    IMMWORD(eaddr);
                    eaddr += ipcreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xDE:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xDF:
                    IMMWORD(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xE0:
                    eaddr = isreg;
                    isreg++;
                    break;

                case 0xE1:
                    eaddr = isreg;
                    isreg += 2;
                    break;

                case 0xE2:
                    isreg--;
                    eaddr = isreg;
                    break;

                case 0xE3:
                    isreg -= 2;
                    eaddr = isreg;
                    break;

                case 0xE4:
                    eaddr = isreg;
                    break;

                case 0xE5:
                    eaddr = isreg + SIGNED(ibreg);
                    break;

                case 0xE6:
                    eaddr = isreg + SIGNED(iareg);
                    break;

                case 0xE7:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILELGAL*/

                case 0xE8:
                    IMMBYTE(eaddr);
                    eaddr = isreg + SIGNED(eaddr);
                    break;

                case 0xE9:
                    IMMWORD(eaddr);
                    eaddr += isreg;
                    break;

                case 0xEA:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xEB:
                    eaddr = isreg + GETDREG;
                    break;

                case 0xEC:
                    IMMBYTE(eaddr);
                    eaddr = ipcreg + SIGNED(eaddr);
                    break;

                case 0xED:
                    IMMWORD(eaddr);
                    eaddr += ipcreg;
                    break;

                case 0xEE:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xEF:
                    IMMWORD(eaddr);
                    break;

                case 0xF0:
                    eaddr = isreg;
                    isreg++;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xF1:
                    eaddr = isreg;
                    isreg += 2;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xF2:
                    isreg--;
                    eaddr = isreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xF3:
                    isreg -= 2;
                    eaddr = isreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xF4:
                    eaddr = isreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xF5:
                    eaddr = isreg + SIGNED(ibreg);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xF6:
                    eaddr = isreg + SIGNED(iareg);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xF7:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILELGAL*/

                case 0xF8:
                    IMMBYTE(eaddr);
                    eaddr = isreg + SIGNED(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xF9:
                    IMMWORD(eaddr);
                    eaddr += isreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xFA:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xFB:
                    eaddr = isreg + GETDREG;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xFC:
                    IMMBYTE(eaddr);
                    eaddr = ipcreg + SIGNED(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xFD:
                    IMMWORD(eaddr);
                    eaddr += ipcreg;
                    eaddr = GETWORD(eaddr);
                    break;

                case 0xFE:
                    eaddr = 0;
                    INVALID_POST;
                    break; /*ILLEGAL*/

                case 0xFF:
                    IMMWORD(eaddr);
                    eaddr = GETWORD(eaddr);
                    break;
            }
        }

        switch (ireg)
        {
            case 0x00: /*NEG direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = -k;
                SETSTATUS(0, k, tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x01:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x02:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x03: /*COM direct*/
                DIRECT;
                tb = ~GETBYTE(eaddr);
                SETNZ8(tb);
                SEC;
                CLV;
                SETBYTE(eaddr, tb);
                break;

            case 0x04: /*LSR direct*/
                DIRECT;
                tb = GETBYTE(eaddr);
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x05:
                INVALID_INSTR;
                break;/* ILLEGAL*/

            case 0x06: /*ROR direct*/
                DIRECT;
                tb = (iccreg & 0x01) << 7;
                k = GETBYTE(eaddr);
                if (k & 0x01) SEC;
                else CLC;
                tw = (k >> 1) + tb;
                SETNZ8(tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x07: /*ASR direct*/
                DIRECT;
                tb = GETBYTE(eaddr);
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                if (tb & 0x40)
                    tb |= 0x80;
                SETBYTE(eaddr, tb)
                SETNZ8(tb);
                break;

            case 0x08: /*ASL direct*/
                k = GETBYTE(eaddr);
                DIRECT;
                tw = k << 1;
                SETSTATUS(k, k, tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x09: /*ROL direct*/
                DIRECT;
                tb = GETBYTE(eaddr);
                tw = iccreg & 0x01;
                if (tb & 0x80) SEC;
                else CLC;
                if ((tb & 0x80) ^ ((tb << 1) & 0x80)) SEV;
                else CLV;
                tb = (tb << 1) + tw;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x0A: /*DEC direct*/
                DIRECT;
                tb = GETBYTE(eaddr) - 1;
                if (tb == 0x7F) SEV;
                else CLV;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x0B:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x0C: /*INC direct*/
                DIRECT;
                tb = GETBYTE(eaddr) + 1;
                if (tb == 0x80) SEV;
                else CLV;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x0D: /*TST direct*/
                DIRECT;
                tb = GETBYTE(eaddr);
                SETNZ8(tb);
                break;

            case 0x0E: /*JMP direct*/
                DIRECT;
                ipcreg = eaddr;
                break;

            case 0x0F: /*CLR direct*/
                DIRECT;
                SETBYTE(eaddr, 0);
                CLN;
                CLV;
                SEZ;
                CLC;
                break;

            case 0x10: /* flag10 */
                iflag = 1;
                goto flaginstr;

            case 0x11: /* flag11 */
                iflag = 2;
                goto flaginstr;

            case 0x12: /* NOP */
                break;

            case 0x13: /* SYNC */
                ipcreg--;
                events |= DO_SYNC;
                break;

            case 0x14:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x15:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x16: /*LBRA*/
                IMMWORD(eaddr);
                ipcreg += eaddr;
                break;

            case 0x17: /*LBSR*/
                IMMWORD(eaddr);
                PUSHWORD(ipcreg);
                ipcreg += eaddr;
                break;

            case 0x18:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x19: /* DAA*/
                tw = iareg;
                if (iccreg & 0x20 || (iareg & 0x0f) > 9)
                    tw += 6;
                if (iccreg & 0x01 || (iareg & 0xf0) > 0x90 ||
                    ((iareg & 0xf0) > 0x80 && (iareg & 0x0f) > 0x09))
                    tw += 0x60;
                if (tw & 0x100) SEC;
                if (tw & 0x80) SEN;
                else CLN;
                iareg = (Byte)tw;
                if (iareg) CLZ;
                else SEZ;
                break;

            case 0x1A: /* ORCC*/
                IMMBYTE(tb);
                iccreg |= tb;
                break;

            case 0x1B:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x1C: /* ANDCC*/
                IMMBYTE(tb);
                iccreg &= tb;
                break;

            case 0x1D: /* SEX */
                tw = SIGNED(ibreg);
                SETNZ16(tw);
                SETDREG(tw)
                break;

            case 0x1E: /* EXG */
                IMMBYTE(tb);
                {
                    Word t2;
                    GETREG(tw, tb >> 4);
                    GETREG(t2, tb & 15);
                    SETREG(t2, tb >> 4);
                    SETREG(tw, tb & 15);
                }
                break;

            case 0x1F: /* TFR */
                IMMBYTE(tb);
                GETREG(tw, tb >> 4);
                SETREG(tw, tb & 15);
                break;

            case 0x20: /* (L)BRA*/
                BRANCH(1);
                break;

            case 0x21: /* (L)BRN*/
                BRANCH(0);
                break;

            case 0x22: /* (L)BHI*/
                BRANCH(!(iccreg & 0x05));
                break;

            case 0x23: /* (L)BLS*/
                BRANCH(iccreg & 0x05);
                break;

            case 0x24: /* (L)BCC*/
                BRANCH(!(iccreg & 0x01));
                break;

            case 0x25: /* (L)BCS*/
                BRANCH(iccreg & 0x01);
                break;

            case 0x26: /* (L)BNE*/
                BRANCH(!(iccreg & 0x04));
                break;

            case 0x27: /* (L)BEQ*/
                BRANCH(iccreg & 0x04);
                break;

            case 0x28: /* (L)BVC*/
                BRANCH(!(iccreg & 0x02));
                break;

            case 0x29: /* (L)BVS*/
                BRANCH(iccreg & 0x02);
                break;

            case 0x2A: /* (L)BPL*/
                BRANCH(!(iccreg & 0x08));
                break;

            case 0x2B: /* (L)BMI*/
                BRANCH(iccreg & 0x08);
                break;

            case 0x2C: /* (L)BGE*/
                BRANCH(!NXORV);
                break;

            case 0x2D: /* (L)BLT*/
                BRANCH(NXORV);
                break;

            case 0x2E: /* (L)BGT*/
                BRANCH(!(NXORV || iccreg & 0x04));
                break;

            case 0x2F: /* (L)BLE*/
                BRANCH(NXORV || iccreg & 0x04);
                break;

            case 0x30: /* LEAX*/
                ixreg = eaddr;

                if (ixreg) CLZ;
                else SEZ;
                break;

            case 0x31: /* LEAY*/
                iyreg = eaddr;

                if (iyreg) CLZ;
                else SEZ;
                break;

            case 0x32: /* LEAS*/
                isreg = eaddr;
                break;

            case 0x33: /* LEAU*/
                iureg = eaddr;
                break;

            case 0x34: /* PSHS*/
                IMMBYTE(tb);
                if (tb & 0x80) PUSHWORD(ipcreg);
                if (tb & 0x40) PUSHWORD(iureg);
                if (tb & 0x20) PUSHWORD(iyreg);
                if (tb & 0x10) PUSHWORD(ixreg);
                if (tb & 0x08) PUSHBYTE(idpreg);
                if (tb & 0x04) PUSHBYTE(ibreg);
                if (tb & 0x02) PUSHBYTE(iareg);
                if (tb & 0x01) PUSHBYTE(iccreg);
                break;

            case 0x35: /* PULS*/
                IMMBYTE(tb);
                if (tb & 0x01) PULLBYTE(iccreg);
                if (tb & 0x02) PULLBYTE(iareg);
                if (tb & 0x04) PULLBYTE(ibreg);
                if (tb & 0x08) PULLBYTE(idpreg);
                if (tb & 0x10) PULLWORD(ixreg);
                if (tb & 0x20) PULLWORD(iyreg);
                if (tb & 0x40) PULLWORD(iureg);
                if (tb & 0x80) PULLWORD(ipcreg);
                break;

            case 0x36: /* PSHU*/
                IMMBYTE(tb);
                if (tb & 0x80) PSHUWORD(ipcreg);
                if (tb & 0x40) PSHUWORD(isreg);
                if (tb & 0x20) PSHUWORD(iyreg);
                if (tb & 0x10) PSHUWORD(ixreg);
                if (tb & 0x08) PSHUBYTE(idpreg);
                if (tb & 0x04) PSHUBYTE(ibreg);
                if (tb & 0x02) PSHUBYTE(iareg);
                if (tb & 0x01) PSHUBYTE(iccreg);
                break;

            case 0x37: /* PULU*/
                IMMBYTE(tb);
                if (tb & 0x01) PULUBYTE(iccreg);
                if (tb & 0x02) PULUBYTE(iareg);
                if (tb & 0x04) PULUBYTE(ibreg);
                if (tb & 0x08) PULUBYTE(idpreg);
                if (tb & 0x10) PULUWORD(ixreg);
                if (tb & 0x20) PULUWORD(iyreg);
                if (tb & 0x40) PULUWORD(isreg);
                if (tb & 0x80) PULUWORD(ipcreg);
                break;

            case 0x39: /* RTS*/
                PULLWORD(ipcreg);
                break;

            case 0x3A: /* ABX*/
                ixreg += ibreg;
                break;

            case 0x3B: /* RTI*/
                tb = iccreg & 0x80;
                PULLBYTE(iccreg);

                if (tb)
                {
                    PULLBYTE(iareg);
                    PULLBYTE(ibreg);
                    PULLBYTE(idpreg);
                    PULLWORD(ixreg);
                    PULLWORD(iyreg);
                    PULLWORD(iureg);
                }

                PULLWORD(ipcreg);
                break;

            case 0x3C: /* CWAI*/
                IMMBYTE(tb);
                iccreg &= tb;
                PUSHWORD(ipcreg);
                PUSHWORD(iureg);
                PUSHWORD(iyreg);
                PUSHWORD(ixreg);
                PUSHBYTE(idpreg);
                PUSHBYTE(ibreg);
                PUSHBYTE(iareg);
                PUSHBYTE(iccreg);
                iccreg |= 0x80;
                ipcreg--;
                events |= DO_CWAI;
                ipcreg--;
                break;

            case 0x3D: /* MUL*/
                tw = iareg * ibreg;
                if (tw) CLZ;
                else SEZ;
                if (tw & 0x80) SEC;
                else CLC;
                SETDREG(tw)
                break;

            case 0x3E:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x3F: /* SWI (SWI2 SWI3)*/
            {
                PUSHWORD(ipcreg);
                PUSHWORD(iureg);
                PUSHWORD(iyreg);
                PUSHWORD(ixreg);
                PUSHBYTE(idpreg);
                PUSHBYTE(ibreg);
                PUSHBYTE(iareg);
                PUSHBYTE(iccreg);
                iccreg |= 0x80;

                if (!iflag)
                {
                    iccreg |= 0x50;
                }

                switch (iflag)
                {
                    case 0:
                        ipcreg = GETWORD(0xfffa);
                        break;

                    case 1:
                        ipcreg = GETWORD(0xfff4);
                        break;

                    case 2:
                        ipcreg = GETWORD(0xfff2);
                        break;
                }
            }
            break;

            case 0x40: /*NEGA*/
                tw = -iareg;
                SETSTATUS(0, iareg, tw);
                iareg = (Byte)tw;
                break;

            case 0x41:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x42:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x43: /*COMA*/
                tb = ~iareg;
                SETNZ8(tb);
                SEC;
                CLV;
                iareg = tb;
                break;

            case 0x44: /*LSRA*/
                tb = iareg;
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                SETNZ8(tb);
                iareg = tb;
                break;

            case 0x45:
                INVALID_INSTR;
                break;/* ILLEGAL*/

            case 0x46: /*RORA*/
                tb = (iccreg & 0x01) << 7;
                if (iareg & 0x01) SEC;
                else CLC;
                iareg = (iareg >> 1) + tb;
                SETNZ8(iareg);
                break;

            case 0x47: /*ASRA*/
                tb = iareg;
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                if (tb & 0x40)
                    tb |= 0x80;
                iareg = tb;
                SETNZ8(tb);
                break;

            case 0x48: /*ASLA*/
                tw = iareg << 1;
                SETSTATUS(iareg, iareg, tw);
                iareg = (Byte)tw;
                break;

            case 0x49: /*ROLA*/
                tb = iareg;
                tw = iccreg & 0x01;
                if (tb & 0x80) SEC;
                else CLC;
                if ((tb & 0x80) ^ ((tb << 1) & 0x80)) SEV;
                else CLV;
                tb = (tb << 1) + tw;
                SETNZ8(tb);
                iareg = tb;
                break;

            case 0x4A: /*DECA*/
                tb = iareg - 1;
                if (tb == 0x7F) SEV;
                else CLV;
                SETNZ8(tb);
                iareg = tb;
                break;

            case 0x4B:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x4C: /*INCA*/
                tb = iareg + 1;
                if (tb == 0x80) SEV;
                else CLV;
                SETNZ8(tb);
                iareg = tb;
                break;

            case 0x4D: /*TSTA*/
                SETNZ8(iareg);
                break;

            case 0x4E:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x4F: /*CLRA*/
                iareg = 0;
                CLN;
                CLV;
                SEZ;
                CLC;
                break;

            case 0x50: /*NEGB*/
                tw = -ibreg;
                SETSTATUS(0, ibreg, tw);
                ibreg = (Byte)tw;
                break;

            case 0x51:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x52:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x53: /*COMB*/
                tb = ~ibreg;
                SETNZ8(tb);
                SEC;
                CLV;
                ibreg = tb;
                break;

            case 0x54: /*LSRB*/
                tb = ibreg;
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                SETNZ8(tb);
                ibreg = tb;
                break;

            case 0x55:
                INVALID_INSTR;
                break;/* ILLEGAL*/

            case 0x56: /*RORB*/
                tb = (iccreg & 0x01) << 7;
                if (ibreg & 0x01) SEC;
                else CLC;
                ibreg = (ibreg >> 1) + tb;
                SETNZ8(ibreg);
                break;

            case 0x57: /*ASRB*/
                tb = ibreg;
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                if (tb & 0x40)
                    tb |= 0x80;
                ibreg = tb;
                SETNZ8(tb);
                break;

            case 0x58: /*ASLB*/
                tw = ibreg << 1;
                SETSTATUS(ibreg, ibreg, tw);
                ibreg = (Byte)tw;
                break;

            case 0x59: /*ROLB*/
                tb = ibreg;
                tw = iccreg & 0x01;
                if (tb & 0x80) SEC;
                else CLC;
                if ((tb & 0x80) ^ ((tb << 1) & 0x80)) SEV;
                else CLV;
                tb = (tb << 1) + tw;
                SETNZ8(tb);
                ibreg = tb;
                break;

            case 0x5A: /*DECB*/
                tb = ibreg - 1;
                if (tb == 0x7F) SEV;
                else CLV;
                SETNZ8(tb);
                ibreg = tb;
                break;

            case 0x5B:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x5C: /*INCB*/
                tb = ibreg + 1;
                if (tb == 0x80) SEV;
                else CLV;
                SETNZ8(tb);
                ibreg = tb;
                break;

            case 0x5D: /*TSTB*/
                SETNZ8(ibreg);
                break;

            case 0x5E:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x5F: /*CLRB*/
                ibreg = 0;
                CLN;
                CLV;
                SEZ;
                CLC;
                break;

            case 0x60: /*NEG indexed*/
                k = GETBYTE(eaddr);
                tw = -k;
                SETSTATUS(0, k, tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x61:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x62:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x63: /*COM indexed*/
                tb = ~GETBYTE(eaddr);
                SETNZ8(tb);
                SEC;
                CLV;
                SETBYTE(eaddr, tb)
                break;

            case 0x64: /*LSR indexed*/
                tb = GETBYTE(eaddr);
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x65:
                INVALID_INSTR;
                break;/* ILLEGAL*/

            case 0x66: /*ROR indexed*/
                tb = (iccreg & 0x01) << 7;
                k = GETBYTE(eaddr);
                if (k & 0x01) SEC;
                else CLC;
                tw = (k >> 1) + tb;
                SETNZ8(tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x67: /*ASR indexed*/
                tb = GETBYTE(eaddr);
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                if (tb & 0x40)
                    tb |= 0x80;
                SETBYTE(eaddr, tb)
                SETNZ8(tb);
                break;

            case 0x68: /*ASL indexed*/
                k = GETBYTE(eaddr);
                tw = k << 1;
                SETSTATUS(k, k, tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x69: /*ROL indexed*/
                tb = GETBYTE(eaddr);
                tw = iccreg & 0x01;
                if (tb & 0x80) SEC;
                else CLC;
                if ((tb & 0x80) ^ ((tb << 1) & 0x80)) SEV;
                else CLV;
                tb = (tb << 1) + tw;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x6A: /*DEC indexed*/
                tb = GETBYTE(eaddr) - 1;
                if (tb == 0x7F) SEV;
                else CLV;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x6B:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x6C: /*INC indexed*/
                tb = GETBYTE(eaddr) + 1;
                if (tb == 0x80) SEV;
                else CLV;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x6D: /*TST indexed*/
                tb = GETBYTE(eaddr);
                SETNZ8(tb);
                break;

            case 0x6E: /*JMP indexed*/
                ipcreg = eaddr;
                break;

            case 0x6F: /*CLR indexed*/
                SETBYTE(eaddr, 0)
                CLN;
                CLV;
                SEZ;
                CLC;
                break;

            case 0x70: /*NEG ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = -k;
                SETSTATUS(0, k, tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x71:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x72:
                INVALID_INSTR;
                break;/*ILLEGAL*/

            case 0x73: /*COM ext*/
                EXTENDED;
                tb = ~GETBYTE(eaddr);
                SETNZ8(tb);
                SEC;
                CLV;
                SETBYTE(eaddr, tb)
                break;

            case 0x74: /*LSR ext*/
                EXTENDED;
                tb = GETBYTE(eaddr);
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x75:
                INVALID_INSTR;
                break;/* ILLEGAL*/

            case 0x76: /*ROR ext*/
                EXTENDED;
                tb = (iccreg & 0x01) << 7;
                k = GETBYTE(eaddr);
                if (k & 0x01) SEC;
                else CLC;
                tw = (k >> 1) + tb;
                SETNZ8(tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x77: /*ASR ext*/
                EXTENDED;
                tb = GETBYTE(eaddr);
                if (tb & 0x01) SEC;
                else CLC;
                if (tb & 0x10) SEH;
                else CLH;
                tb >>= 1;
                if (tb & 0x40)
                    tb |= 0x80;
                SETBYTE(eaddr, tb)
                SETNZ8(tb);
                break;

            case 0x78: /*ASL ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = k << 1;
                SETSTATUS(k, k, tw);
                SETBYTE(eaddr, tw)
                break;

            case 0x79: /*ROL ext*/
                EXTENDED;
                tb = GETBYTE(eaddr);
                tw = iccreg & 0x01;
                if (tb & 0x80) SEC;
                else CLC;
                if ((tb & 0x80) ^ ((tb << 1) & 0x80)) SEV;
                else CLV;
                tb = (tb << 1) + tw;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x7A: /*DEC ext*/
                EXTENDED;
                tb = GETBYTE(eaddr) - 1;
                if (tb == 0x7F) SEV;
                else CLV;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x7B:
                INVALID_INSTR;
                break; /*ILLEGAL*/

            case 0x7C: /*INC ext*/
                EXTENDED;
                tb = GETBYTE(eaddr) + 1;
                if (tb == 0x80) SEV;
                else CLV;
                SETNZ8(tb);
                SETBYTE(eaddr, tb)
                break;

            case 0x7D: /*TST ext*/
                EXTENDED;
                tb = GETBYTE(eaddr);
                SETNZ8(tb);
                break;

            case 0x7E: /*JMP ext*/
                EXTENDED;
                ipcreg = eaddr;
                break;

            case 0x7F: /*CLR ext*/
                EXTENDED;
                SETBYTE(eaddr, 0)
                CLN;
                CLV;
                SEZ;
                CLC;
                break;

            case 0x80: /*SUBA immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = iareg - k;
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0x81: /*CMPA immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = iareg - k;
                SETSTATUS(iareg, k, tw);
                break;

            case 0x82: /*SBCA immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = iareg - k - (iccreg & 0x01);
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0x83: /*SUBD (CMPD CMPU) immediate*/
                IMM16
                {
                    unsigned long res, dreg, breg;

                    dreg = (iflag == 2) ? iureg : GETDREG;
                    breg = GETWORD(eaddr);
                    res = dreg - breg;
                    SETSTATUSD(dreg, breg, res);
                    if (iflag == 0) SETDREG(res)
                    }
                break;

            case 0x84: /*ANDA immediate*/
                IMM8;
                iareg = iareg & GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0x85: /*BITA immediate*/
                IMM8;
                tb = iareg & GETBYTE(eaddr);
                SETNZ8(tb);
                CLV;
                break;

            case 0x86: /*LDA immediate*/
                IMM8;
                LOADAC(iareg);
                CLV;
                SETNZ8(iareg);
                break;

            case 0x87: /*STA immediate (for the sake of orthogonality) */
                IMM8;
                SETNZ8(iareg);
                CLV;
                STOREAC(iareg);
                break;

            case 0x88: /*EORA immediate*/
                IMM8;
                iareg = iareg ^ GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0x89: /*ADCA immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = iareg + k + (iccreg & 0x01);
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0x8A: /*ORA immediate*/
                IMM8;
                iareg = iareg | GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0x8B: /*ADDA immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = iareg + k;
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0x8C: /*CMPX (CMPY CMPS) immediate */
                IMM16
                {
                    unsigned long dreg, breg, res;

                    if (iflag == 0)
                        dreg = ixreg;
                    else if (iflag == 1)
                        dreg = iyreg;
                    else
                        dreg = isreg;
                    breg = GETWORD(eaddr);
                    res = dreg - breg;
                    SETSTATUSD(dreg, breg, res);
                }
                break;

            case 0x8D: /*BSR */
                IMMBYTE(tb);
                PUSHWORD(ipcreg);
                ipcreg += SIGNED(tb);
                break;

            case 0x8E: /* LDX (LDY) immediate */
                IMM16 tw = GETWORD(eaddr);
                CLV;
                SETNZ16(tw);
                if (!iflag)
                    ixreg = tw;
                else
                    iyreg = tw;
                break;

            case 0x8F:  /* STX (STY) immediate (orthogonality) */
                IMM16
                if (!iflag)
                    tw = ixreg;
                else
                    tw = iyreg;
                CLV;
                SETNZ16(tw);
                SETWORD(eaddr, tw)
                break;

            case 0x90: /*SUBA direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = iareg - k;
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0x91: /*CMPA direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = iareg - k;
                SETSTATUS(iareg, k, tw);
                break;

            case 0x92: /*SBCA direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = iareg - k - (iccreg & 0x01);
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0x93: /*SUBD (CMPD CMPU) direct*/
                DIRECT;
                {
                    unsigned long res, dreg, breg;

                    if (iflag == 2)
                        dreg = iureg;
                    else
                        dreg = GETDREG;
                    breg = GETWORD(eaddr);
                    res = dreg - breg;
                    SETSTATUSD(dreg, breg, res);
                    if (iflag == 0) SETDREG(res)
                }
                break;

            case 0x94: /*ANDA direct*/
                DIRECT;
                iareg = iareg & GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0x95: /*BITA direct*/
                DIRECT;
                tb = iareg & GETBYTE(eaddr);
                SETNZ8(tb);
                CLV;
                break;

            case 0x96: /*LDA direct*/
                DIRECT;
                LOADAC(iareg);
                CLV;
                SETNZ8(iareg);
                break;

            case 0x97: /*STA direct */
                DIRECT;
                SETNZ8(iareg);
                CLV;
                STOREAC(iareg);
                break;

            case 0x98: /*EORA direct*/
                DIRECT;
                iareg = iareg ^ GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0x99: /*ADCA direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = iareg + k + (iccreg & 0x01);
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0x9A: /*ORA direct*/
                DIRECT;
                iareg = iareg | GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0x9B: /*ADDA direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = iareg + k;
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0x9C: /*CMPX (CMPY CMPS) direct */
                DIRECT;
                {
                    unsigned long dreg, breg, res;

                    if (iflag == 0)
                        dreg = ixreg;
                    else if (iflag == 1)
                        dreg = iyreg;
                    else
                        dreg = isreg;
                    breg = GETWORD(eaddr);
                    res = dreg - breg;
                    SETSTATUSD(dreg, breg, res);
                }
                break;

            case 0x9D: /*JSR direct */
                DIRECT;
                PUSHWORD(ipcreg);
                ipcreg = eaddr;
                break;

            case 0x9E: /* LDX (LDY) direct */
                DIRECT;
                tw = GETWORD(eaddr);
                CLV;
                SETNZ16(tw);
                if (!iflag)
                    ixreg = tw;
                else
                    iyreg = tw;
                break;

            case 0x9F:  /* STX (STY) direct */
                DIRECT;
                tw = (!iflag) ? ixreg : iyreg;
                CLV;
                SETNZ16(tw);
                SETWORD(eaddr, tw)
                break;

            case 0xA0: /*SUBA indexed*/
                k = GETBYTE(eaddr);
                tw = iareg - k;
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0xA1: /*CMPA indexed*/
                k = GETBYTE(eaddr);
                tw = iareg - k;
                SETSTATUS(iareg, k, tw);
                break;

            case 0xA2: /*SBCA indexed*/
                k = GETBYTE(eaddr);
                tw = iareg - k - (iccreg & 0x01);
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0xA3: /*SUBD (CMPD CMPU) indexed*/
            {
                unsigned long res, dreg, breg;
                dreg = (iflag == 2) ? iureg : GETDREG;
                breg = GETWORD(eaddr);
                res = dreg - breg;
                SETSTATUSD(dreg, breg, res);
                if (iflag == 0)
                    SETDREG(res)
            }
            break;

            case 0xA4: /*ANDA indexed*/
                iareg = iareg & GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0xA5: /*BITA indexed*/
                tb = iareg & GETBYTE(eaddr);
                SETNZ8(tb);
                CLV;
                break;

            case 0xA6: /*LDA indexed*/
                LOADAC(iareg);
                CLV;
                SETNZ8(iareg);
                break;

            case 0xA7: /*STA indexed */
                SETNZ8(iareg);
                CLV;
                STOREAC(iareg);
                break;

            case 0xA8: /*EORA indexed*/
                iareg = iareg ^ GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0xA9: /*ADCA indexed*/
                k = GETBYTE(eaddr);
                tw = iareg + k + (iccreg & 0x01);
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0xAA: /*ORA indexed*/
                iareg = iareg | GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0xAB: /*ADDA indexed*/
                k = GETBYTE(eaddr);
                tw = iareg + k;
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0xAC: /*CMPX (CMPY CMPS) indexed */
            {
                unsigned long dreg, breg, res;
                if (iflag == 0)
                    dreg = ixreg;
                else if (iflag == 1)
                    dreg = iyreg;
                else
                    dreg = isreg;
                breg = GETWORD(eaddr);
                res = dreg - breg;
                SETSTATUSD(dreg, breg, res);
            }
            break;

            case 0xAD: /*JSR indexed */
                PUSHWORD(ipcreg);
                ipcreg = eaddr;
                break;

            case 0xAE: /* LDX (LDY) indexed */
                tw = GETWORD(eaddr);
                CLV;
                SETNZ16(tw);
                if (!iflag)
                    ixreg = tw;
                else
                    iyreg = tw;
                break;

            case 0xAF:  /* STX (STY) indexed */
                tw = (!iflag) ? ixreg : iyreg;
                CLV;
                SETNZ16(tw);
                SETWORD(eaddr, tw)
                break;

            case 0xB0: /*SUBA ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = iareg - k;
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0xB1: /*CMPA ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = iareg - k;
                SETSTATUS(iareg, k, tw);
                break;

            case 0xB2: /*SBCA ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = iareg - k - (iccreg & 0x01);
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0xB3: /*SUBD (CMPD CMPU) ext*/
                EXTENDED;
                {
                    unsigned long res, dreg, breg;

                    dreg = (iflag == 2) ? iureg : GETDREG;
                    breg = GETWORD(eaddr);
                    res = dreg - breg;
                    SETSTATUSD(dreg, breg, res);
                    if (iflag == 0)
                        SETDREG(res)
                }
                break;

            case 0xB4: /*ANDA ext*/
                EXTENDED;
                iareg = iareg & GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0xB5: /*BITA ext*/
                EXTENDED;
                tb = iareg & GETBYTE(eaddr);
                SETNZ8(tb);
                CLV;
                break;

            case 0xB6: /*LDA ext*/
                EXTENDED;
                LOADAC(iareg);
                CLV;
                SETNZ8(iareg);
                break;

            case 0xB7: /*STA ext */
                EXTENDED;
                SETNZ8(iareg);
                CLV;
                STOREAC(iareg);
                break;

            case 0xB8: /*EORA ext*/
                EXTENDED;
                iareg = iareg ^ GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0xB9: /*ADCA ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = iareg + k + (iccreg & 0x01);
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0xBA: /*ORA ext*/
                EXTENDED;
                iareg = iareg | GETBYTE(eaddr);
                SETNZ8(iareg);
                CLV;
                break;

            case 0xBB: /*ADDA ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = iareg + k;
                SETSTATUS(iareg, k, tw);
                iareg = (Byte)tw;
                break;

            case 0xBC: /*CMPX (CMPY CMPS) ext */
                EXTENDED;
                {
                    unsigned long dreg, breg, res;

                    if (iflag == 0)
                        dreg = ixreg;
                    else if (iflag == 1)
                        dreg = iyreg;
                    else
                        dreg = isreg;
                    breg = GETWORD(eaddr);
                    res = dreg - breg;
                    SETSTATUSD(dreg, breg, res);
                }
                break;

            case 0xBD: /*JSR ext */
                EXTENDED;
                PUSHWORD(ipcreg);
                ipcreg = eaddr;
                break;

            case 0xBE: /* LDX (LDY) ext */
                EXTENDED;
                tw = GETWORD(eaddr);
                CLV;
                SETNZ16(tw);
                if (!iflag)
                    ixreg = tw;
                else
                    iyreg = tw;
                break;

            case 0xBF:  /* STX (STY) ext */
                EXTENDED;
                tw = (!iflag) ? ixreg : iyreg;
                CLV;
                SETNZ16(tw);
                SETWORD(eaddr, tw)
                break;

            case 0xC0: /*SUBB immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = ibreg - k;
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xC1: /*CMPB immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = ibreg - k;
                SETSTATUS(ibreg, k, tw);
                break;

            case 0xC2: /*SBCB immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = ibreg - k - (iccreg & 0x01);
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xC3: /*ADDD immediate*/
                IMM16
                {
                    unsigned long res, dreg, breg;
                    dreg = GETDREG;
                    breg = GETWORD(eaddr);
                    res = dreg + breg;
                    SETSTATUSD(dreg, breg, res);
                    SETDREG(res)
                }
                break;

            case 0xC4: /*ANDB immediate*/
                IMM8;
                ibreg = ibreg & GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xC5: /*BITB immediate*/
                IMM8;
                tb = ibreg & GETBYTE(eaddr);
                SETNZ8(tb);
                CLV;
                break;

            case 0xC6: /*LDB immediate*/
                IMM8;
                LOADAC(ibreg);
                CLV;
                SETNZ8(ibreg);
                break;

            case 0xC7: /*STB immediate (for the sake of orthogonality) */
                IMM8;
                SETNZ8(ibreg);
                CLV;
                STOREAC(ibreg);
                break;

            case 0xC8: /*EORB immediate*/
                IMM8;
                ibreg = ibreg ^ GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xC9: /*ADCB immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = ibreg + k + (iccreg & 0x01);
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xCA: /*ORB immediate*/
                IMM8;
                ibreg = ibreg | GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xCB: /*ADDB immediate*/
                IMM8;
                k = GETBYTE(eaddr);
                tw = ibreg + k;
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xCC: /*LDD immediate */
                IMM16 tw = GETWORD(eaddr);
                SETNZ16(tw);
                CLV;
                SETDREG(tw)
                break;

            case 0xCD: /*STD immediate (orthogonality) */
                IMM16
                tw = GETDREG;
                SETNZ16(tw);
                CLV;
                SETWORD(eaddr, tw)
                break;

            case 0xCE: /* LDU (LDS) immediate */
                IMM16 tw = GETWORD(eaddr);
                CLV;
                SETNZ16(tw);
                if (!iflag)
                    iureg = tw;
                else
                    isreg = tw;
                break;

            case 0xCF:  /* STU (STS) immediate (orthogonality) */
                IMM16
                tw = (!iflag) ? iureg : isreg;
                CLV;
                SETNZ16(tw);
                SETWORD(eaddr, tw)
                break;

            case 0xD0: /*SUBB direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = ibreg - k;
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xD1: /*CMPB direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = ibreg - k;
                SETSTATUS(ibreg, k, tw);
                break;

            case 0xD2: /*SBCB direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = ibreg - k - (iccreg & 0x01);
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xD3: /*ADDD direct*/
                DIRECT;
                {
                    unsigned long res, dreg, breg;
                    dreg = GETDREG;
                    breg = GETWORD(eaddr);
                    res = dreg + breg;
                    SETSTATUSD(dreg, breg, res);
                    SETDREG(res)
                }
                break;

            case 0xD4: /*ANDB direct*/
                DIRECT;
                ibreg = ibreg & GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xD5: /*BITB direct*/
                DIRECT;
                tb = ibreg & GETBYTE(eaddr);
                SETNZ8(tb);
                CLV;
                break;

            case 0xD6: /*LDB direct*/
                DIRECT;
                LOADAC(ibreg);
                CLV;
                SETNZ8(ibreg);
                break;

            case 0xD7: /*STB direct  */
                DIRECT;
                SETNZ8(ibreg);
                CLV;
                STOREAC(ibreg);
                break;

            case 0xD8: /*EORB direct*/
                DIRECT;
                ibreg = ibreg ^ GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xD9: /*ADCB direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = ibreg + k + (iccreg & 0x01);
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xDA: /*ORB direct*/
                DIRECT;
                ibreg = ibreg | GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xDB: /*ADDB direct*/
                DIRECT;
                k = GETBYTE(eaddr);
                tw = ibreg + k;
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xDC: /*LDD direct */
                DIRECT;
                tw = GETWORD(eaddr);
                SETNZ16(tw);
                CLV;
                SETDREG(tw)
                break;

            case 0xDD: /*STD direct  */
                DIRECT;
                tw = GETDREG;
                SETNZ16(tw);
                CLV;
                SETWORD(eaddr, tw)
                break;

            case 0xDE: /* LDU (LDS) direct */
                DIRECT;
                tw = GETWORD(eaddr);
                CLV;
                SETNZ16(tw);
                if (!iflag)
                    iureg = tw;
                else
                    isreg = tw;
                break;

            case 0xDF:  /* STU (STS) direct  */
                DIRECT;
                tw = (!iflag) ? iureg : isreg;
                CLV;
                SETNZ16(tw);
                SETWORD(eaddr, tw)
                break;

            case 0xE0: /*SUBB indexed*/
                k = GETBYTE(eaddr);
                tw = ibreg - k;
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xE1: /*CMPB indexed*/
                k = GETBYTE(eaddr);
                tw = ibreg - k;
                SETSTATUS(ibreg, k, tw);
                break;

            case 0xE2: /*SBCB indexed*/
                k = GETBYTE(eaddr);
                tw = ibreg - k - (iccreg & 0x01);
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xE3: /*ADDD indexed*/
            {
                unsigned long res, dreg, breg;
                dreg = GETDREG;
                breg = GETWORD(eaddr);
                res = dreg + breg;
                SETSTATUSD(dreg, breg, res);
                SETDREG(res)
            }
            break;

            case 0xE4: /*ANDB indexed*/
                ibreg = ibreg & GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xE5: /*BITB indexed*/
                tb = ibreg & GETBYTE(eaddr);
                SETNZ8(tb);
                CLV;
                break;

            case 0xE6: /*LDB indexed*/
                LOADAC(ibreg);
                CLV;
                SETNZ8(ibreg);
                break;

            case 0xE7: /*STB indexed  */
                SETNZ8(ibreg);
                CLV;
                STOREAC(ibreg);
                break;

            case 0xE8: /*EORB indexed*/
                ibreg = ibreg ^ GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xE9: /*ADCB indexed*/
                k = GETBYTE(eaddr);
                tw = ibreg + k + (iccreg & 0x01);
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xEA: /*ORB indexed*/
                ibreg = ibreg | GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xEB: /*ADDB indexed*/
                k = GETBYTE(eaddr);
                tw = ibreg + k;
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xEC: /*LDD indexed */
                tw = GETWORD(eaddr);
                SETNZ16(tw);
                CLV;
                SETDREG(tw)
                break;

            case 0xED: /*STD indexed  */
                tw = GETDREG;
                SETNZ16(tw);
                CLV;
                SETWORD(eaddr, tw)
                break;

            case 0xEE: /* LDU (LDS) indexed */
                tw = GETWORD(eaddr);
                CLV;
                SETNZ16(tw);
                if (!iflag)
                    iureg = tw;
                else
                    isreg = tw;
                break;

            case 0xEF:  /* STU (STS) indexed  */
                tw = (!iflag) ? iureg : isreg;
                CLV;
                SETNZ16(tw);
                SETWORD(eaddr, tw)
                break;

            case 0xF0: /*SUBB ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = ibreg - k;
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xF1: /*CMPB ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = ibreg - k;
                SETSTATUS(ibreg, k, tw);
                break;

            case 0xF2: /*SBCB ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = ibreg - k - (iccreg & 0x01);
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xF3: /*ADDD ext*/
                EXTENDED;
                {
                    unsigned long res, dreg, breg;
                    dreg = GETDREG;
                    breg = GETWORD(eaddr);
                    res = dreg + breg;
                    SETSTATUSD(dreg, breg, res);
                    SETDREG(res)
                }
                break;

            case 0xF4: /*ANDB ext*/
                EXTENDED;
                ibreg = ibreg & GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xF5: /*BITB ext*/
                EXTENDED;
                tb = ibreg & GETBYTE(eaddr);
                SETNZ8(tb);
                CLV;
                break;

            case 0xF6: /*LDB ext*/
                EXTENDED;
                LOADAC(ibreg);
                CLV;
                SETNZ8(ibreg);
                break;

            case 0xF7: /*STB ext  */
                EXTENDED;
                SETNZ8(ibreg);
                CLV;
                STOREAC(ibreg);
                break;

            case 0xF8: /*EORB ext*/
                EXTENDED;
                ibreg = ibreg ^ GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xF9: /*ADCB ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = ibreg + k + (iccreg & 0x01);
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xFA: /*ORB ext*/
                EXTENDED;
                ibreg = ibreg | GETBYTE(eaddr);
                SETNZ8(ibreg);
                CLV;
                break;

            case 0xFB: /*ADDB ext*/
                EXTENDED;
                k = GETBYTE(eaddr);
                tw = ibreg + k;
                SETSTATUS(ibreg, k, tw);
                ibreg = (Byte)tw;
                break;

            case 0xFC: /*LDD ext */
                EXTENDED;
                tw = GETWORD(eaddr);
                SETNZ16(tw);
                CLV;
                SETDREG(tw)
                break;

            case 0xFD: /*STD ext  */
                EXTENDED;
                tw = GETDREG;
                SETNZ16(tw);
                CLV;
                SETWORD(eaddr, tw)
                break;

            case 0xFE: /* LDU (LDS) ext */
                EXTENDED;
                tw = GETWORD(eaddr);
                CLV;
                SETNZ16(tw);
                if (!iflag)
                    iureg = tw;
                else
                    isreg = tw;
                break;

            case 0xFF:  /* STU (STS) ext  */
                EXTENDED;
                tw = (!iflag) ? iureg : isreg;
                CLV;
                SETNZ16(tw);
                SETWORD(eaddr, tw)
                break;
        }

        cycles += 48; // return mean value for cycles*10/instruction

