
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

	// extendible array for records

#ifndef RECORDSINCLUDED
#define RECORDSINCLUDED

#include "stdio.h"
#include "basics.h"

typedef struct
   { int prev,next;
   } Tlist; // list of prev next equal char

typedef struct
   { Tpair pair; // pair content
     int freq; // frequency
     int cpos; // 1st position in C
     int hpos; // position in heap
     int kpos; // position in hash
   } Trecord;

typedef struct
   { Trecord *records; 
     int maxsize;  
     int size;
     float factor;
     int minsize;
     void *Hash;  // Thash *
     void *Heap; // Theap *
     void *List; // Tlist *
   } Trarray;

#include "heap.h"
#include "hash.h"

// contents can be accessed as Rec.records[0..Rec.size-1]
class Records
{
public:
	// inserts pair in Rec, returns id, links to/from
	// Hash and Heap, not List. sets freq = 1
	static int insertRecord (Trarray *Rec, Tpair pair);
	// deletes last cell in Rec
	static void deleteRecord (Trarray *Rec);
	 // creates empty array
	static Trarray createRecords (float factor, int minsize);
	// associates structures
	static void assocRecords (Trarray *Rec, void *Hash, void *Heap, void *List); 
	// destroys Rec
	static void destroyRecords (Trarray *Rec);
	// delete record, freq <= 1
	// due to freq 0 or purgue (freq 1)
	// already deleted from heap
	static void removeRecord (Trarray *Rec, int id);
};
#endif
