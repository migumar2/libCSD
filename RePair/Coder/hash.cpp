
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

	// linear probing hash table for pairs

	// table only grows to keep factor, does not shrink
	// this is ok for this application where #pairs rarely decreases

	// value -1 denotes empty cells, -2 is a deletion mark

#include <stdlib.h>
#include "hash.h"

int 
HashRP::searchHash (Thash H, Tpair p)
{
    relong u = ((relong)p.left)<<(8*sizeof(int)) | (relong)p.right;
    int k = ((PRIME*u) >> (8*sizeof(int))) & H.maxpos;
    Trecord *recs = H.Rec->records;
    while (H.table[k] != -1) 
      {	if ((H.table[k] >= 0) && 
	    (recs[H.table[k]].pair.left == p.left) &&
	    (recs[H.table[k]].pair.right == p.right)) break;
	k = (k+1) & H.maxpos;
      }
    return H.table[k];
}

void 
HashRP::deleteHash (Thash *H, int id)
{
    Trecord *rec = H->Rec->records;
    H->table[rec[id].kpos] = -2;
    H->used--;
}

Thash 
HashRP::createHash (int maxpos, Trarray *Rec)
{
    Thash H;
    int i;
	// upgrade maxpos to the next value of the form (1<<smth)-1
    while (maxpos & (maxpos-1)) maxpos &= maxpos-1;
    maxpos = (maxpos-1)<<1 | 1;  // avoids overflow if maxpos = 1<<31
    H.maxpos = maxpos;
    H.used = 0;
    H.table = (int*)malloc((1+maxpos)*sizeof(int));
    for (i=0;i<=maxpos;i++) H.table[i] = -1;
    H.Rec = Rec;
    return H;
}
  
int 
HashRP::finsertHash (Thash H, Tpair p)
{
    relong u = ((relong)p.left)<<(8*sizeof(int)) | (relong)p.right;
    int k = ((PRIME*u) >> (8*sizeof(int))) & H.maxpos;
    while (H.table[k] >= 0) k = (k+1) & H.maxpos;
    return k;
}
  
void 
HashRP::insertHash (Thash *H, int id)
{
    int k;
    Trecord *rec = H->Rec->records;
    if (H->used > H->maxpos * factor) // resize
	{ Thash newH = createHash((H->maxpos<<1)|1,H->Rec);
	  int i;
	  int *tab = H->table;
	  for (i=0;i<=H->maxpos;i++)
	      if (tab[i] >= 0) // also removes marked deletions
		 { k = finsertHash (newH,rec[tab[i]].pair);
		   newH.table[k] = tab[i];
		   rec[tab[i]].kpos = k;
		 }
	  newH.used = H->used;
	  free (H->table);
	  *H = newH;
	}
    H->used++;
    k = finsertHash (*H,rec[id].pair);
    H->table[k] = id;
    rec[id].kpos = k;
}

void 
HashRP::destroyHash (Thash *H)
{
    free (H->table);
    H->table = NULL;
    H->maxpos = 0;
    H->used = 0;
}
 
void 
HashRP::hashRepos (Thash *H, int id)
{
	Trecord *rec = H->Rec->records;
    H->table[rec[id].kpos] = id;
}
