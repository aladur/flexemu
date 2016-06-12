/*
    toflex.c


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997  W. Schwotzer

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
	convert a text file from UNIX to FLEX format

	input:  read from stdin
	output: write to stdout
*/


#include <stdlib.h>
#include <stdio.h>
#include "../typedefs.h"

int main(int argc, char *argv[])
{
	SWord c, spaces = 0;

	if (argc > 1) {
		fprintf(stderr, "syntax: toflex\n");
		fprintf(stderr, " read from stdin\n");
		fprintf(stderr, " write to stdout\n");
		exit(1);
	}
	while ((c = getchar()) != EOF) {
		if (c != ' ' && c != '\t' && spaces) {
			if (spaces > 1) {
				putchar(0x09);
				putchar(spaces);
			} else
				putchar(' ');
			spaces = 0;
		}
		if (c == ' ') {
			/* do space compression */
			if (++spaces == 127) {
				putchar(0x09);
				putchar(spaces);
				spaces = 0;
			}
		} else
		if (c == '\t') {
			/* tab will be converted to 8 spaces */
			if (spaces >= 127 - 8) {
				putchar(0x09);
				putchar(127);
				spaces -= 127 - 8;
			} else
				spaces += 8;
		} else
		if (c == '\n')
			putchar(0x0d);
		else
			putchar(c);
	} /* while */
	if (spaces) {
		putchar(0x09);
		putchar(spaces);
	}
	exit(0);
}

