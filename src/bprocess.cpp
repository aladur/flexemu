/*
    bprocess.cpp


    flexemu, a MC6809 emulator running FLEX
    Copyright (C) 2004-2025  W. Schwotzer

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

#if defined(UNIX) || defined(USE_CMAKE)
#include "config.h"
#else
#include "confignt.h"
#endif
#include "bprocess.h"
#ifdef _WIN32
#include "cvtwchar.h"
#endif
#ifdef UNIX
#include <sys/wait.h>
#endif
#include <utility>
#include <string>
#include <vector>
#include <system_error>
#include <filesystem>
#include <csignal>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace fs = std::filesystem;


BProcess::BProcess(fs::path p_executable,
                   fs::path p_directory /* = "" */,
                   std::vector<std::string> p_arguments /* = {} */)
    : executable(std::move(p_executable))
    , directory(std::move(p_directory))
    , arguments(std::move(p_arguments))
{
}

void BProcess::AddArgument(const std::string &argument)
{
    arguments.push_back(argument);
}

void BProcess::SetDirectory(const fs::path &p_directory)
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

    const auto wExecutable(executable.wstring());
    const auto wDirectory(directory.wstring());
    std::wstring wArguments;

    memset((void *)&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    for (const auto &argument : arguments)
    {
        if (!wArguments.empty())
        {
            wArguments += L" ";
        }
        wArguments += ConvertToUtf16String(argument);
    }

    // For CreateProcessW parameter wArguments must not be const
    auto result = CreateProcess(
        wExecutable.c_str(), // Name/path of executable
        wArguments.data(), // All command line arguments in one string
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

    DWORD exitCode;

    if (GetExitCodeProcess(hProcess, &exitCode) == 0)
    {
        return false;
    }

    return (exitCode == STILL_ACTIVE);
}

int BProcess::Wait()
{
    if (hProcess == nullptr)
    {
        return 0;
    }

    DWORD exitCode;

    WaitForSingleObject(hProcess, INFINITE);
    if (GetExitCodeProcess(hProcess, &exitCode) == 0)
    {
        hProcess = nullptr;
        return 0;
    }

    hProcess = nullptr;
    return static_cast<int>(exitCode);
}
#endif

//***********************************************
// UNIX specific implementation
//***********************************************
#ifdef UNIX

bool BProcess::Start()
{
    std::vector<char *> argv;
    auto sExecutable = executable.u8string();
    struct sigaction default_action{};

    argv.push_back(sExecutable.data());
    for (auto &argument : arguments)
    {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    default_action.sa_handler = SIG_DFL;
    default_action.sa_flags = 0;
    sigemptyset(&default_action.sa_mask);

    pid = fork();
    if (pid == 0)
    {
        sigaction(SIGPIPE, &default_action, nullptr);

        if (!directory.empty())
        {
            std::error_code ec;
            fs::current_path(directory, ec);
            if (ec)
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

int BProcess::Wait()
{
    if (pid == -1)
    {
        return 0;
    }

    int status = 0;

    if (waitpid(pid, &status, 0) == pid && WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }

    pid = -1;
    return 0;
}
#endif

