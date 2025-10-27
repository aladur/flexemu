/*
    bprocess.h


    flexemu, an MC6809 emulator running FLEX
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

#ifndef BPROCESS_INCLUDED
#define BPROCESS_INCLUDED

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;


// This class describes a platform independant Process interface

class BProcess
{
public:
    explicit BProcess(
             fs::path p_executable,
             fs::path p_directory = "",
             std::vector<std::string> p_arguments = {});
#ifdef _WIN32
    ~BProcess();
#endif
#ifdef UNIX
    ~BProcess() = default;
#endif
    void AddArgument(const std::string &argument);
    void SetDirectory(const fs::path &p_directory);
    bool Start(); // Start the Process if not started yet
    bool IsRunning() const; // Check if Process is running
    int Wait(); // Wait until process exited and return exit status
    std::vector<std::string> GetArguments()  const
    {
        return arguments;
    };
    fs::path GetDirectory()  const
    {
        return directory;
    };
    fs::path GetExecutable() const
    {
        return executable;
    };

protected:
    fs::path executable;
    fs::path directory;
    std::vector<std::string> arguments;

#ifdef _WIN32
    HANDLE hProcess{INVALID_HANDLE_VALUE};
#endif
#ifdef UNIX
    pid_t pid{-1};
#endif
};

#endif

