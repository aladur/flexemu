/*
    fromflex.c


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997,2026  W. Schwotzer

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


/*
    convert a text file from FLEX to native file format (UNIX)
*/

#include <stdio.h>

int main(int argc, char **argv)
{
    int c;

    if (argc > 1)
    {
        fprintf(stderr, "*** Error: Superfluous parameter: %s\n", argv[1]);
        fprintf(stderr, "syntax: fromflex\n");
        fprintf(stderr, " read from stdin\n");
        fprintf(stderr, " write to stdout\n");
        return 1;
    }

    while ((c = getchar()) != EOF)
    {
        if (c == 0x0d)
        {
            putchar('\n');
        }
        else if (c == 0x09)
        {
            /* expand space compression */
            c = getchar();
            if (c != EOF)
            {
                while (c--)
                {
                    putchar(' ');
                }
            }
            else
            {
                fprintf(stderr, "warning: file maybe corrupt\n");
                putchar(' ');
            }
        }
        else if (c == 0x00 || c == 0x0a)
        {
            /* Ignore NUL or CR (carriage return) */
        }
        else
        {
            putchar(c);
        }
    }

    return 0;
}

