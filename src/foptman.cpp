/*
    foptman.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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
#include "soptions.h"
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
    fprintf(fp, "  -c <color> define foreground color\n");
    fprintf(fp, "  -i (display inverse video)\n");
    fprintf(fp, "  -n <# of colors>\n");
    fprintf(fp, "  -h (display this)\n");
    fprintf(fp, "  -? (display this)\n");
    fprintf(fp, "  -v (print version number)\n");
} // PrintHelp

void FlexOptionManager::InitOptions(
    struct sGuiOptions &guiOptions,
    struct sOptions &options,
    int argc,
    char *const argv[])
{
    options.drive[0] = "system.dsk";
    options.drive[1] = "";
    options.drive[2] = "";
    options.drive[3] = "";
    options.mdcrDrives[0] = "system.mdcr";
    options.mdcrDrives[1] = "";
    options.hex_file = "neumon54.hex";
    options.startup_command = "";
    options.term_mode = false;
    options.isRamExtension = true;
    options.isHiMem = false;
    options.isFlexibleMmu = false;
    options.isEurocom2V5 = false;
    options.use_undocumented = false;
    options.useRtc = true;
    options.reset_key = 0x1e; // is Ctrl-^ for reset or Sig. INT
    options.frequency = -1.0; // default: ignore

    guiOptions.argc = argc;
    guiOptions.argv = argv;
    guiOptions.color = "green";
    guiOptions.nColors = 2;
    guiOptions.isInverse = false;
#ifdef UNIX
    guiOptions.doc_dir = F_DATADIR;
    options.disk_dir = F_DATADIR;
#endif
#ifdef _WIN32
    guiOptions.doc_dir = getExecutablePath() + PATHSEPARATORSTRING + "Documentation";
    options.disk_dir = getExecutablePath() + PATHSEPARATORSTRING + "Data";
#endif
    guiOptions.pixelSize = 2;
} // InitOptions

#ifdef UNIX
void FlexOptionManager::GetEnvironmentOptions(
    struct sGuiOptions &guiOptions,
    struct sOptions &options)
{
    std::string str;
    int value;
    BEnvironment env;

    // first look for environment variables
    if (env.GetValue((const char *)"FLEX" FLEXINVERSE, &value))
    {
        guiOptions.isInverse = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXRAMEXTENSION, &value))
    {
        options.isRamExtension = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXHIMEM, &value))
    {
        options.isHiMem = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXFLEXIBLEMMU, &value))
    {
        options.isFlexibleMmu = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXEUROCOM2V5, &value))
    {
        options.isEurocom2V5 = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXUNDOCUMENTED, &value))
    {
        options.use_undocumented = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXRTC, &value))
    {
        options.useRtc = (value != 0);
    }

    if (env.GetValue((const char *)"FLEX" FLEXMONITOR, str))
    {
        options.hex_file = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXCOLOR, str))
    {
        guiOptions.color = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXNCOLORS, &value))
    {
        guiOptions.nColors = value;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISKDIR, str))
    {
        options.disk_dir = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISK0, str))
    {
        options.drive[0] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISK1, str))
    {
        options.drive[1] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXDISK2, str))
    {
        options.drive[2] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXMDCRDRIVE0, str))
    {
        options.mdcrDrives[0] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXMDCRDRIVE1, str))
    {
        options.mdcrDrives[1] = str;
    }

    if (env.GetValue((const char *)"FLEX" FLEXFREQUENCY, str))
    {
        try
        {
            options.frequency = (stof(str));
        }
        catch(std::exception &)
        {
            // Intentionally ignore value if not convertible to float.
        }
    }
}
#else
void FlexOptionManager::GetEnvironmentOptions(
    struct sGuiOptions & /* [[maybe_unused]] struct sGuiOptions &guiOptions */,
    struct sOptions & /* [[maybe_unused]] struct sOptions &options */)
{
}
#endif  // ifdef UNIX

void FlexOptionManager::GetCommandlineOptions(
    struct sGuiOptions &guiOptions,
    struct sOptions &options,
    int argc,
    char *const argv[])
{
    char    optstr[64];
    int     i;
    float   f;
    optind = 1;
    opterr = 1;
    strcpy(optstr, "mup:f:0:1:2:3:j:F:C:");
#ifdef HAVE_TERMIOS_H
    strcat(optstr, "tr:");  // terminal mode and reset key
#endif
    strcat(optstr, "ic:n:");          // color, inverse video, # of colors
    strcat(optstr, "vh");   // version and help

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
                options.hex_file = optarg;
                break;

            case '0':
                options.drive[0] = optarg;
                break;

            case '1':
                options.drive[1] = optarg;
                break;

            case '2':
                options.drive[2] = optarg;
                break;

            case '3':
                options.drive[3] = optarg;
                break;

            case 'p':
                options.disk_dir = optarg;
                break;

            case 'm':
                options.isHiMem = true;
                break;

            case 'u':
                options.use_undocumented = true;
                break;

            case 'j':
                sscanf(optarg, "%d", &i);

                if (i > 0 && i <= MAX_PIXELSIZE)
                {
                    guiOptions.pixelSize = i;
                }

                break;

            case 'F':
                sscanf(optarg, "%f", &f);

                if (f >= 0.0)
                {
                    options.frequency = f;
                }

                break;

            case 'C':
                options.startup_command = optarg;
                break;
#ifdef HAVE_TERMIOS_H

            case 't':
                options.term_mode = true;
                break;

            case 'r':
                sscanf(optarg, "%hx", (Word *)&options.reset_key);
                break;
#endif
            case 'n':
                sscanf(optarg, "%d", &i);
                guiOptions.nColors = i;
                break;

            case 'c':
                guiOptions.color = optarg;
                break;

            case 'i':
                guiOptions.isInverse = true;
                break;

            case 'v':
                fprintf(stdout, PROGRAMNAME ": V %s\n", PROGRAM_VERSION);
                exit(EXIT_SUCCESS);

            case '?':
            case 'h':
                PrintHelp(stderr);
                exit(EXIT_SUCCESS);
        }  // switch
    } // while
} // GetCommandlineOptions


void FlexOptionManager::WriteOptions(
    struct sGuiOptions &guiOptions,
    struct sOptions &options,
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

    reg.SetValue(FLEXINVERSE, guiOptions.isInverse ? 1 : 0);
    reg.SetValue(FLEXRAMEXTENSION, options.isRamExtension ? 1 : 0);
    reg.SetValue(FLEXHIMEM, options.isHiMem ? 1 : 0);
    reg.SetValue(FLEXFLEXIBLEMMU, options.isFlexibleMmu ? 1 : 0);
    reg.SetValue(FLEXEUROCOM2V5, options.isEurocom2V5 ? 1 : 0);
    reg.SetValue(FLEXUNDOCUMENTED, options.use_undocumented ? 1 : 0);
    reg.SetValue(FLEXRTC, options.useRtc ? 1 : 0);
    reg.SetValue(FLEXCOLOR, guiOptions.color.c_str());
    reg.SetValue(FLEXNCOLORS, guiOptions.nColors);
    reg.SetValue(FLEXSCREENFACTOR, guiOptions.pixelSize);
    reg.SetValue(FLEXMONITOR, options.hex_file.c_str());
    reg.SetValue(FLEXDISKDIR, options.disk_dir.c_str());
    reg.SetValue(FLEXDISK0, options.drive[0].c_str());
    reg.SetValue(FLEXDISK1, options.drive[1].c_str());
    reg.SetValue(FLEXDISK2, options.drive[2].c_str());
    reg.SetValue(FLEXDISK3, options.drive[3].c_str());
    reg.SetValue(FLEXMDCRDRIVE0, options.mdcrDrives[0].c_str());
    reg.SetValue(FLEXMDCRDRIVE1, options.mdcrDrives[1].c_str());
    reg.SetValue(FLEXFREQUENCY, std::to_string(options.frequency));
    reg.SetValue(FLEXVERSION, VERSION);
    reg.DeleteValue(FLEXDOCDIR); // Deprecated option value
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
    rcFile.SetValue(FLEXINVERSE, guiOptions.isInverse ? 1 : 0);
    rcFile.SetValue(FLEXCOLOR, guiOptions.color.c_str());
    rcFile.SetValue(FLEXNCOLORS, guiOptions.nColors);
    rcFile.SetValue(FLEXSCREENFACTOR, guiOptions.pixelSize);
    rcFile.SetValue(FLEXMONITOR, options.hex_file.c_str());
    rcFile.SetValue(FLEXDISKDIR, options.disk_dir.c_str());
    rcFile.SetValue(FLEXDISK0, options.drive[0].c_str());
    rcFile.SetValue(FLEXDISK1, options.drive[1].c_str());
    rcFile.SetValue(FLEXDISK2, options.drive[2].c_str());
    rcFile.SetValue(FLEXDISK3, options.drive[3].c_str());
    rcFile.SetValue(FLEXMDCRDRIVE0, options.mdcrDrives[0].c_str());
    rcFile.SetValue(FLEXMDCRDRIVE1, options.mdcrDrives[1].c_str());
    rcFile.SetValue(FLEXRAMEXTENSION, options.isRamExtension ? 1 : 0);
    rcFile.SetValue(FLEXHIMEM, options.isHiMem ? 1 : 0);
    rcFile.SetValue(FLEXFLEXIBLEMMU, options.isFlexibleMmu ? 1 : 0);
    rcFile.SetValue(FLEXEUROCOM2V5, options.isEurocom2V5 ? 1 : 0);
    rcFile.SetValue(FLEXUNDOCUMENTED, options.use_undocumented ? 1 : 0);
    rcFile.SetValue(FLEXRTC, options.useRtc ? 1 : 0);
    rcFile.SetValue(FLEXFREQUENCY, std::to_string(options.frequency).c_str());
#endif
} /* WriteOptions */

void FlexOptionManager::GetOptions(
    struct sGuiOptions &guiOptions,
    struct sOptions &options)
{
    int int_result;
    std::string string_result;

#ifdef _WIN32
    BRegistry reg(BRegistry::currentUser, FLEXEMUREG);

    reg.GetValue(FLEXDISKDIR, options.disk_dir);
    reg.GetValue(FLEXDISK0, options.drive[0]);
    reg.GetValue(FLEXDISK1, options.drive[1]);
    reg.GetValue(FLEXDISK2, options.drive[2]);
    reg.GetValue(FLEXDISK3, options.drive[3]);
    reg.GetValue(FLEXMDCRDRIVE0, options.mdcrDrives[0]);
    reg.GetValue(FLEXMDCRDRIVE1, options.mdcrDrives[1]);
    reg.GetValue(FLEXMONITOR, options.hex_file);
    reg.GetValue(FLEXCOLOR, guiOptions.color);

    if (!reg.GetValue(FLEXNCOLORS, int_result))
    {
        if (int_result == 2 || int_result == 8 || int_result == 64)
        {
            guiOptions.nColors = int_result;
        }
    }

    if (!reg.GetValue(FLEXSCREENFACTOR, int_result))
    {
        if (int_result < 1)
        {
            int_result = 1;
        }

        if (int_result > MAX_PIXELSIZE)
        {
            int_result = MAX_PIXELSIZE;
        }

        guiOptions.pixelSize = int_result;
    }

    if (!reg.GetValue(FLEXINVERSE, int_result))
    {
        guiOptions.isInverse = (int_result != 0);
    }

    if (!reg.GetValue(FLEXRAMEXTENSION, int_result))
    {
        options.isRamExtension = (int_result != 0);
    }

    if (!reg.GetValue(FLEXHIMEM, int_result))
    {
        options.isHiMem = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFLEXIBLEMMU, int_result))
    {
        options.isFlexibleMmu = (int_result != 0);
    }

    if (!reg.GetValue(FLEXEUROCOM2V5, int_result))
    {
        options.isEurocom2V5 = (int_result != 0);
    }

    if (!reg.GetValue(FLEXUNDOCUMENTED, int_result))
    {
        options.use_undocumented = (int_result != 0);
    }

    if (!reg.GetValue(FLEXRTC, int_result))
    {
        options.useRtc = (int_result != 0);
    }

    if (!reg.GetValue(FLEXRTC, int_result))
    {
        options.useRtc = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFREQUENCY, string_result))
    {
        try
        {
            options.frequency = (stof(string_result));
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
    rcFile.GetValue(FLEXDISKDIR, options.disk_dir);
    rcFile.GetValue(FLEXDISK0, options.drive[0]);
    rcFile.GetValue(FLEXDISK1, options.drive[1]);
    rcFile.GetValue(FLEXDISK2, options.drive[2]);
    rcFile.GetValue(FLEXDISK3, options.drive[3]);
    rcFile.GetValue(FLEXMDCRDRIVE0, options.mdcrDrives[0]);
    rcFile.GetValue(FLEXMDCRDRIVE1, options.mdcrDrives[1]);
    rcFile.GetValue(FLEXMONITOR, options.hex_file);
    rcFile.GetValue(FLEXCOLOR, guiOptions.color);

    if (!rcFile.GetValue(FLEXNCOLORS, int_result))
    {
        if (int_result == 2 || int_result == 8 || int_result == 64)
        {
            guiOptions.nColors = int_result;
        }
    }

    if (!rcFile.GetValue(FLEXSCREENFACTOR, int_result))
    {
        if (int_result < 1)
        {
            int_result = 1;
        }

        if (int_result > MAX_PIXELSIZE)
        {
            int_result = MAX_PIXELSIZE;
        }

        guiOptions.pixelSize = int_result;
    }

    if (!rcFile.GetValue(FLEXINVERSE, int_result))
    {
        guiOptions.isInverse = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXRAMEXTENSION, int_result))
    {
        options.isRamExtension = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXHIMEM, int_result))
    {
        options.isHiMem = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFLEXIBLEMMU, int_result))
    {
        options.isFlexibleMmu = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXEUROCOM2V5, int_result))
    {
        options.isEurocom2V5 = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXUNDOCUMENTED, int_result))
    {
        options.use_undocumented = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXRTC, int_result))
    {
        options.useRtc = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFREQUENCY, string_result))
    {
        try
        {
            options.frequency = (stof(string_result));
        }
        catch(std::exception &)
        {
            // Intentionally ignore value if not convertible to float.
        }
    }
#endif
} // GetOptions

