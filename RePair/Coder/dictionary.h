
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

#ifndef DICTIONARYINCLUDED
#define DICTIONARYINCLUDED

#include <stdio.h>
#include "basics.h"

typedef struct
   { Tpair rule; // left and righ component
     int l;	 // rule length
   } Trule;

typedef struct
   { Trule *rules; 
     int maxsize;  
     int size;
     float factor;
     int minsize;
   } Tdiccarray;

// contents can be accessed as Dic.rules[0..Dicc.size-1]
class Dictionary
{
public:
	// inserts rule in Dicc, returns id
	static int insertRule (Tdiccarray *Dicc, Trule rule); 
	 // creates empty dictionary
	static Tdiccarray createDicc (float factor, int minsize);
	// destroys Rec
	static void destroyDicc (Tdiccarray *Dicc);
};
#endif
