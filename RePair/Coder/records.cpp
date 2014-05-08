
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

	// extendible array for pairs

#include <stdlib.h>
#include "records.h"

int 
Records::insertRecord (Trarray *Rec, Tpair pair)
{
     int id;
     Trecord *rec;
     if (Rec->size == Rec->maxsize)
	{ if (Rec->maxsize == 0)
	     { Rec->maxsize = Rec->minsize;
	       Rec->records = (Trecord*)malloc (Rec->maxsize * sizeof(Trecord));
	     }
	  else
	     { Rec->maxsize /= Rec->factor;
	       Rec->records = (Trecord*)realloc (Rec->records, Rec->maxsize * sizeof(Trecord));
	     }
	}
     id = Rec->size++;
     rec = &Rec->records[id];
     rec->pair = pair;
     HashRP::insertHash ((Thash*)Rec->Hash,id);
     Heap::insertHeap ((Theap*)Rec->Heap,id);
     return id;
}

void 
Records::deleteRecord (Trarray *Rec)
{
     Rec->size--;
     if (Rec->size == 0)
        { Rec->maxsize = 0;
          free (Rec->records);
          Rec->records = NULL;
        }
     else if ((Rec->size < Rec->maxsize * Rec->factor * Rec->factor) && 
	      (Rec->size * Rec->factor >= Rec->minsize))
	{ Rec->maxsize *= Rec->factor;
	  Rec->records = (Trecord*)realloc (Rec->records, Rec->maxsize * sizeof(Trecord));
	}
}

Trarray 
Records::createRecords (float factor, int minsize)
{
     Trarray Rec;
     Rec.records = NULL;
     Rec.maxsize = 0;
     Rec.size = 0;
     Rec.factor = factor;
     Rec.minsize = minsize;
     Rec.Hash = NULL;
     Rec.Heap = NULL;
     Rec.List = NULL;
     return Rec;
}

void 
Records::assocRecords (Trarray *Rec, void *Hash, void *Heap, void *List)
{
     Rec->Hash = Hash;
     Rec->Heap = Heap;
     Rec->List = List;
}

void 
Records::destroyRecords (Trarray *Rec)
{
     if (Rec->maxsize == 0) return;
     free (Rec->records);
     Rec->records = NULL;
     Rec->maxsize = 0;
     Rec->size = 0;
     Rec->Hash = NULL;
     Rec->Heap = NULL;
     Rec->List = NULL;
}
     
void 
Records::removeRecord (Trarray *Rec, int id) 
{
     Tlist *L = (Tlist*)Rec->List;
     HashRP::deleteHash ((Thash*)Rec->Hash,id); // mark del in hash
     if ((Rec->records[id].cpos != -1) &&
	 (L[Rec->records[id].cpos].prev == -id-1))
	L[Rec->records[id].cpos].prev = NullFreq; // null ptr from L
     if (id != Rec->size-1)
        { Rec->records[id] = Rec->records[Rec->size-1];
          HashRP::hashRepos ((Thash*)Rec->Hash,id);
          Heap::heapRepos ((Theap*)Rec->Heap,id);
          L[Rec->records[id].cpos].prev = -id-1; 
	}
     deleteRecord (Rec);
}
