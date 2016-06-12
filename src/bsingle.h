/*
    bsingle.h

    Basic singleton template class definition
    Copyright (C) 1997-2005  W. Schwotzer

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

#ifndef _BSINGLE_H_
#define _BSINGLE_H_

#include "bdelete.h"

template <class T>
class BSingleton
{
   //friend T; // allow BSingleton to create an instance of T
private:
   static T *instance;

   BSingleton();
   BSingleton(const BSingleton &);
   BSingleton &operator=(const BSingleton &);
public:
   static T &Instance();
   virtual ~BSingleton();
};

template <class T>
T &BSingleton<T>::Instance()
{
   if (instance == NULL)
   {
      instance = new T;
      static BDeleter<T> deleter(instance);
   }
   return *instance;
}

template <class T>
BSingleton<T>::~BSingleton()
{
   delete instance;
   instance = NULL;
}
#endif

