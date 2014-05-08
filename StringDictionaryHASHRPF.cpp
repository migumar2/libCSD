/* StringDictionaryHASHRPF.cpp
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

#include "StringDictionaryHASHRPF.h"

StringDictionaryHASHRPF::StringDictionaryHASHRPF()
{
	this->type = HASHRPF;
	this->elements = 0;
	this->maxlength = 0;
}

StringDictionaryHASHRPF::StringDictionaryHASHRPF(IteratorDictString *it, uint len, int overhead)
{
	this->type = HASHRPF;
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
		hash = new Hashdh(hash_size);
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

		ptr++;
		dict[processed] = maxchar; processed++;
		dict[processed] = 0; processed++;
	}

	processed--;
	delete it;

	rp = new RePair(dict, processed, maxchar);

	{
		// Compacting the sequence
		vector<size_t> textStrings;
		size_t bytesStrings = 0;
		uint io = 0, strings = 0;

		hash->setOffset(sorting[0].hash, 0);

		while (io<processed)
		{
			if (dict[io] >= 0)
			{
				if (dict[io] == 0)
				{
					strings++; io++;
					hash->setOffset(sorting[strings].hash, bytesStrings);
				}
				else
				{
					textStrings.push_back(dict[io]);
					io++; bytesStrings++;
				}
			}
			else
			{
				if (io < processed) io = -(dict[io]+1);
			}
		}

		// Post-processing the hash
		hash->finish(bytesStrings);

		// Building the array for the sequence
		rp->Cls = new LogSequence(&textStrings, bits(rp->rules+rp->terminals));
	}
}

uint
StringDictionaryHASHRPF::locate(uchar *str, uint strLen)
{
	uint id = NORESULT;

	size_t hval = bitwisehash(str, strLen, hash->tsize);
	size_t next;

	if(!hash->b_ht->access(hval))
		return id;

	if (rp->extractStringAndCompareRP(hash->getValuePos(hval), str, strLen) == 0)
		return hash->b_ht->rank1(hval);

	// using double hashing
	size_t h2 = step_value(str, strLen, hash->tsize);

	for(uint i=1; i<hash->tsize ; i++ )
	{
		next = (hval + i*h2)%hash->tsize;

		if(!hash->b_ht->access(next))
			return id;

		if(rp->extractStringAndCompareRP(hash->getValuePos(next), str, strLen) == 0)
			return hash->b_ht->rank1(next);
	}

	return id;
}

uchar *
StringDictionaryHASHRPF::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uchar *s = new uchar[maxlength+1];
		*strLen = 0;

		uint position = hash->getValue(id);

		uint rule;
		uint ptr = 0;

		while (true)
		{
			rule = rp->Cls->getField(position+ptr); ptr++;

			if (rule >= rp->terminals) (*strLen) += rp->expandRule(rule-rp->terminals, (s+(*strLen)));
			else
			{
				s[*strLen] = (uchar)rule;
				(*strLen)++;
			}

			if (s[*strLen-1] == rp->maxchar) break;
		}

		(*strLen)--;
		s[*strLen] = (uchar)'\0';

		return s;
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

IteratorDictID*
StringDictionaryHASHRPF::locatePrefix(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide prefix location" << endl;
	return NULL;
}

IteratorDictID*
StringDictionaryHASHRPF::locateSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint
StringDictionaryHASHRPF::locateRank(uint rank)
{
	cout << "This dictionary does not provide rank location" << endl;
	return 0;
}

IteratorDictString*
StringDictionaryHASHRPF::extractPrefix(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide prefix extraction" << endl;
	return NULL;
}

IteratorDictString*
StringDictionaryHASHRPF::extractSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring extraction" << endl;
	return NULL;
}

uchar *
StringDictionaryHASHRPF::extractRank(uint rank, uint *strLen)
{
	cout << "This dictionary does not provide rank extraction" << endl;
	return NULL;
}

IteratorDictString*
StringDictionaryHASHRPF::extractTable()
{
	vector<uchar*> tabledec(elements);
	uint strLen;

	for (uint i=1; i<=elements; i++) tabledec[i-1] = extract(i, &strLen);

	return new IteratorDictStringVector(&tabledec, elements);
}

size_t
StringDictionaryHASHRPF::getSize()
{
	return hash->getSize()+rp->getSize()+sizeof(StringDictionaryHASHRPF);
}

void
StringDictionaryHASHRPF::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);

	rp->save(out, HASHRPF);
	hash->save(out);
}

StringDictionary*
StringDictionaryHASHRPF::load(ifstream &in, uint technique)
{
	size_t type = loadValue<uint32_t>(in);

	if (type != HASHRPF) return NULL;
	if ((technique != HASHRP) && (technique != HASHBRP) && (technique != HASHBBRP)) return NULL;

	StringDictionaryHASHRPF *dict = new StringDictionaryHASHRPF();
	dict->type = technique;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);

	dict->rp = RePair::load(in);
	dict->hash = Hash::load(in, technique);

	return dict;
}

StringDictionaryHASHRPF::~StringDictionaryHASHRPF()
{
	delete rp;
	delete hash;
}

