/* Hashdh.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements the core functionality for a compressed hash dictionary
 * based on double hashing (dh).
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

#include "Hashdh.h"

	Hashdh::Hashdh()
	{
		tsize=0;
		hashtable=NULL;
		hash = NULL;
		b_ht = NULL;
	}

	Hashdh::Hashdh(uint _tsize)
	{
		tsize = nearest_prime(_tsize);
		n = 0;

		hashtable = new size_t[tsize];
		enclength = new size_t[tsize];

		uint i;
		for(i=0 ; i<tsize ; i++)
		{
			hashtable[i] = (size_t)-1;
			enclength[i] = 0;
		}
	}

	size_t
	Hashdh::search(uchar *w, size_t len)
	{
		size_t hval = bitwisehash(w, len, tsize);
		size_t next;

		if(!b_ht->access(hval))
			return (size_t)-1;

		if(scmp(hash->getField(hval), w, len) == 0)
			return b_ht->rank1(hval)-1;

		//using double hashing
		size_t h2 = step_value(w, len, tsize);

		for(uint i=1 ; i<tsize ; i++ ){
			next = (hval + i*h2)%tsize;

			if(!b_ht->access(next))
				return (size_t)-1;

			if(scmp(hash->getField(next), w, len) == 0)
				return b_ht->rank1(next)-1;
		}

		return (size_t)-1;
	}

	size_t
	Hashdh::getValue(size_t i)
	{
		return hash->getField(b_ht->select1(i));
	}

	size_t
	Hashdh::getValuePos(size_t i)
	{
		return hash->getField(i);
	}

	size_t
	Hashdh::getSize()
	{
		uint mem = sizeof(Hashdh);
		mem += hash->getSize();
		mem += b_ht->getSize();
		return mem;
	}

	Hashdh *
	Hashdh::load(ifstream & fp)
	{
		Hashdh *h_new = new Hashdh();

		h_new->tsize = loadValue<size_t>(fp);
		h_new->n = loadValue<size_t>(fp);
		h_new->hash = new LogSequence(fp);
		h_new->b_ht = BitSequence::load(fp);

		return h_new;
	}

	Hashdh::~Hashdh()
	{
		delete hash;
		delete b_ht;
	}
