/* StringDictionaryRPDAC.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary combining Re-Pair 
 * compression and Directly Addresable Codes.
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


#include "StringDictionaryRPDAC.h"

StringDictionaryRPDAC::StringDictionaryRPDAC()
{
	this->type = RPDAC;
	this->elements = 0;
	this->maxlength = 0;
}

StringDictionaryRPDAC::StringDictionaryRPDAC(IteratorDictString *it)
{
	this->type = RPDAC;
	this->elements = 0;
	this->maxlength = 0;

	uchar *strCurrent=NULL;
	uint lenCurrent=0;
	size_t processed = 0;

	int *dict = new int[it->size()];

	while (it->hasNext())
	{
		strCurrent = it->next(&lenCurrent);
		if (lenCurrent >= maxlength) maxlength = lenCurrent+1;

		for (uint i=0; i<=lenCurrent; i++) dict[processed+i] = strCurrent[i];
		processed += lenCurrent+1;
		elements++;
	}

	delete it;

	rp = new RePair(dict, processed, 0);

	// Compacting the sequence (a -i value is inserted after the i-th string).
	int *cdict = new int[processed];
	uint io = 0, ic = 0, strings = 0;
	uint maxseq = 0, currentseq = 0;

	cerr << "### Writing seq [BEGIN]" << endl; cerr.flush();
	while (io<processed)
	{
		if (dict[io] >= 0)
		{
			if (dict[io] == 0)
			{
				if (currentseq > maxseq) maxseq = currentseq;

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

	// Building the array for the sequence
	rp->Cdac = new DAC_VLS(cdict, ic-2, bits(rp->rules+rp->terminals), maxseq);

	delete [] cdict;
}

uint 
StringDictionaryRPDAC::locate(uchar *str, uint strLen)
{
	// Binary search comparing s with respect to the corresponding
	// pivot rule.
	size_t left = 1, right = elements, center = 0;
	int cmp;
		
	while (left <= right)
	{			
		center = (left+right)/2;
		cmp = rp->extractStringAndCompareDAC(center, str, strLen);

		// The string precedes the current pivot
		if (cmp > 0) right = center-1;
		// The string follows the current pivot
		else if (cmp < 0) left = center+1;
		// The string is the current pivot
		else return center;
	}
		
	return 0;
}

uchar *
StringDictionaryRPDAC::extract(size_t id, uint *strLen)
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
StringDictionaryRPDAC::locatePrefix(uchar *str, uint strLen)
{
	// Binary search comparing s with respect to the corresponding
	// pivot rule.
	size_t left = 1, right = elements, center = 0;
	int cmp = 0;
		
	while (left <= right)
	{			
		center = (left+right)/2;
		cmp = rp->extractPrefixAndCompareDAC(center, str, strLen);

		// The string precedes the current pivot
		if (cmp > 0) right = center-1;
		// The string follows the current pivot
		else if (cmp < 0) left = center+1;
		// The string is the current pivot
		else break;
	}

	// No strings use the required prefix		
	if (cmp != 0) return new IteratorDictIDContiguous(NORESULT, NORESULT);

	if (center > 1)
	{
		// Looking for the left boundary
		uint ll = left, lr = center-1, lc;

		while (ll <= lr)
		{
			lc = (ll+lr)/2;
			cmp = rp->extractPrefixAndCompareDAC(lc, str, strLen);

			if (cmp == 0) lr = lc-1;
			else ll = lc+1;
		}	

		if (lr > NORESULT) left = lr+1;
		else left = 1;
	}
	else left = center;

	if (center < elements)
	{
		// Looking for the right boundary
		uint rl = center, rr = right+1, rc;

		while (rl < (rr-1))
		{
			rc = (rl+rr)/2;
			cmp = rp->extractPrefixAndCompareDAC(rc, str, strLen);

			if (cmp == 0) rl = rc;
		        else rr = rc;
		}

		right = rl;
	}
	else right = center;

	return new IteratorDictIDContiguous(left, right);
}


IteratorDictID*
StringDictionaryRPDAC::locateSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint 
StringDictionaryRPDAC::locateRank(uint rank)
{
	return rank;
}

IteratorDictString*
StringDictionaryRPDAC::extractPrefix(uchar *str, uint strLen)
{
	IteratorDictIDContiguous *it = (IteratorDictIDContiguous*)locatePrefix(str, strLen);

	size_t offset = it->getLeftLimit()-1;
	size_t scanneable = it->getRightLimit();

	delete it;
	return new IteratorDictStringRPDAC(rp->G, rp->terminals, rp->Cdac, offset, scanneable, maxlength);
}

IteratorDictString*
StringDictionaryRPDAC::extractSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring extraction" << endl;
	return 0; 
}

uchar *
StringDictionaryRPDAC::extractRank(uint rank, uint *strLen)
{
	return extract(rank, strLen);
}

IteratorDictString*
StringDictionaryRPDAC::extractTable()
{
	return new IteratorDictStringRPDAC(rp->G, rp->terminals, rp->Cdac, 0, elements, maxlength);
}

size_t 
StringDictionaryRPDAC::getSize()
{
	return rp->getSize()+sizeof(StringDictionaryRPDAC);
}

void 
StringDictionaryRPDAC::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	rp->save(out, RPDAC);
}


StringDictionary*
StringDictionaryRPDAC::load(ifstream &in)
{
	size_t type = loadValue<uint32_t>(in);
	if(type != RPDAC) return NULL;

	StringDictionaryRPDAC *dict = new StringDictionaryRPDAC();

	dict->type = RPDAC;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);
	dict->rp = RePair::load(in);

	return dict;
}

StringDictionaryRPDAC::~StringDictionaryRPDAC()
{
	delete rp;
}

