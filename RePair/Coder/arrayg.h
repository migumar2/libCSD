/*

Repair -- an implementation of Larsson and Moffat's compression and
decompression algorithms.
Copyright (C) 2010-current_year Gonzalo Navarro

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Author's contact: Gonzalo Navarro, Dept. of Computer Science, University of
Chile. Blanco Encalada 2120, Santiago, Chile. gnavarro@dcc.uchile.cl

*/

	// extendible circular array for pairs

#ifndef ARRAYINCLUDED
#define ARRAYINCLUDED

#include "basics.h"

typedef struct
   { int *pairs; // identifiers
     int maxsize;  
     int size;
     int fst; // first of circular array
     float factor;
     int minsize;
     void *Rec; // records
   } Tarray;

// contents can be accessed as A.pairs[0..A.size-1]
class ArrayG
{
public:
	// creates empty array
	static Tarray createArray (void *Rec, float factor, int minsize);
	// destroys A
	static void destroyArray (Tarray *A);

	// inserts pair in A, returns pos
	static int insertArray (Tarray *A, int pair);
	 // deletes last cell in A
	static void deleteArray (Tarray *A);	
};
  
#endif
