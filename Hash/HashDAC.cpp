/* HashDAC.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Class adapting the HashBB dictionary for managing DAC.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Contacting the authors:
 *   Francisco Claude:  	fclaude@recoded.cl
 *   Rodrigo Canovas:  		rcanovas@student.unimelb.edu.au
 *   Miguel A. Martinez-Prieto:	migumar2@infor.uva.es
 */

#include "HashDAC.h"

HashDAC::HashDAC()
{
	tsize=0;

	b_ht = NULL;
	hashtable=NULL;
}

HashDAC::HashDAC(size_t tsize)
{
	this->tsize = nearest_prime(tsize);
	n = 0;

	hashtable = new size_t[this->tsize];

	uint i;
	for(i=0 ; i<this->tsize ; i++) hashtable[i] = (size_t)-1;
}

size_t
HashDAC::insert(uchar *w, size_t len, size_t offset)
{
	size_t hval = bitwisehash(w, len, tsize);

	if(hashtable[hval] == (size_t)-1)
	{
		 hashtable[hval] = offset;
		 n++;
		 return hval;
	}
	else
	{
		 //use double hashing
		 size_t h2 = step_value(w,len, tsize);
		 for(size_t i=1; i<tsize; i++){
			 hval = (hval + h2)%tsize;
			 if(hashtable[hval] == (size_t)-1){
				 hashtable[hval] = offset;
				 n++;
				 return hval;
			 }
		 }
		 cout << "Error Hash table full" << endl;
		 return (size_t)-1;
	}
}

void
HashDAC::setOffset(size_t pos, size_t offset)
{
	hashtable[pos] = offset;
}

void
HashDAC::finish(size_t length)
{
	size_t b_size = (tsize+W-1)/W;

	uint *bitmap = new uint[b_size];
	for(size_t i=0; i<b_size; i++) bitmap[i]=0;

	for(size_t i=0; i<tsize; i++)
		if(hashtable[i]!=(size_t)-1) bitset(bitmap, i);

	b_ht = new BitSequenceRG(bitmap, tsize, 20);

	delete [] bitmap;
	delete [] hashtable;
}

void
HashDAC::setData(DAC_BVLS *data)
{
	this->data = data;
}

size_t
HashDAC::search(uchar *w, size_t len)
{
	uint hval = bitwisehash(w, len, tsize);

	if(!b_ht->access(hval)) return (size_t)-1;
	uint pos = b_ht->rank1(hval)-1;

	if(scmp(pos, w, len) == 0) return pos;

	//using double hashing
	size_t h2 = step_value(w,len, tsize);

	for(size_t i=1 ; i<tsize ; i++ ){
		hval = (hval + h2)%tsize;

		if(!b_ht->access(hval)) return (size_t)-1;
		pos = b_ht->rank1(hval)-1;
		if(scmp(pos, w, len) == 0) return pos;
	}

	return (size_t)-1;

}

size_t
HashDAC::getSize()
{
	uint mem = sizeof(HashDAC);
	mem += b_ht->getSize();
	return mem;
}

void
HashDAC::save(ofstream & fp)
{
	saveValue(fp, tsize);
	saveValue(fp, n);
	b_ht->save(fp);
}

HashDAC *
HashDAC::load(ifstream & fp)
{
	HashDAC *h_new = new HashDAC();

	h_new->tsize = loadValue<size_t>(fp);
	h_new->n = loadValue<size_t>(fp);
	h_new->b_ht = BitSequence::load(fp);

	return h_new;
}

int
HashDAC::scmp(size_t pos, uchar *w, size_t len)
{
	uint id = pos+1;
	uint level = 0;
	size_t value;

	while(id != (uint)-1)
	{
		value = data->access_next(level, &id);
		if(w[level] != value) return 1;
		level++;
	}

	return 0;
}


HashDAC::~HashDAC()
{
	delete b_ht;
}


