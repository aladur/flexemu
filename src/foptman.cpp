/*
    foptman.cpp


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
#include <signal.h>
#include <new>

#include "absgui.h"
#include "foptman.h"
#include "bregistr.h"
#include "brcfile.h"
#include "benv.h"

#ifndef UNIX
    extern int optind;
    extern int opterr;
    extern char *optarg;
#endif


#ifdef WIN32
// uses its own implementation of getopt
int optind = 1;
int opterr = 0;
char *optarg = NULL;

int getopt(int argc, char *const argv[], char *optstr)
{
    int     i;

    while (1)
    {
        char    opt;

        optarg = optstr;

        for (i = 1; i < optind; i++)
        {
            if (*(++optarg) == ':')
            {
                optarg++;
            }
        }

        if ((opt = *optarg) == '\0')
        {
            return -1;
        }

        optind++;
        i = 1;

        while (i < argc)
        {
            if (argv[i][0] == '-' && argv[i][1] == opt)
            {
                // found option
                if (*(optarg + 1) == ':')
                {
                    // option has a parameter
                    if (argv[i][2] != '\0')
                    {
                        optarg = &argv[i][2];
                        return opt;
                    }
                    else
                    {
                        if (++i < argc)
                        {
                            optarg = &argv[i][0];
                            return opt;
                        }
                        else
                        {
                            break;
                        }
                    } // else
                }
                else
                    // option has no parameter
                {
                    return opt;
                }
            } // if

            i++;
        } // while
    } // while

    return -1;
} // getopt
#endif


void FlexOptionManager::PrintHelp(FILE *fp)
{
    fprintf(fp, "usage: flexemu <options>\n");
    fprintf(fp, "  <options> are:\n");
    fprintf(fp, "  -f <hexfile>\n");
    fprintf(fp, "  -0 <diskimage/directory drive 0>\n");
    fprintf(fp, "  -1 <diskimage/directory drive 1>\n");
    fprintf(fp, "  -2 <diskimage/directory drive 2>\n");
    fprintf(fp, "  -3 <diskimage/directory drive 3>\n");
    fprintf(fp, "  -p (directory for FLEX disks)\n");
    fprintf(fp, "  -j <factor for screen width>\n");
    fprintf(fp, "  -k <factor for screen height>\n");
    fprintf(fp, "  -m (use 2 x 288 K RAM extension)\n");
    fprintf(fp, "  -u (support undocumented MC6809 processor instructions)\n");
#ifdef HAVE_TERMIOS_H
    fprintf(fp, "  -t (terminal only mode)\n");
    fprintf(fp, "  -r <two-hex-digit reset key>\n");
#endif
#ifdef HAVE_X11
    fprintf(fp, "  -s (run X11 synchronized)\n");
    fprintf(fp, "  -c <color> define foreground color\n");
    fprintf(fp, "  -i (display inverse video)\n");
    fprintf(fp, "  -n <# of colors>\n");
#ifdef WIN32
    fprintf(fp, "  -g [win32|x11] (select type of GUI)\n");
#endif
#endif
    fprintf(fp, "  -h (display this)\n");
    fprintf(fp, "  -? (display this)\n");
    fprintf(fp, "  -v (print version number)\n");
} // PrintHelp

void FlexOptionManager::InitOptions(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions,
    int argc,
    char *const argv[])
{
    pOptions->drive[0]         = "system.dsk";
    pOptions->drive[1]         = "";
    pOptions->drive[2]         = "";
    pOptions->drive[3]         = "";
    pOptions->hex_file         = "neumon54.hex";
    pOptions->term_mode        = false;
    pOptions->isHiMem          = false;
    pOptions->use_undocumented = false;
#ifndef WIN32
#ifndef HAVE_X11
    pOptions->term_mode        = true;
#endif
#endif
    pOptions->reset_key        = 0x1e; // is Ctrl-^ for reset or Sig. INT

    pGuiOptions->argc          = argc;
    pGuiOptions->argv          = argv;
    pGuiOptions->color         = "green";
    pGuiOptions->nColors       = 2;
    pGuiOptions->inverse       = 0;
#ifdef UNIX
    pGuiOptions->html_viewer   = "firefox";
#endif
#ifdef UNIX
    pGuiOptions->doc_dir       = F_DATADIR;
    pGuiOptions->guiType       = GUI_XTOOLKIT;
    pOptions->disk_dir         = F_DATADIR;
#endif
#ifdef WIN32
    pGuiOptions->doc_dir       = "";
    pGuiOptions->guiType       = GUI_WINDOWS;
    pOptions->disk_dir         = "";
#endif
    pGuiOptions->guiXSize      = 2;
    pGuiOptions->guiYSize      = 2;
    pGuiOptions->synchronized  = 0;
    // automatic SERPAR switch only if Eurocom II Monitorprogram loaded
    pGuiOptions->switch_sp = !strcmp(pOptions->hex_file, "neumon54.hex");
} // InitOptions

void FlexOptionManager::GetEnvironmentOptions(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions)
{
#ifdef UNIX
    BString str;
    int value;
    BEnvironment env;

    // first look for environment variables
    if (env.GetValue((const char *)"FLEX" FLEXINVERSE, &value))
    {
        pGuiOptions->inverse = value;
    }

    if (env.GetValue((const char *)"FLEX" FLEXHIMEM, &value))
    {
        pOptions->isHiMem = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXUNDOCUMENTED, &value))
    {
        pOptions->use_undocumented = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXHTMLVIEWER, str))
    {
        pGuiOptions->html_viewer = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDOCDIR, str))
    {
        pGuiOptions->doc_dir = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXMONITOR, str))
    {
        pOptions->hex_file = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXCOLOR, str))
    {
        pGuiOptions->color = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXNCOLORS, &value))
    {
        pGuiOptions->nColors = value;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISKDIR, str))
    {
        pOptions->disk_dir = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISK0, str))
    {
        pOptions->drive[0] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISK1, str))
    {
        pOptions->drive[1] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISK2, str))
    {
        pOptions->drive[2] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISK3, str))
    {
        pOptions->drive[3] = str;
    }

#endif  // ifdef UNIX
}

void FlexOptionManager::GetCommandlineOptions(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions,
    int argc,
    char *const argv[])
{
    char    optstr[32];
    int     i;
    optind = 1;
    opterr = 0;
    strcpy((char *)optstr, "mup:f:0:1:2:3:j:k:");
#ifdef HAVE_TERMIOS_H
    strcat((char *)optstr, "tr:");  // terminal mode and reset key
#endif
#ifdef HAVE_X11
    strcat(optstr, "s");            // X11 synchronised
#ifdef WIN32
    strcat(optstr, "g:");           // Select type of GUI
#endif
#endif
#if defined (HAVE_X11) || defined(WIN32)
    strcat(optstr, "ic:n:");          // color, inverse video, # of colors
    strcat((char *)optstr, "vh");   // version and help
#endif

    while (1)
    {
        int result = getopt(argc, argv, optstr);

        if (result == -1)
        {
            break;
        }

        switch (result)
        {
            case 'f':
                pOptions->hex_file            = optarg;
                break;

            case '0':
                pOptions->drive[0]           = optarg;
                break;

            case '1':
                pOptions->drive[1]           = optarg;
                break;

            case '2':
                pOptions->drive[2]           = optarg;
                break;

            case '3':
                pOptions->drive[3]           = optarg;
                break;

            case 'p':
                pOptions->disk_dir            = optarg;
                break;

            case 'm':
                pOptions->isHiMem = true;
                break;

            case 'u':
                pOptions->use_undocumented = true;
                break;

            case 'j':
                sscanf(optarg, "%d", &i);

                if (i > 0 && i <= MAX_GUIXSIZE)
                {
                    pGuiOptions->guiXSize = i;
                }

                break;

            case 'k':
                sscanf(optarg, "%d", &i);

                if (i > 0 && i <= MAX_GUIYSIZE)
                {
                    pGuiOptions->guiYSize = i;
                }

                break;
#ifdef HAVE_TERMIOS_H

            case 't':
                pOptions->term_mode           = true;
                break;

            case 'r':
                sscanf(optarg, "%hx", (Word *)&pOptions->reset_key);
                break;
#endif
#ifdef HAVE_X11

            case 's':
                pGuiOptions->synchronized     = 1;
                break;
#ifdef WIN32

            case 'g':
                if (stricmp(optarg, "win32") == 0)
                {
                    pGuiOptions->guiType = GUI_WINDOWS;
                }

                if (stricmp(optarg, "x11") == 0)
                {
                    pGuiOptions->guiType = GUI_XTOOLKIT;
                }

                break;
#endif
#endif
#if defined (HAVE_X11) || defined(WIN32)

            case 'n':
                sscanf(optarg, "%d", &i);
                pGuiOptions->nColors          = i;
                break;

            case 'c':
                pGuiOptions->color            = optarg;
                break;

            case 'i':
                pGuiOptions->inverse          = 1;
                break;

            case 'v':
                fprintf(stdout, PROGRAMNAME ": V %s\n", PROGRAM_VERSION);
                exit(EXIT_SUCCESS);

            case '?':
            case 'h':
                PrintHelp(stderr);
                exit(EXIT_SUCCESS);
#endif
        }  // switch
    } // while
} // GetCommandlineOptions


void FlexOptionManager::WriteOptions(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions,
    bool  ifNotExists /* = false */
)
{
#ifdef WIN32
    BRegistry *reg = NULL;
    BString   v;

    reg = new BRegistry(BRegistry::currentUser, FLEXEMUREG);

    if (ifNotExists && reg->GetValue(FLEXVERSION, v) == ERROR_SUCCESS)
    {
        delete reg;
        return;
    }

    reg->SetValue(FLEXINVERSE, pGuiOptions->inverse ? 1 : 0);
    reg->SetValue(FLEXHIMEM, pOptions->isHiMem ? 1 : 0);
    reg->SetValue(FLEXUNDOCUMENTED, pOptions->use_undocumented ? 1 : 0);
    reg->SetValue(FLEXCOLOR, pGuiOptions->color);
    reg->SetValue(FLEXNCOLORS, pGuiOptions->nColors);
    reg->SetValue(FLEXDOCDIR, pGuiOptions->doc_dir);
    reg->SetValue(FLEXSCREENWIDTH, pGuiOptions->guiXSize);
    reg->SetValue(FLEXSCREENHEIGHT, pGuiOptions->guiYSize);
    reg->SetValue(FLEXMONITOR, pOptions->hex_file);
    reg->SetValue(FLEXDISKDIR, pOptions->disk_dir);
    reg->SetValue(FLEXDISK0, pOptions->drive[0]);
    reg->SetValue(FLEXDISK1, pOptions->drive[1]);
    reg->SetValue(FLEXDISK2, pOptions->drive[2]);
    reg->SetValue(FLEXDISK3, pOptions->drive[3]);
    reg->SetValue(FLEXVERSION, VERSION);
    delete reg;
#endif
#ifdef UNIX
    BRcFile *rcFile;
    BString rcFileName;
    BEnvironment env;

    if (!env.GetValue("HOME", rcFileName))
    {
        rcFileName = ".";
    }

    rcFileName += PATHSEPARATORSTRING FLEXEMURC;

    if (ifNotExists && access(rcFileName, F_OK) == 0)
    {
        return;
    }

    rcFile = new BRcFile(rcFileName);
    rcFile->Initialize(); // truncate file
    rcFile->SetValue(FLEXINVERSE, pGuiOptions->inverse ? 1 : 0);
    rcFile->SetValue(FLEXCOLOR, pGuiOptions->color);
    rcFile->SetValue(FLEXNCOLORS, pGuiOptions->nColors);
    rcFile->SetValue(FLEXDOCDIR, pGuiOptions->doc_dir);
    rcFile->SetValue(FLEXSCREENWIDTH, pGuiOptions->guiXSize);
    rcFile->SetValue(FLEXSCREENHEIGHT, pGuiOptions->guiYSize);
    rcFile->SetValue(FLEXMONITOR, pOptions->hex_file);
    rcFile->SetValue(FLEXHTMLVIEWER, pGuiOptions->html_viewer);
    rcFile->SetValue(FLEXDISKDIR, pOptions->disk_dir);
    rcFile->SetValue(FLEXDISK0, pOptions->drive[0]);
    rcFile->SetValue(FLEXDISK1, pOptions->drive[1]);
    rcFile->SetValue(FLEXDISK2, pOptions->drive[2]);
    rcFile->SetValue(FLEXDISK3, pOptions->drive[3]);
    rcFile->SetValue(FLEXHIMEM, pOptions->isHiMem ? 1 : 0);
    rcFile->SetValue(FLEXUNDOCUMENTED, pOptions->use_undocumented ? 1 : 0);
    delete rcFile;
#endif
} /* WriteOptions */

void FlexOptionManager::GetOptions(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions)
{
    int val;
#ifdef WIN32
    BRegistry *reg;

    reg = new BRegistry(BRegistry::localMachine, FLEXEMUREG);
    reg->GetValue(FLEXDISKDIR, pOptions->disk_dir);
    reg->GetValue(FLEXDISK0, pOptions->drive[0]);
    reg->GetValue(FLEXDISK1, pOptions->drive[1]);
    reg->GetValue(FLEXDISK2, pOptions->drive[2]);
    reg->GetValue(FLEXDISK3, pOptions->drive[3]);
    reg->GetValue(FLEXMONITOR, pOptions->hex_file);
    reg->GetValue(FLEXCOLOR, pGuiOptions->color);

    if (!reg->GetValue(FLEXDOCDIR, pGuiOptions->doc_dir))
        if (!reg->GetValue(FLEXNCOLORS, &val))
        {
            if (val == 2 || val == 8 || val == 64)
            {
                pGuiOptions->nColors = val;
            }
        }

    if (!reg->GetValue(FLEXSCREENWIDTH, &val))
    {
        if (val < 1)
        {
            val = 1;
        }

        if (val > MAX_GUIXSIZE)
        {
            val = MAX_GUIXSIZE;
        }

        pGuiOptions->guiXSize = val;
    }

    if (!reg->GetValue(FLEXSCREENHEIGHT, &val))
    {
        if (val < 1)
        {
            val = 1;
        }

        if (val > MAX_GUIYSIZE)
        {
            val = MAX_GUIYSIZE;
        }

        pGuiOptions->guiYSize = val;
    }

    if (!reg->GetValue(FLEXINVERSE, &val))
    {
        pGuiOptions->inverse = val;
    }

    if (!reg->GetValue(FLEXHIMEM, &val))
    {
        pOptions->isHiMem = (val != 0);
    }

    if (!reg->GetValue(FLEXUNDOCUMENTED, &val))
    {
        pOptions->use_undocumented = (val != 0);
    }

    delete reg;
#endif
#ifdef UNIX
    BString rcFileName;
    BEnvironment env;
    BRcFile *rcFile;

    if (!env.GetValue("HOME", rcFileName))
    {
        rcFileName = ".";
    }

    rcFileName += PATHSEPARATORSTRING FLEXEMURC;
    rcFile = new BRcFile(rcFileName);
    rcFile->GetValue(FLEXDISKDIR, pOptions->disk_dir);
    rcFile->GetValue(FLEXDISK0, pOptions->drive[0]);
    rcFile->GetValue(FLEXDISK1, pOptions->drive[1]);
    rcFile->GetValue(FLEXDISK2, pOptions->drive[2]);
    rcFile->GetValue(FLEXDISK3, pOptions->drive[3]);
    rcFile->GetValue(FLEXHTMLVIEWER, pGuiOptions->html_viewer);
    rcFile->GetValue(FLEXMONITOR, pOptions->hex_file);
    rcFile->GetValue(FLEXCOLOR, pGuiOptions->color);
    rcFile->GetValue(FLEXDOCDIR, pGuiOptions->doc_dir);

    if (!rcFile->GetValue(FLEXNCOLORS, &val))
    {
        if (val == 2 || val == 8 || val == 64)
        {
            pGuiOptions->nColors = val;
        }
    }

    if (!rcFile->GetValue(FLEXSCREENWIDTH, &val))
    {
        if (val < 1)
        {
            val = 1;
        }

        if (val > MAX_GUIXSIZE)
        {
            val = MAX_GUIXSIZE;
        }

        pGuiOptions->guiXSize = val;
    }

    if (!rcFile->GetValue(FLEXSCREENHEIGHT, &val))
    {
        if (val < 1)
        {
            val = 1;
        }

        if (val > MAX_GUIYSIZE)
        {
            val = MAX_GUIYSIZE;
        }

        pGuiOptions->guiYSize = val;
    }

    if (!rcFile->GetValue(FLEXINVERSE, &val))
    {
        pGuiOptions->inverse = val;
    }

    if (!rcFile->GetValue(FLEXHIMEM, &val))
    {
        pOptions->isHiMem = (val != 0);
    }

    if (!rcFile->GetValue(FLEXUNDOCUMENTED, &val))
    {
        pOptions->use_undocumented = (val != 0);
    }

    delete rcFile;
#endif
} // GetOptions

