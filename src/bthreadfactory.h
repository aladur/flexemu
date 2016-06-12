/*
    bthreadfactory.h


    Basic class for thread creation
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

#ifndef BTHREADFACTORY_H
#define BTHREADFACTORY_H

#include "misc1.h" // needed for NULL
#include "bsingle.h"

// Thread factory creating platform dependant Thread classes

class BThreadImp;
class BThreadFactory;

typedef BSingleton<BThreadFactory> SThreadFactory;

class BThreadFactory {

friend BThreadFactory &SThreadFactory::Instance();

public:
   BThreadImp *CreateBThreadImp();
   virtual ~BThreadFactory() { };
private:
   BThreadFactory() { };

};

#endif

