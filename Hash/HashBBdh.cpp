/* HashBBdh.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Class implementating the HashBB dictionary.
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


#include "HashBBdh.h"

	HashBBdh::HashBBdh(){
		tsize=0;
		hashtable=NULL;
		b_ht = NULL;
		offsets = NULL;
	}

	HashBBdh::HashBBdh(uint _tsize){
		uint i;
		tsize = nearest_prime(_tsize);
		n = 0;
		hashtable = new size_t[tsize];
		for(i=0 ; i<tsize ; i++ )
			hashtable[i] = (size_t)-1;
	}

	size_t HashBBdh::search(uchar *w, size_t len)
	{
		size_t hval = bitwisehash(w, len, tsize);
		size_t pos, off_pos;

		if(!b_ht->access(hval))
			return (size_t)-1;

		pos = b_ht->rank1(hval);
		off_pos = offsets->select1(pos);

		if(scmp(off_pos, w, len) == 0)
			return pos-1;

		//using double hashing
		size_t h2 = step_value(w, len, tsize);
		for(uint i=1 ; i<tsize ; i++ ){
			hval = (hval + h2)%tsize;

			if(!b_ht->access(hval))
				return (size_t)-1;

			pos = b_ht->rank1(hval);
			off_pos = offsets->select1(pos);

			if(scmp(off_pos, w, len) == 0)
				return pos-1;
		}
		return (size_t)-1;
	}

	size_t HashBBdh::getValue(size_t i)
	{
		return offsets->select1(i);
	}

	size_t
	HashBBdh::getValuePos(size_t i)
	{
		return offsets->select1(b_ht->rank1(i));
	}
			
	size_t HashBBdh::getSize()
	{
		uint mem = sizeof(HashBBdh);
		mem += offsets->getSize();
		mem += b_ht->getSize();
		return mem;
	}

	HashBBdh * HashBBdh::load(ifstream & fp)
	{
		HashBBdh *h_new = new HashBBdh();

		h_new->tsize = loadValue<size_t>(fp);
		h_new->n = loadValue<size_t>(fp);
		h_new->hash = new LogSequence(fp);
		h_new->b_ht = BitSequence::load(fp);

		uint last = (h_new->b_ht)->select1(h_new->n);
		BitString *offsets = new BitString(h_new->hash->getField(last)+1);

		uint ones = 0;

		for (uint i=1; i<=h_new->n; i++)
		{

			uint pos = (h_new->b_ht)->select1(i);
			offsets->setBit(h_new->hash->getField(pos), true);

			ones++;
		}

		// This additional bit is required by RRR
		offsets->setBit(h_new->hash->getField((h_new->b_ht)->select1(h_new->n)), true);
		h_new->offsets = new BitSequenceRRR(*offsets);

		delete offsets;
		delete h_new->hash;

		return h_new;
	}

	HashBBdh::~HashBBdh()
	{
		delete offsets;
		delete b_ht;
	}
