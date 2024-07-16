/*
    bprocess.cpp


    flexemu, a MC6809 emulator running FLEX
    Copyright (C) 2004-2024  W. Schwotzer

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
    #include <csignal>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <vector>
    #include <array>
#endif
#include "bprocess.h"
#include "cvtwchar.h"

BProcess::BProcess(std::string p_executable,
                   std::string p_directory /* = "" */,
                   std::string p_arguments /* = "" */)
    : executable(std::move(p_executable))
    , arguments(std::move(p_arguments))
    , directory(std::move(p_directory))
{
}

void BProcess::AddArgument(const std::string &argument)
{
    if (!arguments.empty())
    {
        arguments += " ";
    }

    arguments += argument;
}

void BProcess::SetDirectory(const std::string &p_directory)
{
    directory = p_directory;
}

//***********************************************
// Win32 specific implementation
//***********************************************
#ifdef _WIN32
BProcess::~BProcess()
{
    CloseHandle(hProcess);
    hProcess = nullptr;
}

bool BProcess::Start()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    const auto wExecutable(ConvertToUtf16String(executable));
    auto wArguments(ConvertToUtf16String(arguments));
    const auto wDirectory(ConvertToUtf16String(directory));
    memset((void *)&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    // For CreateProcessW parameter wArguments must not be const
    auto result = CreateProcess(
        wExecutable.c_str(), // Name/path of executable
        &wArguments[0], // Command line arguments
        nullptr, // Security attributes, handle can not be inherited
        nullptr, // Thread attributes, handle can not be inherited
        FALSE, // Handles are not inherited
        0, // Process creation flags
        nullptr, // Environment block for new process
        wDirectory.c_str(), // Current directory for new process
        &si, // Startup info
        &pi); // Process info

    if (result != 0)
    {
        hProcess = pi.hProcess;
        CloseHandle(pi.hThread);
    }

    return result != 0;
}

bool BProcess::IsRunning() const
{
    if (hProcess == nullptr)
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

// ATTENTION: multiple arguments are not supported yet!
bool BProcess::Start()
{
    std::vector<std::string> args{ executable, arguments };
    std::array<char *, 3> argv{};
    struct sigaction default_action{};
    int i = 0;

    for (auto &arg : args)
    {
        argv[i++] = arg.data();
    }

    default_action.sa_handler = SIG_DFL;
    default_action.sa_flags = 0;
    sigemptyset(&default_action.sa_mask);

    pid = fork();
    if (pid == 0)
    {
        sigaction(SIGPIPE, &default_action, nullptr);

        if (!directory.empty())
        {
            if (chdir(directory.c_str()) < 0)
                {
                    // What to do here?
                }
        }

        execvp(argv[0], argv.data());
        // if we come here an error has occured
        _exit(1);
    }

    return IsRunning();
}

bool BProcess::IsRunning() const
{
    if (pid == -1)
    {
        return false;
    }

    int status = 0;

    return waitpid(pid, &status, WNOHANG) == 0;
}
#endif

