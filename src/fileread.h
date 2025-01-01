/*
    fileread.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2025  W. Schwotzer

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

#ifndef FILEREAD_INCLUDED
#define FILEREAD_INCLUDED


#include <limits>
#include <ostream>
#include <string>
#include "memsrc.h"
#include "memtgt.h"

extern int load_hexfile(const std::string &filename,
                        MemoryTarget<DWord> &memtgt,
                        DWord &startAddress);
extern int load_flex_binary(const std::string &filename,
                            MemoryTarget<DWord> &memtgt,
                            DWord &startAddress);
extern int write_flex_binary(const std::string &filename,
                             const MemorySource<DWord> &memsrc,
                             DWord startAddress =
                                 std::numeric_limits<DWord>::max());
extern int write_intel_hex(const std::string &filename,
                           const MemorySource<DWord> &memsrc,
                           DWord startAddress =
                              std::numeric_limits<DWord>::max());
extern int write_motorola_srecord(const std::string &filename,
                                  const MemorySource<DWord> &memsrc,
                                  DWord startAddress =
                                   std::numeric_limits<DWord>::max());
extern int write_raw_binary(const std::string &filename,
                            const MemorySource<DWord> &memsrc,
                            DWord startAddress =
                                std::numeric_limits<DWord>::max());
extern void print_hexfile_error(std::ostream &ostream, int error_id);

#endif // FILREAD_INCLUDED

