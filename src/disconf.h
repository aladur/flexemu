/*
    disconf.h

    disassembler configuration class
    Copyright (C) 2003-2004  W. Schwotzer

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

#ifndef __disconf_h__
#define __disconf_h__

//#include <stdio.h>
#include "bsortlst.h"
#include "baddrrng.h"
#include "buint.h"


class DisassemblerConfig {

private:
	BUIntSList entryPoints;
	BIdentifierSList labelList;
	BIdentifierSList equateList;
	BAddressRanges codeRanges;
	BAddressRanges dataRangesInt8;
	BAddressRanges dataRangesInt16;
	BAddressRanges dataRangesInt32;
	BAddressRanges dataRangesChar;

public:

	enum {
		DISCONF_CODE = 1,
		DISCONF_INT8 = 2,
		DISCONF_INT16 = 4,
		DISCONF_INT32 = 8,
		DISCONF_CHAR = 16
	};

	DisassemblerConfig();					// public constructor
	virtual ~DisassemblerConfig();			// public destructor

	void AddEntryPoint(DWord address);
	void RemoveEntryPoint(DWord address);
	void AddLabel(const BIdentifier &i);
	void RemoveLabel(const BIdentifier &i);
	void AddEquate(const BIdentifier &i);
	void RemoveEquate(const BIdentifier &i);
	void SetStartAddress(DWord address, const char *label);
	void Add(const BAddressRanges &r, int type = DISCONF_CODE);
	void Add(const BInterval &i, int type = DISCONF_INT8);
	void Remove(const BInterval &i);
	bool Contains(const BInterval &i,
		int type = DISCONF_CODE | DISCONF_CHAR | DISCONF_INT8 | DISCONF_INT16 | DISCONF_INT32) const;
	bool Contains(DWord address,
		int type = DISCONF_CODE | DISCONF_CHAR | DISCONF_INT8 | DISCONF_INT16 | DISCONF_INT32) const;
	bool ContainsLabel(const BIdentifier &i) const;
	bool ContainsEquate(const BIdentifier &i) const;
	bool GetFirstEntryPoint(DWord *result) const;
	bool GetNextEntryPoint(DWord *result) const;
	void Clear();
	void PrintOn(FILE *fp) const;
}; // class DisassemblerConfig

inline void DisassemblerConfig::AddEntryPoint(DWord address) { entryPoints.Add(BUInt(address)); };
inline void DisassemblerConfig::RemoveEntryPoint(DWord address) { entryPoints.Remove(BUInt(address)); };
inline void DisassemblerConfig::AddLabel(const BIdentifier &i) { labelList.Add(i); };
inline void DisassemblerConfig::RemoveLabel(const BIdentifier &i) { labelList.Remove(i); };
inline void DisassemblerConfig::AddEquate(const BIdentifier &i) { equateList.Add(i); };
inline void DisassemblerConfig::RemoveEquate(const BIdentifier &i) { equateList.Remove(i); };
inline bool DisassemblerConfig::ContainsLabel(const BIdentifier &i) const { return labelList.Contains(i); };
inline bool DisassemblerConfig::ContainsEquate(const BIdentifier &i) const { return equateList.Contains(i); };

#endif // #ifndef __disconf_h__
