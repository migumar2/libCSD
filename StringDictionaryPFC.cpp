/* StringDictionaryPFC.cpp
 * Copyright (C) 2012, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a compressed StringDictionaryPFC combining Huffman
 * and Hu-Tucker compression over a dictionary of strings encoded with 
 * FrontCoding.
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
 *   Francisco Claude:  	fclaude@cs.uwaterloo.ca
 *   Rodrigo Canovas:  		rcanovas@student.unimelb.edu.au
 *   Miguel A. Martinez-Prieto:	migumar2@infor.uva.es
 */


#include "StringDictionaryPFC.h"

StringDictionaryPFC::StringDictionaryPFC()
{
	this->type = PFC;
	this->elements = 0;
	this->maxlength = 0;

	this->buckets = 0;
	this->bucketsize = 0;
	this->bytesHeaders = 0;
	this->bytesStrings = 0;
}

StringDictionaryPFC::StringDictionaryPFC(IteratorDictString *it, uint bucketsize)
{
	cerr << endl;
	cerr << " --- Building Plain Front Coding Dictionary with bucketsize " << this->bucketsize << " ---" << endl;
	cerr << endl;

	this->type = PFC;
	this->elements = 0;
	this->maxlength = 0;

	this->buckets = 0;
	this->bucketsize = bucketsize;
	this->bytesHeaders = 0;
	this->bytesStrings = 0;

	uchar *strCurrent=NULL, *strPrev=NULL;
	uint lenCurrent=0, lenPrev=0;

	// Variables for headers management
	size_t reservedHeaders = 32768;
	textHeaders = new uchar[reservedHeaders];
	vector<size_t> xblHeaders;

	// Variables for strings management
	size_t reservedStrings = reservedHeaders*bucketsize;
	textStrings = new uchar[reservedStrings];
	vector<size_t> xblStrings;

	{
		xblHeaders.push_back(bytesHeaders);
		xblStrings.push_back(bytesStrings);
	}

	while (it->hasNext())
	{
		strCurrent = it->next(&lenCurrent);
		if (lenCurrent >= maxlength) maxlength = lenCurrent+1;

		if ((elements % bucketsize) == 0)
		{
			// First string in the current bucket!
			// ===================================

			// Updating positional indexes
			xblHeaders.push_back(bytesHeaders);
			xblStrings.push_back(bytesStrings);

			buckets++;

			// Checking the available space in headers and realloc
			// if required
			while ((bytesHeaders+(2*lenCurrent)) > reservedHeaders)
				reservedHeaders = Reallocate(&textHeaders, reservedHeaders);

			// The string is explicitly copied
			strcpy((char*)(textHeaders+bytesHeaders), (char*)strCurrent);
			bytesHeaders += lenCurrent;

			textHeaders[bytesHeaders] = '\0';
			bytesHeaders++;		
		}
		else
		{
			// Regular string
			// ==============

			// Checking the available space in strings and realloc 
			// if required
			while ((bytesStrings+(2*lenCurrent)) > reservedStrings)
				reservedStrings = Reallocate(&textStrings, reservedStrings);

			// Obtaining the long common prefix: lcp
			uint lcp = 0;
			longestCommonPrefix(strPrev, strCurrent, lenPrev, &lcp);
			// The lcp value is encoded (VByte)
			bytesStrings += VByte::encode(lcp, textStrings+bytesStrings);
			// The remaining suffix is explicitly copied
			strncpy((char*)(textStrings+bytesStrings), (char*)strCurrent+lcp, lenCurrent-lcp);
                        bytesStrings += lenCurrent-lcp;

			textStrings[bytesStrings] = '\0';
			bytesStrings++;
		}


		// New string processed
		elements++;		
		strPrev = strCurrent;
		lenPrev = lenCurrent;
	}

	// Obtaining the positional indexes
	blHeaders = new Array(xblHeaders, bits(bytesHeaders));
	blStrings = new Array(xblStrings, bits(bytesStrings));
}

uint 
StringDictionaryPFC::locate(uchar *str, size_t strLen)
{
	// Locating the candidate bucket for the string
	size_t idbucket;
	bool cmp = locateBucket(str, &idbucket);

	// The string is the header of the bucket
        if (cmp) return ((idbucket-1)*bucketsize)+1;
        else
	{
		// The string is previous to any other one in the dictionary
		if (idbucket == NORESULT) return NORESULT;
		// The bucket is sequentially scanned to find the string
		else
		{
			IteratorDictStringBucketPFC it = createBucketIterator(idbucket);
			uint id = it.search(str, strLen);

			if (id != NORESULT) return ((idbucket-1)*bucketsize)+id;
			else return NORESULT;
		}
        }
}

uchar *
StringDictionaryPFC::extract(size_t id, size_t *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uint idbucket = 1+((id-1)/bucketsize);
		uint pos = ((id-1)%bucketsize);

		size_t ptrH = blHeaders->getField(idbucket);
		uchar *ptr = textHeaders+ptrH;

		if (pos == 0)
		{
			*strLen = strlen((char*)ptr);
			uchar *str = new uchar[*strLen+1];
			strncpy((char*)str, (char*)ptr, *strLen+1);

			return str;
		}
		else
		{
			size_t ptrS = blStrings->getField(idbucket);
			IteratorDictStringBucketPFC it(ptr, textStrings+ptrS, bucketsize, maxlength);
			return it.retrieve(pos, strLen);
		}
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

inline IteratorDictStringBucketPFC 
StringDictionaryPFC::createBucketIterator(size_t idbucket)
{
	size_t ptrH = blHeaders->getField(idbucket);
	size_t ptrS = blStrings->getField(idbucket);

	uint scanneable = bucketsize;
	if ((idbucket == buckets) && ((elements%bucketsize) != 0))
		scanneable = (elements%bucketsize);

	return IteratorDictStringBucketPFC(textHeaders+ptrH, textStrings+ptrS, scanneable, maxlength);
}

IteratorDictID*
StringDictionaryPFC::locatePrefix(uchar *str, uint strLen)
{
	size_t leftBucket = 1, rightBucket = buckets;
	size_t leftID = 0, rightID = 0;

	// Locating the candidate buckets for the prefix
	locateBoundaryBuckets(str, strLen, &leftBucket, &rightBucket);

	if (leftBucket > NORESULT)
	{
		if (leftBucket == rightBucket)
		{
			// All candidate results are in the same bucket
			IteratorDictStringBucketPFC it = createBucketIterator(leftBucket);
			leftID = it.searchPrefix(str, strLen);

			// No strings use the required prefix
			if (leftID == NORESULT) 
				return new IteratorDictIDContiguous(NORESULT, NORESULT);
			else
				rightID = it.searchDistinctPrefix(str, strLen);

			leftID += (leftBucket-1)*bucketsize;
			rightID += (rightBucket-1)*bucketsize;
		}
		else
		{
			// All prefixes exceeds (possibly) a single bucket
			{
				// Searching the left limit
				IteratorDictStringBucketPFC it = createBucketIterator(leftBucket);
				leftID = it.searchPrefix(str, strLen);

				// The first prefix is the next bucket header
				if (leftID == NORESULT) leftID = leftBucket*bucketsize+1;
				// The first prefix is an internal string of the leftBucket
				else leftID += (leftBucket-1)*bucketsize;
			}

			{
				// Searching the right limit
				IteratorDictStringBucketPFC it = createBucketIterator(rightBucket);
				rightID = it.searchDistinctPrefix(str, strLen);

				rightID += (rightBucket-1)*bucketsize;
			}
		}

		return new IteratorDictIDContiguous(leftID, rightID);
	}
	else
	{
		// No strings use the required prefix
		return new IteratorDictIDContiguous(NORESULT, NORESULT);
	}
}


IteratorDictID*
StringDictionaryPFC::locateSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint 
StringDictionaryPFC::locateRank(uint rank)
{
	return rank;
}

IteratorDictString*
StringDictionaryPFC::extractPrefix(uchar *str, uint strLen)
{
	IteratorDictIDContiguous *it = (IteratorDictIDContiguous*)locatePrefix(str, strLen);

	// Positioning the LEFT Limit
	size_t left = it->getLeftLimit();
	uint leftbucket = 1+((left-1)/bucketsize);
	uint leftpos = ((left-1)%bucketsize);

	// Positioning the RIGHT Limit
	size_t right = it->getRightLimit();

	delete it;

	size_t ptrH = blHeaders->getField(leftbucket);
	size_t ptrS = blStrings->getField(leftbucket);

	return new IteratorDictStringPFC(textHeaders+ptrH, textStrings+ptrS, leftpos, bucketsize, right-left+1, maxlength);
}

IteratorDictString*
StringDictionaryPFC::extractSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring extraction" << endl;
	return 0; 
}

uchar *
StringDictionaryPFC::extractRank(uint rank, uint *strLen)
{
	return extract(rank, strLen);
}

IteratorDictString*
StringDictionaryPFC::extractTable()
{
	size_t ptrH = blHeaders->getField(1);
	size_t ptrS = blStrings->getField(1);

	return new IteratorDictStringPFC(textHeaders+ptrH, textStrings+ptrS, 0, bucketsize, elements, maxlength);
}

size_t 
StringDictionaryPFC::getSize()
{
	return 8*((bytesHeaders+bytesStrings)*sizeof(uchar)+blHeaders->getSize()+blStrings->getSize()+sizeof(StringDictionaryPFC));
}

bool 
StringDictionaryPFC::locateBucket(uchar *str, size_t *idbucket)
{
	size_t left = 1, right = buckets, center = 0;
        int cmp = 0;

	while (left <= right)
        {
		center = (left+right)/2;
                cmp = strcmp((char*)(textHeaders+blHeaders->getField(center)), (char*)str);

		// The string is in any preceding bucket
		if (cmp > 0) right = center-1;
		// The string is in any subsequent bucket
                else if (cmp < 0) left = center+1;
		// The string is the first one in the c-th bucket
		else { *idbucket = center; return true; }
	}

        // c is the candidate bucket for the string
        if (cmp < 0) *idbucket = center;
        // c-1 is the candidate bucket for the string
        else *idbucket = center-1;

	return false;
}

void
StringDictionaryPFC::locateBoundaryBuckets(uchar *str, uint strLen, size_t *left, size_t *right)
{
	size_t center = 0;
	int cmp = 0;

	while (*left <= *right)
        {
		center = (*left+*right)/2;
                cmp = strncmp((char*)(textHeaders+blHeaders->getField(center)), (char*)str, strLen);

		if (cmp > 0) *right = center-1;
                else if (cmp < 0) *left = center+1;
                else break;
	}

	if (cmp != 0)
	{
		// All prefixes are in the same block
        	if (cmp < 0) { *left = center; *right = center;}
	        else { *left = center-1; *right = center-1; }
		return;
	}

	if (center > 1)
	{
		// Looking for the left boundary
		uint ll = *left, lr = center-1, lc;

		while (ll <= lr)
		{
			lc = (ll+lr)/2;
			cmp = strncmp((char*)(textHeaders+blHeaders->getField(lc)), (char*)str, strLen);

			if (cmp == 0) lr = lc-1;
			else ll = lc+1;
		}	

		if (lr > NORESULT) *left = lr;
		else *left = 1;
	}

	if (center < buckets)
	{
		// Looking for the right boundary
		uint rl = center, rr = *right+1, rc;

		while (rl < (rr-1))
		{
			rc = (rl+rr)/2;
			cmp = strncmp((char*)(textHeaders+blHeaders->getField(rc)), (char*)str, strLen);

			if (cmp == 0) rl = rc;
		        else rr = rc;
		}

		*right = rl;
	}
}


void 
StringDictionaryPFC::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	saveValue<uint32_t>(out, buckets);
	saveValue<uint32_t>(out, bucketsize);
	saveValue<uint64_t>(out, bytesHeaders);
	saveValue<uchar>(out, textHeaders, bytesHeaders);
	blHeaders->save(out);

	saveValue<uint64_t>(out, bytesStrings);
	saveValue<uchar>(out, textStrings, bytesStrings);
	blStrings->save(out);	
}


StringDictionary*
StringDictionaryPFC::load(ifstream &in)
{
	size_t type = loadValue<uint32_t>(in);
	if(type != PFC) return NULL;

	StringDictionaryPFC *dict = new StringDictionaryPFC();

	dict->type = PFC;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);
	dict->buckets = loadValue<uint32_t>(in);
	dict->bucketsize = loadValue<uint32_t>(in);
	dict->bytesHeaders = loadValue<uint64_t>(in);
	dict->textHeaders = loadValue<uchar>(in, dict->bytesHeaders);
	dict->blHeaders = new Array(in);

	dict->bytesStrings = loadValue<uint64_t>(in);
	dict->textStrings = loadValue<uchar>(in, dict->bytesStrings);
	dict->blStrings = new Array(in);

	return dict;
}

StringDictionaryPFC::~StringDictionaryPFC()
{
	delete [] textHeaders; delete blHeaders;
	delete [] textStrings; delete blStrings;
}

