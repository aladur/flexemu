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
    Convert a FLEX text file to a regular text file.
*/

#include "convert.h"
#include <stdio.h>
#ifdef __linux__
#include <getopt.h>
#endif
#include <unistd.h>

extern char *optarg;

void usage()
{
    fprintf(stderr,
"Usage: fromflex [INPUT]... [OUTPUT]\n"
"Convert FLEX text file(s) into a regular text file.\n"
"Usage: fromflex [-i <flex_text_file> [ -i <...> [...]]] [-o <text_file>]\n"
"\n"
"INPUT:\n"
"  -i <flex_text_file>: FLEX text file, multiple input options are possible.\n"
"                       Default is to read from standard input.\n"
"OUTPUT:\n"
"  -o <text_file>:      Text file. Default is to write to standard output.\n"
"\n"
"If input and output are files the access and modification timestamp from\n"
"first input file is copied to output file.\n"
    );
}

int main(int argc, char **argv)
{
    const char *optstr = "i:o:h";
    int result;
    const char *ifiles[argc];
    const char *ofile = NULL;
    int count = 0;

    while ((result = getopt(argc, argv, optstr)) != -1)
    {
        switch (result)
        {
            case 'i':
                ifiles[count++] = optarg;
                break;

            case 'o':
                ofile = optarg;
                break;

            case 'h':
                usage();
                return 0;
        }
    }

    return convert_files(ifiles, count, ofile, convert_from_flex);
}

