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

#ifndef IREPAIR
#define IREPAIR

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "basics.h"
#include "records.h"
#include "hash.h"
#include "heap.h"
#include "dictionary.h"

static const int PRNC = 0;
static const int PRNR = 0;
static const int PRNP = 0;
static const int PRNL = 0;
// to avoid many reallocs at small sizes, should be ok as is
static const int minsize = 256;

class IRePair
{
public:
	IRePair(){};
	int compress(int *text, unsigned int length, size_t *csymbols, size_t *crules, Tdiccarray **rules);

private:
	int u;		// |text| and later current |C| with gaps
	int *C; 		// compressed text
	int c;  		// real |C|
	int alph;	// max used terminal symbol
	int n; 		// |R|
	Tlist *L; 	// |L| = c;
	Thash Hash; 	// hash table of pairs
	Theap Heap; 	// special heap of pairs
	Trarray Rec; 	// records
	Tdiccarray Dicc;	// compressed dictionary

	int repair();
	int expand (int i, int d);

	void prepare(int len);
	void prnSym(int c);
	void prnC(void);
	void prnRec(void);
};
#endif


