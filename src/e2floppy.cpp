/*
	e2floppy.cpp

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

#include "e2floppy.h"
#include "bstring.h"
#include "ffilecnt.h"
#include "rfilecnt.h"
#include "ndircont.h"
#include "fcinfo.h"
#include "flexerr.h"
#include "bmutex.h"
#include "bdir.h"


E2floppy::E2floppy(Inout *x_io, Mc6809 *x_cpu) :
	selected(4), pfs(NULL),
	pStatusMutex(NULL), cpu(x_cpu), io(x_io)
{
	Word i;

	pStatusMutex = new BMutex();
	disk_dir = "";
	for (i = 0; i <= 4; i++)
	{
		track[i]      = 1; // position all drives to track != 0  !!!
		floppy[i]     = NULL;
		driveStatus[i] = DISK_STAT_EMPTY;
	}
} // E2floppy


E2floppy::~E2floppy(void)
{
	for (int i = 0; i < 4; i++) {
		pStatusMutex->lock();
		if (floppy[i] != NULL) {
			try {
				floppy[i]->Close();
				delete floppy[i];
				floppy[i] = NULL;
			} catch (...) {
				// ignore errors
			}
		}
		pStatusMutex->unlock();
	}
	delete pStatusMutex;
	pStatusMutex = NULL;
} // ~E2floppy

bool E2floppy::umount_drive(Word drive_nr)
{
	FileContainerIfSector *pfloppy;

	if (drive_nr > 3 || (pfloppy = floppy[drive_nr]) == NULL)
		return 0;
	pStatusMutex->lock();
	try {
		pfloppy->Close();
		delete pfloppy;
		floppy[drive_nr] = NULL;
		driveStatus[drive_nr] = DISK_STAT_EMPTY;
	} catch (FlexException UNUSED(&e)) {
		// ignore errors
	}
	pStatusMutex->unlock();
	return 1;
} // umount_drive

bool E2floppy::mount_drive(const char *path, Word drive_nr, tMountOption option)
{
	int i = 0;
	FileContainerIfSector	*pfloppy = NULL;

	if (drive_nr > 3 || path == NULL || strlen(path) == 0)
		return false;
	// check if already mounted
	if (floppy[drive_nr] != NULL)
		return false;
	track[drive_nr] = 1;	// position to a track != 0  !!!

	BString containerPath;

	// first try with given path
	containerPath = path;
#ifdef WIN32
	containerPath.replaceall('|', ':');
	containerPath.replaceall('/', '\\');
#endif

	for (i = 0; i < 2; i++)
	{
#ifdef NAFS
	   if (BDirectory::Exists(containerPath))
	   {
	      try {
		  pfloppy = new NafsDirectoryContainer(containerPath);
	      } catch (FlexException UNUSED(&e)) {
		  // just ignore
	      }
	   } else
#endif
	   if (option == MOUNT_RAM)
	   {
	      try {
		   pfloppy = new FlexRamFileContainer(containerPath, "rb+");
	      } catch (FlexException UNUSED(&e)) {
	   	   try {
		      pfloppy = new FlexRamFileContainer(containerPath, "rb");
	   	   } catch (FlexException UNUSED(&e)) {
		  	// just ignore
		   }
	      }
	   } else {
	      try {
		   pfloppy = new FlexFileContainer(containerPath, "rb+");
	      } catch (FlexException UNUSED(&e)) {
	   	   try {
		      pfloppy = new FlexFileContainer(containerPath, "rb");
	   	   } catch (FlexException UNUSED(&e)) {
		  	// just ignore
		   }
	      }
	   }

	   pStatusMutex->lock();
	   floppy[drive_nr] = pfloppy;
	   if (pfloppy != NULL)
	   {
			driveStatus[drive_nr] = DISK_STAT_ACTIVE;
			pStatusMutex->unlock();
			return true;
	   }
	   pStatusMutex->unlock();

	   // second try with path within disk_dir directory
	   containerPath = disk_dir;
	   if (containerPath.lastchar() != PATHSEPARATOR)
		containerPath += PATHSEPARATORSTRING;
	   containerPath += path;
	} // for
	return (pfloppy != NULL);
} // mount_drive

void E2floppy::disk_directory(const char *x_disk_dir)
{
	disk_dir = x_disk_dir;
}

void E2floppy::mount_all_drives(BString drive[])
{
	int i;

	for (i = 0; i < 4; i++)
		mount_drive(drive[i], i);
	selected = 4;			// deselect all drives
	pfs = NULL;
}  // mount_all_drives

bool E2floppy::umount_all_drives(void)
{
	Word i;
	bool result;

	result = true;
	for (i = 0; i < 4; i++)
		if (!umount_drive(i))
			result = false;
	return result;
}  // umount_all_drives

// get info for corresponding drive or NULL
// the info string should not exceed 512 Bytes
// it is dynamically allocated and should be freed
// by the calling program

BString E2floppy::drive_info(Word drive_nr)
{
	BString			str;
	FileContainerIfSector	*pfl;

	if (drive_nr <= 3) {
		pStatusMutex->lock();
		if ((pfl = floppy[drive_nr]) == NULL) {
			str.printf("drive #%d not ready\n", drive_nr);
		} else {
			FlexContainerInfo info;
			int trk, sec;

			try {
				pfl->GetInfo(info);
			} catch (FlexException &e) {
				str.printf("%s\n", e.what());
				pStatusMutex->unlock();
				return str;
			}
			info.GetTrackSector(&trk, &sec);
			str.printf(
"drive       #%d\n"
"type:       %s\n"
"name:       %s\n"
"path:       %s\n"
"tracks:     %d\n"
"sectors:    %d\n"
"write-prot: %s\n", drive_nr, info.GetTypeString().chars(), info.GetName(),
				info.GetPath().chars(), trk, sec,
				pfl->IsWriteProtected() ? "yes" : "no");

		}
		pStatusMutex->unlock();
	}
	return str;
} // drive_info

const char *E2floppy::open_mode(char *path)
{
	    int  wp;	// write protect flag
	const char *mode;
	
	wp = access(path, W_OK);
	mode = wp ? "rb" : "rb+";
	return mode;
} // open_mode


bool E2floppy::update_all_drives(void)
{
	FileContainerIfSector	*pfloppy;
	Word			i;
	bool			result;

	result = true;
	for (i = 0; i < 4; i++) {
		pfloppy = floppy[i];
		if (pfloppy == NULL)
			// no error if drive not ready
			continue;
		if (!update_drive(i))
			result = false;
	}
	return result;
} // update_all_drives

bool E2floppy::update_drive(Word drive_nr)
{
	FileContainerIfSector *pfloppy;

	if (drive_nr > 3)
		return false;
	pfloppy = floppy[drive_nr];
	if (pfloppy == NULL)
		// error if drive not ready
		return false;
	return 1;
} // update_drive

void E2floppy::resetIo()
{
	drisel = 0;
	Wd1793::resetIo();
}


Byte E2floppy::readIo(Word offset)
{
	if (offset <= 3)
		return Wd1793::readIo(offset);
	status = 0xff;	// unused is logical high
	if (!side)
		status &= 0xfd;
	if (!irq)
		status &= 0xbf;
	if (!drq)
		status &= 0x7f;
	return status;
} // readIo


void E2floppy::writeIo(Word offset, Byte val)
{
	if (offset <= 3)
		Wd1793::writeIo(offset, val);
	else {
		drisel = val;
		side = (drisel & 0x10) ? 1 : 0;
		track[selected] = tr;
		switch (drisel & 0x0f) {
			case 0x01 : selected = 0;
				    break;
			case 0x02 : selected = 1;
				    break;
			case 0x04 : selected = 2;
				    break;
			case 0x08 : selected = 3;
				    break;
			default   : selected = 4;
		};
		pfs = floppy[selected];
		tr = track[selected];
	}
} // writeIo


Byte E2floppy::readByte(Word index)
{
	if (pfs == NULL)
		return 0;

	pStatusMutex->lock();
	if (index == pfs->GetBytesPerSector()) {
		driveStatus[selected] = DISK_STAT_ACTIVE;
		if (!pfs->ReadSector((Byte *)&sector_buffer, tr, sr)) {
			drq = 0;
			str = 0x10;
			byteCount = 0;
		}
	} // if
	pStatusMutex->unlock();
	return sector_buffer[SECTOR_SIZE - index];
} // readByte


void E2floppy::writeByte(Word index)
{
	//unsigned int error;

	sector_buffer[SECTOR_SIZE - index] = dr;
	if (index == 1) {
		pStatusMutex->lock();
		driveStatus[selected] = DISK_STAT_ACTIVE;
		if (!pfs->WriteSector((Byte *)&sector_buffer, tr, sr)) {
			// how to react on write error???
		}
		pStatusMutex->unlock();
	} // if
} // writeByte


Byte E2floppy::recordNotFound(void)
{
	if (pfs == NULL)
		return 1;
	return !pfs->IsSectorValid(tr, sr);
} // recordNotFound

Byte E2floppy::seekError(Byte new_track)
{
	if (pfs == NULL)
		return 1;
	return !pfs->IsTrackValid(new_track);
} // seekError

Byte E2floppy::driveReady(void)
{
	return pfs != NULL;
}  // driveReady

Byte E2floppy::writeProtect(void)
{
	if (pfs == NULL)
		return 1;
	return pfs->IsWriteProtected();
}  // writeProtect

void E2floppy::get_drive_status(tDiskStatus stat[4])
{
	Word i;

	pStatusMutex->lock();
	for (i = 0; i < 4; ++i)
	{
		stat[i] = driveStatus[i];
		if (driveStatus[i] != DISK_STAT_EMPTY)
			driveStatus[i] = DISK_STAT_INACTIVE;
	}
	pStatusMutex->unlock();
}

bool E2floppy::format_disk(SWord trk, SWord sec, char *name,
						   int type /* = TYPE_DSK_CONTAINER */)
{
	FileContainerIfSector *pfloppy = NULL;

	try {
	   switch (type) {
#ifdef NAFS
		case TYPE_NAFS_DIRECTORY:
		     pfloppy = NafsDirectoryContainer::Create(disk_dir, name,
                                                              trk, sec, type);
                     break;
#endif
		case TYPE_DSK_CONTAINER: 
		case TYPE_FLX_CONTAINER:
		     pfloppy = FlexFileContainer::Create(disk_dir, name,
                                                              trk, sec, type);
                     break;
	   }
	} catch (FlexException UNUSED(&e)) {
		return false;
	}
	delete pfloppy;
	return false;
} // format_disk

