/* StringDictionaryPFC.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary which differentially
 * encodes the strings using (Plain) Front-Coding.
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


#include "StringDictionaryPFC.h"

StringDictionaryPFC::StringDictionaryPFC()
{
	this->type = PFC;
	this->elements = 0;
	this->maxlength = 0;

	this->buckets = 0;
	this->bucketsize = 0;
	this->bytesStrings = 0;
}

StringDictionaryPFC::StringDictionaryPFC(IteratorDictString *it, uint bucketsize)
{
	this->type = PFC;
	this->elements = 0;
	this->maxlength = 0;

	if (bucketsize < 2)
	{
		cerr << "[WARNING] The bucketsize value must be greater than 1. ";
		cerr << "The dictionary is built using buckets of size 2" << endl;
		this->bucketsize = 2;
	}
	else this->bucketsize = bucketsize;

	this->buckets = 0;
	this->bytesStrings = 0;

	// Bulding the Front-Coding representation
	uchar *strCurrent=NULL, *strPrev=NULL;
	uint lenCurrent=0, lenPrev=0;

	// Variables for strings management
	size_t reservedStrings = MEMALLOC*bucketsize;
	textStrings = new uchar[reservedStrings];
	vector<size_t> xblStrings;

	xblStrings.push_back(bytesStrings);

	while (it->hasNext())
	{
		strCurrent = it->next(&lenCurrent);
		if (lenCurrent >= maxlength) maxlength = lenCurrent+1;

		// Checking the available space in textStrings and 
		// realloc if required
		while ((bytesStrings+(2*lenCurrent)) > reservedStrings)
			reservedStrings = Reallocate(&textStrings, reservedStrings);

		if ((elements % bucketsize) == 0)
		{
			// First string in the current bucket!
			// ===================================

			// Updating the positional index
			xblStrings.push_back(bytesStrings);
			buckets++;

			// The string is explicitly copied
			strcpy((char*)(textStrings+bytesStrings), (char*)strCurrent);
			bytesStrings += lenCurrent;
		}
		else
		{
			// Regular string
			// ==============

			// Obtaining the long common prefix: lcp
			uint lcp = 0;
			longestCommonPrefix(strPrev, strCurrent, lenPrev, &lcp);
			// The lcp value is encoded (VByte)
			bytesStrings += VByte::encode(lcp, textStrings+bytesStrings);
			// The remaining suffix is explicitly copied
			strncpy((char*)(textStrings+bytesStrings), (char*)strCurrent+lcp, lenCurrent-lcp);
           		bytesStrings += lenCurrent-lcp;
		}

		textStrings[bytesStrings] = '\0'; 
		bytesStrings++;		

		// New string processed
		elements++;		
		strPrev = strCurrent;
		lenPrev = lenCurrent;
	}

	delete it;

	// Obtaining the positional indexes
	xblStrings.push_back(bytesStrings);
	blStrings = new LogSequence(&xblStrings, bits(bytesStrings));
}

uint 
StringDictionaryPFC::locate(uchar *str, uint strLen)
{
	uint id = NORESULT;

	// Locating the candidate bucket for the string
	size_t idbucket;
	bool cmp = locateBucket(str, &idbucket);

	// The string is the header of the bucket
	if (cmp) return ((idbucket-1)*bucketsize)+1;
	else
	{
		// The string is previous to any other one in the dictionary
		if (idbucket != NORESULT)
		{
			// The bucket is sequentially scanned to find the string
			uchar *decoded; uint decLen;
			uchar *ptr = getHeader(idbucket, &decoded, &decLen);

			uint scanneable = bucketsize;
			if ((idbucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

			if (scanneable > 1)
			{
				uint sharedCurr=0, sharedPrev=0;
				int cmp=0;

				// Processing the first internal string
				ptr += VByte::decode(&sharedPrev, ptr);
				decodeNextString(&ptr, sharedPrev, decoded, &decLen);
				cmp=longestCommonPrefix(decoded+sharedCurr, str+sharedCurr, decLen-sharedCurr+1, &sharedCurr);

				if (cmp != 0)
				{
					for (uint i=2; i<scanneable; i++)
					{
						ptr += VByte::decode(&sharedPrev, ptr);

						if  (sharedPrev < sharedCurr) break;
						decodeNextString(&ptr, sharedPrev, decoded, &decLen);

						if (sharedPrev == sharedCurr)
							cmp=longestCommonPrefix(decoded+sharedCurr, str+sharedCurr, decLen-sharedCurr+1, &sharedCurr);

						if (cmp == 0)
						{
							id = ((idbucket-1)*bucketsize)+i+1;
							delete [] decoded;
							return id;
						}
						else if (cmp > 0) break;
					}
				}
				else id = ((idbucket-1)*bucketsize)+2;
			}

			delete [] decoded;
		}
	}

	return id;
}

uchar *
StringDictionaryPFC::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uint idbucket = 1+((id-1)/bucketsize);
		uint pos = ((id-1)%bucketsize);

		uchar *decoded; uint decLen;
		uchar *ptr = getHeader(idbucket, &decoded, &decLen);
		uint lenPrefix;

		if (pos > 0) for (uint i=1; i<=pos; i++)
		{
			ptr += VByte::decode(&lenPrefix, ptr);
			decodeNextString(&ptr, lenPrefix, decoded, &decLen);
		}

		*strLen = decLen;
		return decoded;
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
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
		uchar *decoded; uint decLen;
		uchar *ptr = getHeader(leftBucket, &decoded, &decLen);

		uint scanneable = bucketsize;
		if ((leftBucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

		if (leftBucket == rightBucket)
		{
			// All candidate results are in the same bucket
			leftID = searchPrefix(&ptr, scanneable, decoded, &decLen, str, strLen);

			// No strings use the required prefix
			if (leftID == NORESULT) 
				return new IteratorDictIDContiguous(NORESULT, NORESULT);
			else
				rightID = leftID+searchDistinctPrefix(ptr, scanneable-leftID, decoded, &decLen, str, strLen)-1;

			leftID += (leftBucket-1)*bucketsize;
			rightID += (rightBucket-1)*bucketsize;
		}
		else
		{
			// All prefixes exceed (possibly) a single bucket
			{
				// Searching the left limit
				leftID = searchPrefix(&ptr, scanneable, decoded, &decLen, str, strLen);

				// The first prefix is the next bucket header
				if (leftID == NORESULT) leftID = leftBucket*bucketsize+1;
				// The first prefix is an internal string of the leftBucket
				else leftID += (leftBucket-1)*bucketsize;
			}

			{
				// Searching the right limit
				delete [] decoded;

				ptr = getHeader(rightBucket, &decoded, &decLen);

				scanneable = bucketsize;
				if ((rightBucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

				rightID = searchDistinctPrefix(ptr, scanneable-1, decoded, &decLen, str, strLen);
				rightID += (rightBucket-1)*bucketsize;
			}
		}

		delete [] decoded;
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
	cerr << "This dictionary does not provide substring location" << endl;
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

	if (it->getLeftLimit() != NORESULT)
	{
		// Positioning the LEFT Limit
		size_t left = it->getLeftLimit();
		uint leftbucket = 1+((left-1)/bucketsize);
		uint leftpos = ((left-1)%bucketsize);

		// Positioning the RIGHT Limit
		size_t right = it->getRightLimit();

		delete it;

		size_t ptrS = blStrings->getField(leftbucket);

		return new IteratorDictStringPFC(textStrings+ptrS, leftpos, bucketsize, right-left+1, maxlength);
	}
	else return NULL;
}

IteratorDictString*
StringDictionaryPFC::extractSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring extraction" << endl;
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
	size_t ptrS = blStrings->getField(1);

	return new IteratorDictStringPFC(textStrings+ptrS, 0, bucketsize, elements, maxlength);
}

size_t 
StringDictionaryPFC::getSize()
{
	return (bytesStrings*sizeof(uchar))+blStrings->getSize()+sizeof(StringDictionaryPFC);
}

void 
StringDictionaryPFC::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	saveValue<uint32_t>(out, buckets);
	saveValue<uint32_t>(out, bucketsize);
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
	dict->bytesStrings = loadValue<uint64_t>(in);
	dict->textStrings = loadValue<uchar>(in, dict->bytesStrings);
	dict->blStrings = new LogSequence(in);

	return dict;
}

inline uchar*
StringDictionaryPFC::getHeader(size_t idbucket, uchar **str, uint *strLen)
{
	uchar *ptr = textStrings+blStrings->getField(idbucket);
	*strLen = strlen((char*)ptr);

	*str = new uchar[maxlength];
	strncpy((char*)*str, (char*)ptr, *strLen+1);

	return ptr+(*strLen)+1;
}

void
StringDictionaryPFC::decodeNextString(uchar **ptr, uint lenPrefix, uchar *str, uint *strLen)
{
	uint lenSuffix;

	lenSuffix = strlen((char*)*ptr);
	strncpy((char*)(str+lenPrefix), (char*)*ptr, lenSuffix+1);

	*ptr += lenSuffix+1;
	*strLen = lenPrefix+lenSuffix;
}

bool 
StringDictionaryPFC::locateBucket(uchar *str, size_t *idbucket)
{
	size_t left = 1, right = buckets, center = 0;
	int cmp = 0;

	while (left <= right)
	{
		center = (left+right)/2;
        	cmp = strcmp((char*)(textStrings+blStrings->getField(center)), (char*)str);

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
		cmp = strncmp((char*)(textStrings+blStrings->getField(center)), (char*)str, strLen);

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
			cmp = strncmp((char*)(textStrings+blStrings->getField(lc)), (char*)str, strLen);

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
			cmp = strncmp((char*)(textStrings+blStrings->getField(rc)), (char*)str, strLen);

			if (cmp == 0) rl = rc;
		        else rr = rc;
		}

		*right = rl;
	}
}

uint
StringDictionaryPFC::searchPrefix(uchar **ptr, uint scanneable, uchar *decoded, uint *decLen, uchar *str, uint strLen)
{
	uint sharedCurr=0, sharedPrev=0;
	int cmp=0; uint id=1;

	while (true)
	{
		cmp=longestCommonPrefix(decoded+sharedCurr, str+sharedCurr, *decLen-sharedCurr, &sharedCurr);

		if (sharedCurr==strLen) break;
		else
		{
			id++;
			if ((cmp > 0) || (id > scanneable)) break;

			*ptr += VByte::decode(&sharedPrev, *ptr);
			if  (sharedPrev < sharedCurr) break;
			decodeNextString(ptr, sharedPrev, decoded, decLen);
		}
	}

	return id;
}

uint
StringDictionaryPFC::searchDistinctPrefix(uchar *ptr, uint scanneable, uchar *decoded, uint *decLen, uchar *str, uint strLen)
{
	uint id = 1;
	uint lenPrefix;

	for (id=1; id<=scanneable; id++)
	{
		ptr += VByte::decode(&lenPrefix, ptr);
		if (lenPrefix < strLen) break;
		decodeNextString(&ptr, lenPrefix, decoded, decLen);
	}

	return id;
}

StringDictionaryPFC::~StringDictionaryPFC()
{
	delete [] textStrings; delete blStrings;
}

