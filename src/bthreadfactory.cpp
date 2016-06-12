/*
    bthreadfactory.cpp


    Basic class for platform independent thread creation
    Copyright (C) 2001-2005  W. Schwotzer

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
#include "bthreadfactory.h"
#ifdef UNIX
#include "bposixthreadimp.h"
#endif
#ifdef WIN32
#include "bwin32threadimp.h"
#endif

template <> BThreadFactory *SThreadFactory::instance = NULL;

BThreadImp *BThreadFactory::CreateBThreadImp()
{
#ifdef WIN32
  return new BWin32ThreadImp;
#else
  #ifdef UNIX
  return new BPosixThreadImp();
  #else
    return NULL; // Sw: Which other platforms to support ???
  #endif
#endif
}
