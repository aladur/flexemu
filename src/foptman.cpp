/*
    foptman.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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
#include <signal.h>
#include <stdexcept>
#include <string>
#include <new>

#include "e2.h"
#include "flexemu.h"
#include "sguiopts.h"
#include "foptman.h"
#include "bregistr.h"
#include "brcfile.h"
#include "benv.h"

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
    fprintf(fp, "  -F <frequency> (set CPU frequency in MHz)\n");
    fprintf(fp, "     0.0 runs CPU with maximum frequency\n");
    fprintf(fp, "  -C <startup command>\n");
#ifdef HAVE_TERMIOS_H
    fprintf(fp, "  -t (terminal only mode)\n");
    fprintf(fp, "  -r <two-hex-digit reset key>\n");
#endif
#ifdef HAVE_X11
    fprintf(fp, "  -s (run X11 synchronized)\n");
    fprintf(fp, "  -c <color> define foreground color\n");
    fprintf(fp, "  -i (display inverse video)\n");
    fprintf(fp, "  -n <# of colors>\n");
#ifdef _WIN32
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
    pOptions->mdcrDrives[0]    = "system.mdcr";
    pOptions->mdcrDrives[1]    = "";
    pOptions->hex_file         = "neumon54.hex";
    pOptions->startup_command  = "";
    pOptions->term_mode        = false;
    pOptions->isRamExtension   = true;
    pOptions->isHiMem          = false;
    pOptions->isFlexibleMmu    = false;
    pOptions->isEurocom2V5     = false;
    pOptions->use_undocumented = false;
    pOptions->useRtc           = true;
#ifndef _WIN32
#ifndef HAVE_X11
    pOptions->term_mode        = true;
#endif
#endif
    pOptions->reset_key        = 0x1e; // is Ctrl-^ for reset or Sig. INT
    pOptions->frequency        = -1.0; // default: ignore

    pGuiOptions->argc          = argc;
    pGuiOptions->argv          = argv;
    pGuiOptions->color         = "green";
    pGuiOptions->nColors       = 2;
    pGuiOptions->isInverse     = false;
#ifdef UNIX
    pGuiOptions->doc_dir       = F_DATADIR;
    pGuiOptions->guiType       = GuiType::XTOOLKIT;
    pOptions->disk_dir         = F_DATADIR;
#endif
#ifdef _WIN32
    pGuiOptions->doc_dir       = getExecutablePath() + PATHSEPARATORSTRING + "Documentation";
    pGuiOptions->guiType       = GuiType::WINDOWS;
    pOptions->disk_dir         = getExecutablePath() + PATHSEPARATORSTRING + "Data";
#endif
    pGuiOptions->pixelSizeX      = 2;
    pGuiOptions->pixelSizeY      = 2;
    pGuiOptions->isSynchronized  = false;
} // InitOptions

#ifdef UNIX
void FlexOptionManager::GetEnvironmentOptions(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions)
{
    std::string str;
    int value;
    BEnvironment env;

    // first look for environment variables
    if (env.GetValue((const char *)"FLEX" FLEXINVERSE, &value))
    {
        pGuiOptions->isInverse = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXRAMEXTENSION, &value))
    {
        pOptions->isRamExtension = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXHIMEM, &value))
    {
        pOptions->isHiMem = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXFLEXIBLEMMU, &value))
    {
        pOptions->isFlexibleMmu = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXEUROCOM2V5, &value))
    {
        pOptions->isEurocom2V5 = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXUNDOCUMENTED, &value))
    {
        pOptions->use_undocumented = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXRTC, &value))
    {
        pOptions->useRtc = (value != 0);
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

    if (env.GetValue((const char *)"FLEX" FLEXMDCRDRIVE0, str))
    {
        pOptions->mdcrDrives[0] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXMDCRDRIVE1, str))
    {
        pOptions->mdcrDrives[1] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXFREQUENCY, str))
    {
        try
        {
            pOptions->frequency = (stof(str));
        }
        catch(std::exception &)
        {
            // Intentionally ignore value if not convertible to float.
        }
    }
}
#else
void FlexOptionManager::GetEnvironmentOptions(
    struct sGuiOptions * /* [[maybe_unused]] struct sGuiOptions pGuiOptions */,
    struct sOptions * /* [[maybe_unused]] struct sOptions pOptions */)
{
}
#endif  // ifdef UNIX

void FlexOptionManager::GetCommandlineOptions(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions,
    int argc,
    char *const argv[])
{
    char    optstr[64];
    int     i;
    float   f;
    optind = 1;
    opterr = 0;
    strcpy((char *)optstr, "mup:f:0:1:2:3:j:k:F:C:");
#ifdef HAVE_TERMIOS_H
    strcat((char *)optstr, "tr:");  // terminal mode and reset key
#endif
#ifdef HAVE_X11
    strcat(optstr, "s");            // X11 synchronised
#ifdef _WIN32
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

                if (i > 0 && i <= MAX_PIXELSIZEX)
                {
                    pGuiOptions->pixelSizeX = i;
                }

                break;

            case 'k':
                sscanf(optarg, "%d", &i);

                if (i > 0 && i <= MAX_PIXELSIZEY)
                {
                    pGuiOptions->pixelSizeY = i;
                }

                break;

            case 'F':
                sscanf(optarg, "%f", &f);

                if (f >= 0.0)
                {
                    pOptions->frequency = f;
                }

                break;

            case 'C':
                pOptions->startup_command = optarg;
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
                pGuiOptions->isSynchronized     = true;
                break;
#ifdef _WIN32

            case 'g':
                if (stricmp(optarg, "win32") == 0)
                {
                    pGuiOptions->guiType = GuiType::WINDOWS;
                }

                if (stricmp(optarg, "x11") == 0)
                {
                    pGuiOptions->guiType = GuiType::XTOOLKIT;
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
                pGuiOptions->isInverse        = true;
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
#ifdef _WIN32
    std::string   v;

    BRegistry reg(BRegistry::currentUser, FLEXEMUREG);

    if (ifNotExists && reg.GetValue(FLEXVERSION, v) == ERROR_SUCCESS)
    {
        return;
    }

    reg.SetValue(FLEXINVERSE, pGuiOptions->isInverse ? 1 : 0);
    reg.SetValue(FLEXRAMEXTENSION, pOptions->isRamExtension ? 1 : 0);
    reg.SetValue(FLEXHIMEM, pOptions->isHiMem ? 1 : 0);
    reg.SetValue(FLEXFLEXIBLEMMU, pOptions->isFlexibleMmu ? 1 : 0);
    reg.SetValue(FLEXEUROCOM2V5, pOptions->isEurocom2V5 ? 1 : 0);
    reg.SetValue(FLEXUNDOCUMENTED, pOptions->use_undocumented ? 1 : 0);
    reg.SetValue(FLEXRTC, pOptions->useRtc ? 1 : 0);
    reg.SetValue(FLEXCOLOR, pGuiOptions->color.c_str());
    reg.SetValue(FLEXNCOLORS, pGuiOptions->nColors);
    reg.SetValue(FLEXDOCDIR, pGuiOptions->doc_dir.c_str());
    reg.SetValue(FLEXSCREENWIDTH, pGuiOptions->pixelSizeX);
    reg.SetValue(FLEXSCREENHEIGHT, pGuiOptions->pixelSizeY);
    reg.SetValue(FLEXMONITOR, pOptions->hex_file.c_str());
    reg.SetValue(FLEXDISKDIR, pOptions->disk_dir.c_str());
    reg.SetValue(FLEXDISK0, pOptions->drive[0].c_str());
    reg.SetValue(FLEXDISK1, pOptions->drive[1].c_str());
    reg.SetValue(FLEXDISK2, pOptions->drive[2].c_str());
    reg.SetValue(FLEXDISK3, pOptions->drive[3].c_str());
    reg.SetValue(FLEXMDCRDRIVE0, pOptions->mdcrDrives[0].c_str());
    reg.SetValue(FLEXMDCRDRIVE1, pOptions->mdcrDrives[1].c_str());
    reg.SetValue(FLEXFREQUENCY, std::to_string(pOptions->frequency));
    reg.SetValue(FLEXVERSION, VERSION);
#endif
#ifdef UNIX
    std::string rcFileName;
    BEnvironment env;

    if (!env.GetValue("HOME", rcFileName))
    {
        rcFileName = ".";
    }

    rcFileName += PATHSEPARATORSTRING FLEXEMURC;

    if (ifNotExists && access(rcFileName.c_str(), F_OK) == 0)
    {
        return;
    }

    BRcFile rcFile(rcFileName.c_str());
    rcFile.Initialize(); // truncate file
    rcFile.SetValue(FLEXINVERSE, pGuiOptions->isInverse ? 1 : 0);
    rcFile.SetValue(FLEXCOLOR, pGuiOptions->color.c_str());
    rcFile.SetValue(FLEXNCOLORS, pGuiOptions->nColors);
    rcFile.SetValue(FLEXDOCDIR, pGuiOptions->doc_dir.c_str());
    rcFile.SetValue(FLEXSCREENWIDTH, pGuiOptions->pixelSizeX);
    rcFile.SetValue(FLEXSCREENHEIGHT, pGuiOptions->pixelSizeY);
    rcFile.SetValue(FLEXMONITOR, pOptions->hex_file.c_str());
    rcFile.SetValue(FLEXDISKDIR, pOptions->disk_dir.c_str());
    rcFile.SetValue(FLEXDISK0, pOptions->drive[0].c_str());
    rcFile.SetValue(FLEXDISK1, pOptions->drive[1].c_str());
    rcFile.SetValue(FLEXDISK2, pOptions->drive[2].c_str());
    rcFile.SetValue(FLEXDISK3, pOptions->drive[3].c_str());
    rcFile.SetValue(FLEXMDCRDRIVE0, pOptions->mdcrDrives[0].c_str());
    rcFile.SetValue(FLEXMDCRDRIVE1, pOptions->mdcrDrives[1].c_str());
    rcFile.SetValue(FLEXRAMEXTENSION, pOptions->isRamExtension ? 1 : 0);
    rcFile.SetValue(FLEXHIMEM, pOptions->isHiMem ? 1 : 0);
    rcFile.SetValue(FLEXFLEXIBLEMMU, pOptions->isFlexibleMmu ? 1 : 0);
    rcFile.SetValue(FLEXEUROCOM2V5, pOptions->isEurocom2V5 ? 1 : 0);
    rcFile.SetValue(FLEXUNDOCUMENTED, pOptions->use_undocumented ? 1 : 0);
    rcFile.SetValue(FLEXRTC, pOptions->useRtc ? 1 : 0);
    rcFile.SetValue(FLEXFREQUENCY, std::to_string(pOptions->frequency).c_str());
#endif
} /* WriteOptions */

void FlexOptionManager::GetOptions(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions)
{
    int val;
    std::string string_value;

#ifdef _WIN32
    BRegistry reg(BRegistry::currentUser, FLEXEMUREG);

    reg.GetValue(FLEXDISKDIR, pOptions->disk_dir);
    reg.GetValue(FLEXDISK0, pOptions->drive[0]);
    reg.GetValue(FLEXDISK1, pOptions->drive[1]);
    reg.GetValue(FLEXDISK2, pOptions->drive[2]);
    reg.GetValue(FLEXDISK3, pOptions->drive[3]);
    reg.GetValue(FLEXMDCRDRIVE0, pOptions->mdcrDrives[0]);
    reg.GetValue(FLEXMDCRDRIVE1, pOptions->mdcrDrives[1]);
    reg.GetValue(FLEXMONITOR, pOptions->hex_file);
    reg.GetValue(FLEXCOLOR, pGuiOptions->color);

    if (!reg.GetValue(FLEXDOCDIR, pGuiOptions->doc_dir))
        if (!reg.GetValue(FLEXNCOLORS, &val))
        {
            if (val == 2 || val == 8 || val == 64)
            {
                pGuiOptions->nColors = val;
            }
        }

    if (!reg.GetValue(FLEXSCREENWIDTH, &val))
    {
        if (val < 1)
        {
            val = 1;
        }

        if (val > MAX_PIXELSIZEX)
        {
            val = MAX_PIXELSIZEX;
        }

        pGuiOptions->pixelSizeX = val;
    }

    if (!reg.GetValue(FLEXSCREENHEIGHT, &val))
    {
        if (val < 1)
        {
            val = 1;
        }

        if (val > MAX_PIXELSIZEY)
        {
            val = MAX_PIXELSIZEY;
        }

        pGuiOptions->pixelSizeY = val;
    }

    if (!reg.GetValue(FLEXINVERSE, &val))
    {
        pGuiOptions->isInverse = (val != 0);
    }

    if (!reg.GetValue(FLEXRAMEXTENSION, &val))
    {
        pOptions->isRamExtension = (val != 0);
    }

    if (!reg.GetValue(FLEXHIMEM, &val))
    {
        pOptions->isHiMem = (val != 0);
    }

    if (!reg.GetValue(FLEXFLEXIBLEMMU, &val))
    {
        pOptions->isFlexibleMmu = (val != 0);
    }

    if (!reg.GetValue(FLEXEUROCOM2V5, &val))
    {
        pOptions->isEurocom2V5 = (val != 0);
    }

    if (!reg.GetValue(FLEXUNDOCUMENTED, &val))
    {
        pOptions->use_undocumented = (val != 0);
    }

    if (!reg.GetValue(FLEXRTC, &val))
    {
        pOptions->useRtc = (val != 0);
    }

    if (!reg.GetValue(FLEXRTC, &val))
    {
        pOptions->useRtc = (val != 0);
    }

    if (!reg.GetValue(FLEXFREQUENCY, &string_value))
    {
        try
        {
            pOptions->frequency = (stof(string_value));
        }
        catch(std::exception &)
        {
            // Intentionally ignore value if not convertible to float.
        }
    }
#endif
#ifdef UNIX
    std::string rcFileName;
    BEnvironment env;

    if (!env.GetValue("HOME", rcFileName))
    {
        rcFileName = ".";
    }

    rcFileName += PATHSEPARATORSTRING FLEXEMURC;
    BRcFile rcFile(rcFileName.c_str());
    rcFile.GetValue(FLEXDISKDIR, pOptions->disk_dir);
    rcFile.GetValue(FLEXDISK0, pOptions->drive[0]);
    rcFile.GetValue(FLEXDISK1, pOptions->drive[1]);
    rcFile.GetValue(FLEXDISK2, pOptions->drive[2]);
    rcFile.GetValue(FLEXDISK3, pOptions->drive[3]);
    rcFile.GetValue(FLEXMDCRDRIVE0, pOptions->mdcrDrives[0]);
    rcFile.GetValue(FLEXMDCRDRIVE1, pOptions->mdcrDrives[1]);
    rcFile.GetValue(FLEXMONITOR, pOptions->hex_file);
    rcFile.GetValue(FLEXCOLOR, pGuiOptions->color);
    rcFile.GetValue(FLEXDOCDIR, pGuiOptions->doc_dir);

    if (!rcFile.GetValue(FLEXNCOLORS, &val))
    {
        if (val == 2 || val == 8 || val == 64)
        {
            pGuiOptions->nColors = val;
        }
    }

    if (!rcFile.GetValue(FLEXSCREENWIDTH, &val))
    {
        if (val < 1)
        {
            val = 1;
        }

        if (val > MAX_PIXELSIZEX)
        {
            val = MAX_PIXELSIZEX;
        }

        pGuiOptions->pixelSizeX = val;
    }

    if (!rcFile.GetValue(FLEXSCREENHEIGHT, &val))
    {
        if (val < 1)
        {
            val = 1;
        }

        if (val > MAX_PIXELSIZEY)
        {
            val = MAX_PIXELSIZEY;
        }

        pGuiOptions->pixelSizeY = val;
    }

    if (!rcFile.GetValue(FLEXINVERSE, &val))
    {
        pGuiOptions->isInverse = (val != 0);
    }

    if (!rcFile.GetValue(FLEXRAMEXTENSION, &val))
    {
        pOptions->isRamExtension = (val != 0);
    }

    if (!rcFile.GetValue(FLEXHIMEM, &val))
    {
        pOptions->isHiMem = (val != 0);
    }

    if (!rcFile.GetValue(FLEXFLEXIBLEMMU, &val))
    {
        pOptions->isFlexibleMmu = (val != 0);
    }

    if (!rcFile.GetValue(FLEXEUROCOM2V5, &val))
    {
        pOptions->isEurocom2V5 = (val != 0);
    }

    if (!rcFile.GetValue(FLEXUNDOCUMENTED, &val))
    {
        pOptions->use_undocumented = (val != 0);
    }

    if (!rcFile.GetValue(FLEXRTC, &val))
    {
        pOptions->useRtc = (val != 0);
    }

    if (!rcFile.GetValue(FLEXFREQUENCY, string_value))
    {
        try
        {
            pOptions->frequency = (stof(string_value));
        }
        catch(std::exception &)
        {
            // Intentionally ignore value if not convertible to float.
        }
    }
#endif
} // GetOptions

