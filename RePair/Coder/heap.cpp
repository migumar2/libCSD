
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

#include <stdlib.h>
#include <stdio.h>
#include "heap.h"

Theap 
Heap::createHeap (int u, Trarray *Rec, float factor, int minsize) 
{
    Theap H;
    int i;
    H.sqrtu = 2;
    while (H.sqrtu * H.sqrtu < u) H.sqrtu++;
    H.infreq = (Tarray*)malloc(H.sqrtu * sizeof(Tarray));
    for (i=1;i<H.sqrtu;i++) H.infreq[i] = ArrayG::createArray(Rec,factor,minsize);
    H.freq = (Thnode*)malloc (H.sqrtu * sizeof(Thnode));
    H.freef = 0;
    for (i=0;i<H.sqrtu-1;i++) H.freq[i].next = i+1;
    H.freq[H.sqrtu-1].next = -1;
    H.ff = (Thfreq*)malloc (H.sqrtu * sizeof(Thfreq));
    H.freeff = 0;
    for (i=0;i<H.sqrtu-1;i++) H.ff[i].larger = i+1;
    H.ff[H.sqrtu-1].larger = -1;
    H.smallest = H.largest = -1;
    H.max = H.sqrtu;
    H.Rec = Rec;
    return H;
}
  
void 
Heap::destroyHeap (Theap *H) 
{
    int i;
//    Thfreq *l,*n;
    for (i=1;i<H->sqrtu;i++) ArrayG::destroyArray(&H->infreq[i]);
    free (H->infreq); H->infreq = NULL;
    free (H->freq); H->freq = NULL;
    free (H->ff); H->ff = NULL;
    H->smallest = H->largest = -1;
    H->sqrtu = 0;
}

void 
Heap::move (Tarray A, int i, int j, Trecord *rec)
{
    int id = A.pairs[j];
    A.pairs[i] = id;
    rec[id].hpos = i;
}

void 
Heap::prnH (Theap *H)
{
    Thfreq *f;
    static int X = 0;
    int prevf = 1<<30;
    int fp = H->largest;
    if (fp == -1) return;
    X++;
    printf ("Heap %i = \n",X);
    while (fp != -1)
       { f = &H->ff[fp];
         printf ("freq=%i, elems=%i\n",f->freq,f->elems);
	 if (prevf <= f->freq)
	    { fp++; }
	 prevf = f->freq;
	 fp = f->smaller;
       }
}

void 
Heap::incFreq (Theap *H, int id) 
{
    Trecord *rec = H->Rec->records;
    Thnode *p;
    Thfreq *f,*lf;
    int fp,lfp;
	
    if ((rec[id].pair.left == 0) || (rec[id].pair.right == 0)) return;

    int freq = rec[id].freq++;
    int hpos = rec[id].hpos;
	
if (PRNH) prnH(H);
    if (freq >= H->sqrtu) // high freq part, hpos is a ptr within freq
       { p = &H->freq[hpos];
	 fp = p->fnode;
	 f = &H->ff[fp];
	 freq++;
		// shortcut for common case: f has only p and !exists f+1 
	 if ((p->prev == -1) && (p->next == -1) && 
	     ((f->larger == -1) || (H->ff[f->larger].freq != freq)))
	    { f->freq = freq; }
	 else  // the long way, hopefully not so common
	    {   	// remove p from f list
	      if (p->prev == -1) f->elems = p->next; 
	      else H->freq[p->prev].next = p->next;
	      if (p->next != -1) H->freq[p->next].prev = p->prev;
		     // add p to larger list (lf)
	      p->prev = -1;
	      if ((f->larger != -1) && (H->ff[f->larger].freq == freq)) 
						// next freq exists
	         { lfp = f->larger;
		   lf = &H->ff[lfp];
	           p->next = lf->elems;
		   H->freq[lf->elems].prev = hpos;
	         }
	      else // create one
	         { lfp = H->freeff;
	           lf = &H->ff[lfp];
		   H->freeff = lf->larger;
	           lf->freq = freq;
	           lf->smaller = fp; lf->larger = f->larger;
		   if (f->larger != -1) H->ff[f->larger].smaller = lfp;
		   else H->largest = lfp;
	           f->larger = lfp;
	           p->next = -1;
	         }
	      lf->elems = hpos;
	      p->fnode = lfp;
		     // see if f disappears
	      if (f->elems == -1)
	         { if (f->smaller == -1) H->smallest = f->larger;
	           else H->ff[f->smaller].larger = f->larger;
	           if (f->larger == -1) H->largest = f->smaller;
	           else H->ff[f->larger].smaller = f->smaller;
	           f->larger = H->freeff;
		   H->freeff = fp;
	         }
	    }
       }
    else // del from low freq part
       { move (H->infreq[freq],hpos,H->infreq[freq].fst,rec);
	 ArrayG::deleteArray(&H->infreq[freq]);
	 freq++;
         if (freq < H->sqrtu) // ins in low freq part
            { rec[id].hpos = ArrayG::insertArray (&H->infreq[freq],id);
            }
	 else // ins in freq part 
	    { 		// allocate a free cell for it
	      hpos = H->freef;
	      H->freef = H->freq[H->freef].next;
	      rec[id].hpos = hpos;
	      p = &H->freq[hpos];
	      p->prev = -1;
	      p->id = id;
	      if ((H->smallest != -1) && (H->ff[H->smallest].freq == freq)) 
							// freq exists
		 { fp = H->smallest;
		   f = &H->ff[fp];
		   p->next = f->elems;
		   H->freq[f->elems].prev = hpos;
		 }
	      else // create freq
	         { fp = H->freeff;
		   f = &H->ff[fp];
		   H->freeff = f->larger;
	           f->freq = freq;
	           f->smaller = -1;
		   f->larger = H->smallest;
		   if (H->smallest != -1) H->ff[H->smallest].smaller = fp;
		   H->smallest = fp;
		   if (H->largest == -1) H->largest = fp;
	           p->next = -1;
	         }
	      f->elems = hpos;
	      p->fnode = fp;
	    }
       }
}

void 
Heap::decFreq (Theap *H, int id) 
{
    Trecord *rec = H->Rec->records;
    int freq = rec[id].freq--;
    int hpos = rec[id].hpos;
    Thnode *p;
    Thfreq *f,*sf;
    int fp,sfp;
if (PRNH) prnH(H);
    if (freq > H->sqrtu) // high freq part
       { p = &H->freq[hpos];
	 fp = p->fnode;
	 f = &H->ff[p->fnode];
	 freq--;
		// shortcut for common case: f has only p and !exists f-1 
	 if ((p->prev == -1) && (p->next == -1) && 
	     ((f->smaller == -1) || (H->ff[f->smaller].freq != freq)))
	    { f->freq = freq; }
	 else  // the long way, hopefully not so common
	    {   	// remove p from f list
	      if (p->prev == -1) f->elems = p->next; 
	      else H->freq[p->prev].next = p->next;
	      if (p->next != -1) H->freq[p->next].prev = p->prev;
		     // add p to smaller list (sf)
	      p->prev = -1;
	      if ((f->smaller != -1) && (H->ff[f->smaller].freq == freq)) 
							// next freq exists
	         { sfp = f->smaller;
	           sf = &H->ff[sfp];
	           p->next = sf->elems;
		   H->freq[sf->elems].prev = hpos;
	         }
	      else // create one
	         { sfp = H->freeff;
		   sf = &H->ff[sfp];
		   H->freeff = sf->larger;
	           sf->freq = freq;
	           sf->larger = fp; sf->smaller = f->smaller;
		   if (f->smaller != -1) H->ff[f->smaller].larger = sfp;
		   else H->smallest = sfp;
	           f->smaller = sfp;
	           p->next = -1;
	         }
	      sf->elems = hpos;
	      p->fnode = sfp;
		     // see if f disappears
	      if (f->elems == -1)
	         { if (f->smaller == -1) H->smallest = f->larger;
	           else H->ff[f->smaller].larger = f->larger;
	           if (f->larger == -1) H->largest = f->smaller;
	           else H->ff[f->larger].smaller = f->smaller;
	           f->larger = H->freeff;
		   H->freeff = fp;
	         }
	    }
       }
    else // ins in low freq part
       { if (freq < H->sqrtu) // del from low freq part
            { move (H->infreq[freq],hpos,H->infreq[freq].fst,rec);
	      ArrayG::deleteArray(&H->infreq[freq]);
	    }
	 else // del from heap, must be minimal 
	    {   	// remove p from f list
              p = &H->freq[hpos];
	      fp = p->fnode;
	      f = &H->ff[p->fnode];
	      if (p->prev == -1) f->elems = p->next; 
	      else H->freq[p->prev].next = p->next;
	      if (p->next != -1) H->freq[p->next].prev = p->prev;
			// add its cell to free list
	      p->next = H->freef;
	      H->freef = hpos;
		     // see if f disappears
	      if (f->elems == -1)
	         { H->smallest = f->larger;
	           if (f->larger == -1) H->largest = -1;
	           else H->ff[f->larger].smaller = -1;
	           f->larger = H->freeff;
		   H->freeff = fp;
	         }
	    }
	 if (--freq > 0) // freq 0 disappears from pairs
            rec[id].hpos = ArrayG::insertArray (&H->infreq[freq],id);
	 else Records::removeRecord(H->Rec,id);
       }
}

void 
Heap::insertHeap (Theap *H, int id)  
{
    Trecord *rec = H->Rec->records;
    rec[id].hpos = ArrayG::insertArray (&H->infreq[1],id);
    rec[id].freq = 1;
}

int 
Heap::extractMax (Theap *H)
{
//***    Trecord *rec = H->Rec->records;
    int ret;
    Thnode *p;
    Thfreq *f;
    int fp;
if (PRNH) prnH(H);
    if ((H->max == H->sqrtu) && (H->largest == -1)) H->max--;
    if (H->max < H->sqrtu)
       { while (H->max && (H->infreq[H->max].size == 0)) H->max--;
	 if (!H->max) return -1; // empty heap
         ret = H->infreq[H->max].pairs[H->infreq[H->max].fst];
         ArrayG::deleteArray(&H->infreq[H->max]);
       }
    else
       { fp = H->largest;
	 f = &H->ff[fp];
	 p = &H->freq[f->elems];
         ret = p->id;
		// remove element
	 f->elems = p->next;
	 if (p->next != -1) H->freq[p->next].prev = -1;
	 else { // remove f as well
	        H->largest = f->smaller;
	        if (f->smaller == -1) H->smallest = -1;
	        else H->ff[f->smaller].larger = -1;
	        f->larger = H->freeff;
		H->freeff = fp;
	      }
		// add elem to free list
	 p->next = H->freef;
	 H->freef = p-H->freq;
       }
    return ret;
}

void 
Heap::purgeHeap (Theap *H) 
{
//    Trecord *rec = H->Rec->records;
//    int i,id,fst,size,max;
    int id,fst,size,max;
    size = H->infreq[1].size;
    fst = H->infreq[1].fst;
    max = H->infreq[1].maxsize;
    while (size--)
	{ id = H->infreq[1].pairs[fst];
          fst = (fst+1) % max;
	  Records::removeRecord (H->Rec,id);
	}
    ArrayG::destroyArray(&H->infreq[1]);
}

void 
Heap::heapRepos (Theap *H, int id) 
{
    Trecord *rec = H->Rec->records;
    if (rec[id].freq < H->sqrtu) 
         H->infreq[rec[id].freq].pairs[rec[id].hpos] = id;
    else H->freq[rec[id].hpos].id = id;
}
