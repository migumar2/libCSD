/* HashBdh.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Class implementating the HashB dictionary (Huffman compression).
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

#include "HashBdh.h"

	HashBdh::HashBdh()
	{
		tsize=0;
		hashtable=NULL;
		hash = NULL;
		b_ht = NULL;
	}

	HashBdh::HashBdh(size_t _tsize)
	{
		uint i;
		tsize = nearest_prime(_tsize);
		n = 0;
		hashtable = new size_t[tsize];

		for(i=0 ; i<tsize ; i++)
			hashtable[i] = (size_t)-1;
	}
	
	size_t HashBdh::search(uchar *w, size_t len)
	{
		uint hval = bitwisehash(w, len, tsize);
		uint pos;

		if(!b_ht->access(hval))
			return (size_t)-1;

		pos = b_ht->rank1(hval)-1;

		if(scmp(hash->getField(pos), w, len) == 0)
			return pos;

		//using double hashing
		size_t h2 = step_value(w,len, tsize);

		for(size_t i=1 ; i<tsize ; i++ ){
			hval = (hval + h2)%tsize;

			if(!b_ht->access(hval))
				return (size_t)-1;
			pos = b_ht->rank1(hval)-1;

			if(scmp(hash->getField(pos), w, len) == 0)
				return pos;
		}

		return (size_t)-1; 
	}

	size_t
	HashBdh::getValue(size_t i)
	{
		return hash->getField(i-1);
	}

	size_t
	HashBdh::getValuePos(size_t i)
	{
		return hash->getField(b_ht->rank1(i)-1);
	}


	size_t HashBdh::getSize()
	{
		uint mem = sizeof(HashBdh);
		mem += hash->getSize();
		mem += b_ht->getSize();
		return mem;
	}

	HashBdh * HashBdh::load(ifstream & fp)
	{
		HashBdh *h_new = new HashBdh();

		h_new->tsize = loadValue<size_t>(fp);
		h_new->n = loadValue<size_t>(fp);
		LogSequence *seq = new LogSequence(fp);
		h_new->b_ht = BitSequence::load(fp);

		h_new->hash = new LogSequence(seq->getNumbits(), h_new->n);

		for (uint i=1; i<=h_new->n; i++)
		{
			uint pos = (h_new->b_ht)->select1(i);
			(h_new->hash)->setField(i-1, seq->getField(pos));
		}

		delete seq;

		return h_new;
	}

	HashBdh::~HashBdh()
	{
		delete hash;
		delete b_ht;
	}

