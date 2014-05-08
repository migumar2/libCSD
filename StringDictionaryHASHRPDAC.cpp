/* StringDictionaryHASHRPDAC.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary based on double hashing.
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

#include "StringDictionaryHASHRPDAC.h"

StringDictionaryHASHRPDAC::StringDictionaryHASHRPDAC()
{
	this->type = HASHRPDAC;
	this->elements = 0;
	this->maxlength = 0;
}

StringDictionaryHASHRPDAC::StringDictionaryHASHRPDAC(IteratorDictString *it, uint len, int overhead)
{
	this->type = HASHRPDAC;
	this->elements = 0;
	this->maxlength = 0;

	uchar maxchar = 0;

	{
		// Counting the elements in Tdict for building the hash table structure
		uchar *strCurrent;
		uint lenCurrent=0;

		while (it->hasNext())
		{
			strCurrent = it->next(&lenCurrent);
			if (lenCurrent >= maxlength) maxlength = lenCurrent+1;
			elements++;

			for (uint i=0; i<lenCurrent; i++)
			{
				if (strCurrent[i] > maxchar) maxchar = strCurrent[i];
			}
		}

		maxchar++;
		((IteratorDictStringPlain*)it)->restart();
	}

	{
		// Initializing the hash table
		uint hash_size = (uint)(elements*(1+(overhead*1.0/100.0)));
		hash = new HashDAC(hash_size);
	}

	// Performing Tdict reorganization
	vector<SortString> sorting(elements);

	uchar *strCurrent;
	uint lenCurrent=0;
	uint processed=0;

	for (uint current=0; current<elements; current++)
	{
		strCurrent= it->next(&lenCurrent);

		// Storing the string position in Tdict
		sorting[current].original = processed;
		// Simulating the string insertion in the hash table
		sorting[current].hash = hash->insert(strCurrent, lenCurrent);

		processed += lenCurrent+1;
	}

	// String sorting for Tdict*
	std::sort(sorting.begin(), sorting.end(), sortTdict);

	// Obtaining Tdict*
	uchar *text = ((IteratorDictStringPlain*)it)->getPlainText();
	int *dict = new int[it->size()+elements];
	processed=0;

	for (uint current=0; current<elements; current++)
	{
		uint ptr = sorting[current].original;

		do
		{
			dict[processed] = text[ptr];
			processed++; ptr++;
		}
		while (text[ptr] != 0);

		dict[processed] = 0;
		processed++;
	}

	delete it;

	rp = new RePair(dict, processed, maxchar);

	{
		// Compacting the sequence (a -i value is inserted after the i-th string).
		int *cdict = new int[processed];
		uint io = 0, ic = 0, strings = 0;
		uint maxseq = 0, currentseq = 0;

		hash->setOffset(sorting[0].hash, 0);

		while (io<processed)
		{
			if (dict[io] >= 0)
			{
				if (dict[io] == 0)
				{
					if (currentseq > maxseq) maxseq = currentseq;

					hash->setOffset(sorting[strings].hash, ic);
					strings++;

					cdict[ic] = -strings;
					io++; ic++;
					currentseq = 0;
				}
				else
				{
					cdict[ic] = dict[io];
					io++; ic++;
					currentseq++;
				}
			}
			else
			{
				if (io < processed) io = -(dict[io]+1);
			}
		}

		delete [] dict;

		// Post-processing the hash
		hash->finish(ic);

		// Building the array for the sequence
		rp->Cdac = new DAC_VLS(cdict, ic-2, bits(rp->rules+rp->terminals), maxseq);

		delete [] cdict;
	}
}

uint
StringDictionaryHASHRPDAC::locate(uchar *str, uint strLen)
{
	uint id = NORESULT;

	size_t hval = bitwisehash(str, strLen, hash->tsize);
	size_t next;

	if(!hash->b_ht->access(hval))
		return id;

	uint pos = hash->b_ht->rank1(hval);

	if (rp->extractStringAndCompareDAC(pos, str, strLen) == 0)
		return pos;

	// using double hashing
	size_t h2 = step_value(str, strLen, hash->tsize);

	for(uint i=1; i<hash->tsize ; i++ )
	{
		next = (hval + i*h2)%hash->tsize;

		if(!hash->b_ht->access(next))
			return id;

		pos = hash->b_ht->rank1(next);

		if(rp->extractStringAndCompareDAC(pos, str, strLen) == 0)
			return hash->b_ht->rank1(next);
	}

	return id;
}

uchar *
StringDictionaryHASHRPDAC::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uint *rules;
		uint len = rp->Cdac->access(id, &rules);
		uchar *s = new uchar[maxlength+1];

		*strLen = 0;

		for (uint i=0; i<len; i++)
		{
			if (rules[i] >= rp->terminals) (*strLen) += rp->expandRule(rules[i]-rp->terminals, (s+(*strLen)));
			else
			{
				s[*strLen] = (uchar)rules[i];
				(*strLen)++;
			}
		}

		s[*strLen] = (uchar)'\0';
		delete [] rules;

		return s;
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

IteratorDictID*
StringDictionaryHASHRPDAC::locatePrefix(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide prefix location" << endl;
	return NULL;
}

IteratorDictID*
StringDictionaryHASHRPDAC::locateSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint
StringDictionaryHASHRPDAC::locateRank(uint rank)
{
	cout << "This dictionary does not provide rank location" << endl;
	return 0;
}

IteratorDictString*
StringDictionaryHASHRPDAC::extractPrefix(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide prefix extraction" << endl;
	return NULL;
}

IteratorDictString*
StringDictionaryHASHRPDAC::extractSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring extraction" << endl;
	return NULL;
}

uchar *
StringDictionaryHASHRPDAC::extractRank(uint rank, uint *strLen)
{
	cout << "This dictionary does not provide rank extraction" << endl;
	return NULL;
}

IteratorDictString*
StringDictionaryHASHRPDAC::extractTable()
{
	vector<uchar*> tabledec(elements);
	uint strLen;

	for (uint i=1; i<=elements; i++) tabledec[i-1] = extract(i, &strLen);

	return new IteratorDictStringVector(&tabledec, elements);
}

size_t
StringDictionaryHASHRPDAC::getSize()
{
	return hash->getSize()+rp->getSize()+sizeof(StringDictionaryHASHRPDAC);
}

void
StringDictionaryHASHRPDAC::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);

	rp->save(out, type);
	hash->save(out);
}

StringDictionary*
StringDictionaryHASHRPDAC::load(ifstream &in, uint technique)
{
	size_t type = loadValue<uint32_t>(in);
	if (type != HASHRPDAC) return NULL;

	StringDictionaryHASHRPDAC *dict = new StringDictionaryHASHRPDAC();
	dict->type = technique;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);

	dict->rp = RePair::load(in);
	dict->hash = HashDAC::load(in);

	return dict;
}

StringDictionaryHASHRPDAC::~StringDictionaryHASHRPDAC()
{
	delete rp;
	delete hash;
}

