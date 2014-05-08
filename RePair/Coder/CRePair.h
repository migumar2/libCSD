/* CRePair.h
   Copyright (C) 2007, Rodrigo Gonzalez, Francisco Claude F, all rights reserved.

   Compressed RePair

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef CREPAIR_H
#define CREPAIR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include <Array.h>
#include <BitSequence.h>

using namespace cds_utils;
using namespace cds_static;

using namespace std;

class CRePair {

  public:
	CRePair();

	void compress(uint min_value, uint max_value, uint max_assigned, uint m, uint n, uint *symbols, uint *symbols_pair);

	uint* getSequence();
	Array* getSymbolsGrammar();
	BitSequence* getBitsGrammar();

	~CRePair();
      
  protected:
	uint min_value, max_value, max_assigned, m, n, pos, symbols_new_len;
	uint *symbols, *symbols_pair, *symbols_new, *symbols_new_bit;
	Array * final_symbols_dict;
	BitSequence *BRR;

	bool compress_pair_table();
	void fillBR();
	void unroll(uint s, uint i);
	void new_value(uint *symbols_pair, uint *symbols_new_value, uint *k1, uint *j, uint pos);
};

#endif

