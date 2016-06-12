/*
    misc1.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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
#include <ctype.h>
#include <stdio.h>
#include "bfileptr.h"

const char *gMemoryAllocationErrorString =
			"Bad memory allocation.\n"
			"Close other applications\n"
			"and try again.";

int copyFile(const char *srcPath, const char *destPath)
{
	BFilePtr sFp(srcPath,  "rb");
	BFilePtr dFp(destPath, "wb");
	int c;

	if (sFp == NULL || dFp == NULL)
		return 0;
	while ((c = fgetc(sFp)) != EOF)
		fputc(c, dFp);
	return 1;       
}

void strupper(char *pstr)
{
	while (*pstr)
	{
		*pstr = toupper(*pstr);
		pstr++;
	}
} // strupper


void strlower(char *pstr)
{
	while (*pstr)
	{
		*pstr = tolower(*pstr);
		pstr++;
	}
} // strlower

// Base 2 and Base 16 conversion functions
char *binstr(Byte x)
{
	static char             tmp[9] = "        ";

	for (SWord i = 7; i >= 0; --i) {
		tmp[i] = (x & 1) + '0';
		x >>= 1;
	}

	return tmp;
}

static char hex_digit(Byte x)
{
	x &= 0x0f;
	if (x <= 9) {
		return '0' + x;
	} else {
		return 'a' + x - 10;
	}
}

char *hexstr(Byte x)
{
	static char             tmp[3] = "  ";

	tmp[1] = hex_digit(x);  x >>= 4;
	tmp[0] = hex_digit(x);

	return tmp;
}

char *hexstr(Word x)
{
	static char             tmp[5] = "    ";

	tmp[3] = hex_digit((Byte)x);    x >>= 4;
	tmp[2] = hex_digit((Byte)x);    x >>= 4;
	tmp[1] = hex_digit((Byte)x);    x >>= 4;
	tmp[0] = hex_digit((Byte)x);

	return tmp;
}

char *ascchr(Byte x)
{
	static char             tmp[2] = " ";

	x &= 0x7f;
	tmp[0] = ((x >= 0x20) && (x < 0x7f)) ? x : '.';

	return tmp;
}

#if defined(__GNUC__) && !(defined(__MINGW32) || defined (__CYGWIN32) )
int stricmp(const char *string1, const char *string2)
{
	unsigned int i;

	for (i = 0; i < strlen(string1); i++) {
		if (tolower(*(string1 + i)) < tolower(*(string2 + i)))
			return -1;
		if (tolower(*(string1 + i)) > tolower(*(string2 + i)))
			return 1;
		if (!*string1)
			return 0;
	}
	return 0;
} // stricmp
#endif

