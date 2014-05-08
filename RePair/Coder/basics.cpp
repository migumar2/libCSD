
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

#include <stdlib.h>
#include <stdio.h>

int NullFreq = 1 << (8*sizeof(int)-1);

void *myMalloc (long long n)
{
    void *p;
    if (n == 0) return NULL;
    p = (void*)malloc(n);
    if (p == NULL)
       { fprintf(stderr,"Error: malloc failed\n");
	 exit(1);
       }
    return p;
}

void *myRealloc (void *p, long long n)
{
    if (n == 0) { free(p); return NULL; }
    if (p == NULL) return myMalloc(n);
    p = (void*)realloc(p,n);
    if (p == NULL)
       { fprintf(stderr,"Error: realloc failed\n");
	 exit(1);
       }
    return p;
}

int blog (int x)
{
     int l=0;
     while (x) { x>>=1; l++; }
     return l;
}
