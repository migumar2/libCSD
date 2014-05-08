/* Hash.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Abstract class for implementing compressed hash dictionaries.
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

#include "Hash.h"

	size_t
	Hash::insert(uchar *w, size_t len, size_t offset)
	{
		size_t hval = bitwisehash(w, len, tsize);

		if(hashtable[hval] == (size_t)-1)
		{
			 hashtable[hval] = offset;
			 enclength[hval] = len;
			 n++;
			 return hval;
		}
		else
		{
			 //use double hashing
			 size_t h2 = step_value(w, len, tsize);
			 for(size_t i=1; i<tsize; i++){
				 hval = (hval + h2)%tsize;
				 if(hashtable[hval] == (size_t)-1){
					 hashtable[hval] = offset;
					 enclength[hval] = len;
					 n++;
					 return hval;
				 }
			 }
			 cout << "Error Hash table full" << endl;
			 return (size_t)-1;
		}
	}

	void
	Hash::setOffset(size_t pos, size_t offset)
	{
		hashtable[pos] = offset;
	}

	void
	Hash::finish(size_t length)
	{
		size_t b_size = (tsize+W-1)/W;

		hash = new LogSequence(bits(length), tsize);
		uint *bitmap = new uint[b_size];
		for(size_t i=0; i<b_size; i++) bitmap[i]=0;

		for(size_t i=0; i<tsize; i++)
		{
			if(hashtable[i]!=(size_t)-1)
			{
				hash->setField(i, hashtable[i]);
				bitset(bitmap, i);
			}
			else hash->setField(i, 0);
		}

		b_ht = new BitSequenceRG(bitmap, tsize, 20);

		delete [] bitmap;
		delete [] hashtable;
		delete [] enclength;
	}

	void
	Hash::setData(uchar *data)
	{
		this->data = data;
	}

	void
	Hash::save(ofstream & fp)
	{
		saveValue(fp, tsize);
		saveValue(fp, n);
		hash->save(fp);
		b_ht->save(fp);
	}

	Hash *
	Hash::load(ifstream & fp, int r)
	{
		switch(r) {
			// HASHUFF and HASHRP must be the same value
			case (HASHUFF):
				return Hashdh::load(fp);
			// HASBHUFF and HASHVRP must be the same value
			case (HASHBHUFF): 
				return HashBdh::load(fp);
			// HASBBHUFF and HASBBHRP must be the same value
			case (HASHBBHUFF): 
				return HashBBdh::load(fp);
		}

		return NULL;
	}

	int
	Hash::scmp(size_t offset, uchar *w, size_t len)
	{
		size_t ini_pos =  offset;
		for(size_t i=0; i<len; i++){
			if(w[i] != data[ini_pos+i])
				return 1;
		}

		return 0;
	}
