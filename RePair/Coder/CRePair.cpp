/* CRePair.cpp
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

#include "CRePair.h"

CRePair::CRePair() { }

void
CRePair::compress(uint min_value, uint max_value, uint max_assigned, uint m, uint n, uint *symbols, uint *symbols_pair)
{
	this->min_value = min_value;
	this->max_value = max_value;
	this->max_assigned = max_assigned;
	this->m = m;
	this->n = n;
	this->symbols = symbols;
	this->symbols_pair = symbols_pair;

	compress_pair_table();
}

uint*
CRePair::getSequence()
{
	return symbols;
}

Array*
CRePair::getSymbolsGrammar()
{
	return final_symbols_dict;
}

BitSequence*
CRePair::getBitsGrammar()
{
	return BRR;
}


bool 
CRePair::compress_pair_table() 
{
	uint aux,j,i;

	/* Compress table of pairs */
	fillBR();

	uint *symbols_pair_tmp = (uint*) malloc(sizeof(uint)*(max_assigned-max_value));
	for (i =0 ; i< (max_assigned-max_value) ; i++) symbols_pair_tmp[i]=0;

	for (i =0 ; i< (max_assigned-max_value) ; i++) 
	{
		aux=symbols_pair[2*i]; 
		if (aux > max_value) symbols_pair_tmp[aux-max_value-1]++;

		aux=symbols_pair[2*i+1]; 
		if (aux > max_value) symbols_pair_tmp[aux-max_value-1]++;
  	}

	j=0;
	for (i =0 ; i< (max_assigned-max_value); i++) 
	{
    		if (symbols_pair_tmp[i] != 0) j++;
  	}

	symbols_new_len = 2*(max_assigned-max_value)-j;
	symbols_new = (uint*) malloc(sizeof(uint)*(symbols_new_len));
	symbols_new_bit = new uint[((symbols_new_len+(max_assigned-max_value))/W+1)];
	uint *symbols_new_value = (uint*) malloc(sizeof(uint)*(max_assigned-max_value));

	for (i =0 ; i<((symbols_new_len+(max_assigned-max_value))/W+1);i++) symbols_new_bit[i]=0;
	for (i =0 ; i<symbols_new_len;i++) symbols_new[i]=0;
	for (i =0 ; i<(max_assigned-max_value);i++) symbols_new_value[i]=0;

	j=1;
	uint k1=0;
	for (i =0 ; i< (max_assigned-max_value) ; i++)
	{
		if (symbols_pair_tmp[i] == 0) 
		{
			symbols_new_value[i]=j; 
			bitset(symbols_new_bit,j-1); j++;
			new_value(symbols_pair,symbols_new_value,&k1,&j,i);
		}
	}

	uint symbols_new_bit_len = j;
	free(symbols_pair_tmp);

	{
		// OBTAINING THE FINAL SEQUENCE
		for (i =0 ; i< m ; i++)
		{
			if (symbols[i] > max_value)
			{
				cout << symbols[i] << " -> " << symbols_new_value[symbols[i]-max_value-1]+max_value << endl;
				symbols[i] = symbols_new_value[symbols[i]-max_value-1]+max_value;
			}
		}

		delete [] symbols_pair;
		free(symbols_new_value);
	}

	{
		// OBTAINING THE COMPRESSED GRAMMAR
		uint *symbols_new_bit_aux = new uint [(symbols_new_bit_len/W+1)];
		for (i =0 ; i<symbols_new_bit_len/W+1;i++) symbols_new_bit_aux[i]= symbols_new_bit[i];
		delete [] symbols_new_bit;
		symbols_new_bit=symbols_new_bit_aux;

		BRR = new BitSequenceRG((uint*)symbols_new_bit, (uint)symbols_new_bit_len, 20);
		delete [] symbols_new_bit;


		uint max_nd = symbols_new[0];
		for(uint k = 1; k<symbols_new_len; k++)
		{
			cerr << symbols_new[k] << endl;
			if(max_nd < symbols_new[k]) max_nd = symbols_new[k];
		}

		cerr << "**** " << symbols_new_len << " (" << max_nd << ") ***" << endl;

		final_symbols_dict = new Array(symbols_new_len, max_nd);
		for(uint k=0; k<symbols_new_len; k++) final_symbols_dict->setField(k, symbols_new[k]);

		free(symbols_new);
	}

	return true;
}

void 
CRePair::fillBR() 
{
	uint bits_BR_len = (uint)(n+2);
	uint *bits_BR = new uint[((bits_BR_len+W-1)/W)];

	for(uint i=0; i<(W-1+bits_BR_len)/W;i++) bits_BR[i]=0;

	pos = 0;
	for(uint i=0;i<m;i++) { bitset(bits_BR,pos); unroll(symbols[i],i); }
	bitset(bits_BR,pos);

	delete [] bits_BR;
}


void 
CRePair::unroll(uint s, uint i) 
{
	if(s>max_value) 
	{
		unroll(symbols_pair[2*(s-max_value-1)],i);
		unroll(symbols_pair[1+2*(s-max_value-1)],i);
		return;
	}
	pos++;
}

void 
CRePair::new_value(uint *symbols_pair, uint *symbols_new_value, uint *k1, uint *j, uint pos) 
{
	uint izq,der;
	izq=symbols_pair[2*pos];
	der=symbols_pair[2*pos+1];

	if (izq>max_value) 
	{
		izq=izq-max_value-1;
		if (symbols_new_value[izq] == 0) 
		{
 			symbols_new_value[izq]=*j; 
			bitset(symbols_new_bit,*j-1);
			(*j)++;
      			new_value(symbols_pair,symbols_new_value,k1,j,izq);
    		} 
		else 
		{
			symbols_new[*k1]=symbols_new_value[izq]+max_value; 
			(*j)++; 
			(*k1)++;
		}
	} 
	else 
	{
		symbols_new[*k1]=izq;
		(*j)++;
		(*k1)++;
	}

	if (der>max_value)
	{
		der=der-max_value-1;
		if (symbols_new_value[der] == 0) 
		{
			symbols_new_value[der]=*j; 
			bitset(symbols_new_bit,*j-1); 
			(*j)++;
			new_value(symbols_pair,symbols_new_value,k1,j,der);
    		} 
		else 
		{
			symbols_new[*k1]=symbols_new_value[der]+max_value; 
			(*j)++; 
			(*k1)++;
		}
	} 
	else 
	{
		symbols_new[*k1]=der;
		(*j)++;
		(*k1)++;
	}
}

CRePair::~CRePair()
{
}

