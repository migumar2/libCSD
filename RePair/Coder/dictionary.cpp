
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

	// extendible array for rules

#include <stdlib.h>
#include "dictionary.h"

int 
Dictionary::insertRule (Tdiccarray *Dicc, Trule rule)
   { int id;
     Trule *nrule;
     if (Dicc->size == Dicc->maxsize)
	{ if (Dicc->maxsize == 0)
	     { Dicc->maxsize = Dicc->minsize;
	       Dicc->rules = (Trule*) malloc (Dicc->maxsize * sizeof(Trule));
	     }
	  else
	     { Dicc->maxsize /= Dicc->factor;
	       Dicc->rules = (Trule*) realloc (Dicc->rules, Dicc->maxsize * sizeof(Trule));
	     }
	}
     id = Dicc->size++;
     nrule = &Dicc->rules[id];
     *nrule = rule;
	 
     return id;
   }


Tdiccarray 
Dictionary::createDicc (float factor, int minsize)
   { Tdiccarray Dicc;
     Dicc.rules = NULL;
     Dicc.maxsize = 0;
     Dicc.size = 0;
     Dicc.factor = factor;
     Dicc.minsize = minsize;
     return Dicc;
   }

void 
Dictionary::destroyDicc (Tdiccarray *Dicc)
   { if (Dicc->maxsize == 0) return;
     free (Dicc->rules);
     Dicc->rules = NULL;
     Dicc->maxsize = 0;
     Dicc->size = 0;
   }
   
