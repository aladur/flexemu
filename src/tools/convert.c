/*
    convert.c


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2026  W. Schwotzer

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

#include "convert.h"
#include <stdio.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void convert_from_flex(FILE *fin, FILE *fout)
{
    int ch;

    while ((ch = fgetc(fin)) != EOF)
    {
        if (ch == 0x0d)
        {
            fputc('\n', fout);
        }
        else if (ch == 0x09)
        {
            /* expand space compression */
            ch = fgetc(fin);
            if (ch != EOF)
            {
                while (ch--)
                {
                    fputc(' ', fout);
                }
            }
            else
            {
                fprintf(stderr, "warning: file maybe corrupt\n");
                fputc(' ', fout);
            }
        }
        else if (ch == 0x00 || ch == 0x0a)
        {
            /* Ignore NUL or CR (carriage return) */
        }
        else
        {
            fputc(ch, fout);
        }
    }
}

void convert_to_flex(FILE *fin, FILE *fout)
{
    int ch = 0;
    int spaces = 0;

    while ((ch = fgetc(fin)) != EOF)
    {
        if (ch != ' ' && ch != '\t' && spaces)
        {
            if (spaces > 1)
            {
                fputc(0x09, fout);
                fputc(spaces, fout);
            }
            else
            {
                fputc(' ', fout);
            }
            spaces = 0;
        }
        if (ch == ' ')
        {
            /* do space compression */
            if (++spaces == 127)
            {
                fputc(0x09, fout);
                fputc(spaces, fout);
                spaces = 0;
            }
        } else if (ch == '\t')
        {
            /* tab will be converted to 8 spaces */
            if (spaces >= 127 - 8)
            {
                fputc(0x09, fout);
                fputc(127, fout);
                spaces -= 127 - 8;
            }
            else
            {
                spaces += 8;
            }
        } else if (ch == '\n')
        {
            fputc(0x0d, fout);
        }
        else
        {
            fputc(ch, fout);
        }
    }

    if (spaces)
    {
        fputc(0x09, fout);
        fputc(spaces, fout);
    }
}

int convert_files(const char *ifiles[], int ifiles_count, const char *ofile,
        convert_fct convert_file)
{
    FILE *fin = stdin;
    FILE *fout = stdout;
    int index;

    if (ofile != NULL)
    {
        fout = fopen(ofile, "w");
        if (fout == NULL)
        {
            fprintf(stderr, "*** Error: Opening '%s' for writing.\n", ofile);
            return 1;
        }
    }

    if (ifiles_count == 0)
    {
        convert_file(fin, fout);
        return 0;
    }

    for (index = 0; index < ifiles_count; ++index)
    {
        fin = fopen(ifiles[index], "r");
        if (fin == NULL)
        {
            fprintf(stderr, "*** Error: Opening '%s' for reading.\n",
                    ifiles[index]);
            if (ofile != NULL)
            {
                fclose(fout);
            }
            return 1;
        }

        convert_file(fin, fout);

        fclose(fin);
    }

    if (ofile != NULL)
    {
        fclose(fout);

        if (ifiles_count > 0)
        {
            struct stat sbuf;

            if (stat(ifiles[0], &sbuf) == 0)
            {
                struct utimbuf timebuf;

                timebuf.actime = sbuf.st_atime;
                timebuf.modtime = sbuf.st_mtime;
                utime(ofile, &timebuf);
            }
        }
    }

    return 0;
}

