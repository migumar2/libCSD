
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

	// binary heap with sqrt(u) heaps for least occurring ones
	// and list of frequencies, each with a list of elems, for the others
	// which are no more than sqrt(u) overall. this guarantees O(1) 
	// operation times for the heap, as frequencies change by +-1, and
	// O(sqrt(u)) integers overhead over the bare arrays of ids (/factor).

#ifndef HEAPINCLUDED
#define HEAPINCLUDED

#include <stdlib.h>
#include "basics.h"
#include "arrayg.h"
#include "records.h"

static const int PRNH = 0;

typedef struct 
  { int freq;
    int elems; // a pointer within freq array
    int larger,smaller; // pointers within ff array
  } Thfreq;

typedef struct 
  { int id;
    int prev,next; // actually pointers within freq array
    int fnode; // ptr to its freq node (ptr to ff)
  } Thnode;

typedef struct
  { Thnode *freq; // space for all frequent nodes is preallocated, sqrt(u)
    int freef; // ptr to free list in freq
    Thfreq *ff; // space for all frequencies of frequent nodes prealloc idem
    int freeff; // ptr to free list in ff
    int smallest,largest; // list of frequent ones (ptrs in ff)
    Tarray *infreq; // vectors for infrequent ones
    int sqrtu;
    int max;  // max freq heap used
    Trarray *Rec; // records
  } Theap;

class Heap
{
public:
	// creates new empty heap
	// 0<factor<1: occupancy factor
	// sqrt(u)*max(minsize,n/factor) integers
	static Theap createHeap (int u, Trarray *Rec, float factor, int minsize);
	// destroys H
	static void destroyHeap (Theap *H);
	// inc freq of pair Rec[id]
	static void incFreq (Theap *H, int id);
	// dec freq of pair Rec[id]
	static void decFreq (Theap *H, int id);
	// with freq 1
	static void insertHeap (Theap *H, int id);

	static int extractMax (Theap *H);
	// remove elems with freq 1
	static void purgeHeap (Theap *H); 
	// repositions pair
	static void heapRepos (Theap *H, int id);

	static void move (Tarray A, int i, int j, Trecord *rec);
	static void prnH (Theap *H);
};
#endif
