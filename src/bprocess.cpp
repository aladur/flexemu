/*
    bprocess.cpp


    flexemu, a MC6809 emulator running FLEX
    Copyright (C) 2004-2018  W. Schwotzer

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
#ifdef UNIX
    #include <signal.h>
    #include <sys/types.h>
    #include <sys/wait.h>
#endif
#include "bprocess.h"
#include "cvtwchar.h"

BProcess::BProcess(const std::string &x_exec,
                   const std::string &x_dir /* = "" */,
                   const std::string &x_arg /* = "" */) :
    executable(x_exec), arguments(x_arg), directory(x_dir)
#ifdef WIN32
    , hProcess(NULL)
#endif
#ifdef UNIX
    , pid(-1)
#endif
{
}

void BProcess::AddArgument(const std::string &x_arg)
{
    if (!arguments.empty())
    {
        arguments += " ";
    }

    arguments += x_arg;
}

void BProcess::SetDirectory(const std::string &x_dir)
{
    directory = x_dir;
}

//***********************************************
// Win32 specific implementation
//***********************************************
#ifdef WIN32
BProcess::~BProcess()
{
    CloseHandle(hProcess);
    hProcess = NULL;
}

bool BProcess::Start()
{
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;
    std::string             commandline;
    DWORD                   result;

    commandline = executable + " " + arguments;
    memset((void *)&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
#ifdef UNICODE
    const wchar_t *pDirectory = NULL;
    wchar_t *pCommandLine = NULL;
    std::wstring directoryw;

    if (!directory.empty())
    {
        directoryw = ConvertToUtf16String(directory).c_str();
        pDirectory = directoryw.c_str();
    }

    // For CreateProcessW parameter lpCommandLine must not be const
    std::wstring commandlinew = ConvertToUtf16String(commandline);
    pCommandLine = new wchar_t[commandlinew.size()+1];
    wcscpy(pCommandLine, commandlinew.c_str());
    result = CreateProcess(NULL, pCommandLine,
        NULL, NULL, 0, 0, NULL, pDirectory, &si, &pi);
    delete pCommandLine;
#else
    const char *pDirectory = NULL;

    if (!directory.empty())
    {
        pDirectory = directory.c_str();
    }

    result = CreateProcess(NULL,
        const_cast<char *>(commandline.c_str()),
        NULL, NULL, 0, 0, NULL, pDirectory, &si, &pi);
#endif

    if (result != 0)
    {
        hProcess = pi.hProcess;
        CloseHandle(pi.hThread);
    }

    return result != 0;
}

bool BProcess::IsRunning()
{
    if (hProcess == NULL)
    {
        return false;
    }

    DWORD status;

    if (GetExitCodeProcess(hProcess, &status) == 0)
    {
        return false;
    }

    return (status == STILL_ACTIVE);
}
#endif

//***********************************************
// UNIX specific implementation
//***********************************************
#ifdef UNIX
BProcess::~BProcess()
{
    // nothing to clean up
}

// ATTENTION: multiple arguments are not supported yet!
bool BProcess::Start()
{
    const char *args[3];
    struct sigaction default_action;

    args[0] = executable.c_str();
    args[1] = arguments.c_str();
    args[2] = NULL;
    default_action.sa_handler = SIG_DFL;
    default_action.sa_flags   = 0;
    sigemptyset(&default_action.sa_mask);

    if ((pid = fork()) == 0)
    {
        sigaction(SIGPIPE, &default_action, NULL);

        if (!directory.empty())
        {
            if (chdir(directory.c_str()) < 0)
                {
                    // What to do here?
                }
        }

        execvp(args[0], (char *const *)args);
        // if we come here an error has occured
        _exit(1);
    }

    return IsRunning();
}

bool BProcess::IsRunning()
{
    if (pid == -1)
    {
        return false;
    }

    int status = 0;

    if (waitpid(pid, &status, WNOHANG) == 0)
    {
        return true;
    }

    return false;
}
#endif

