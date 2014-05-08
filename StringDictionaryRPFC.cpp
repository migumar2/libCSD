/* StringDictionaryRPFC.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a compressed StringDictionaryRPFC combining Re-Pair
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
 *   Francisco Claude:  	fclaude@recoded.cl
 *   Rodrigo Canovas:  		rcanovas@student.unimelb.edu.au
 *   Miguel A. Martinez-Prieto:	migumar2@infor.uva.es
 */


#include "StringDictionaryRPFC.h"

StringDictionaryRPFC::StringDictionaryRPFC()
{
	this->type = RPFC;
	this->elements = 0;
	this->maxlength = 0;

	this->buckets = 0;
	this->bucketsize = 0;

	this->bytesStrings = 0;
}

StringDictionaryRPFC::StringDictionaryRPFC(IteratorDictString *it, uint bucketsize)
{

	this->type = RPFC;

	if (bucketsize < 2)
	{
		cerr << "[WARNING] The bucketsize value must be greater than 1. ";
		cerr << "The dictionary is built using buckets of size 2" << endl;
		this->bucketsize = 2;
	}
	else this->bucketsize = bucketsize;

	// 1) Bulding the Front-Coding representation
	StringDictionaryPFC *dict = new StringDictionaryPFC(it, this->bucketsize);
	this->maxlength = dict->maxlength;
	this->elements = dict->elements;
	this->buckets = dict->buckets;


	// 2) Obtaining the char frequencies and building the Hu-Tucker tree
	//    and the Re-Pair encoding of the internal strings.
	vector< vector<uchar> > headers(dict->buckets+1);
	size_t reservedInts = elements;
	int* rpdict = new int[reservedInts];

	size_t pbeg = 0, pend = 0;
	uint bucket = 1; uint buckets = dict->buckets;
	size_t ptrpdict = 0;

	uint ends = 0;

	while (bucket <= buckets)
	{
		{
			// Extracting headers

			pbeg = dict->blStrings->getField(bucket);
			pend = dict->blStrings->getField(bucket+1);

			for (; dict->textStrings[pbeg] != 0; pbeg++)
				(headers[bucket]).push_back(dict->textStrings[pbeg]);
			(headers[bucket]).push_back(0);

			pbeg++; bucket++;
		}

		{
			// Extracting the internal strings for Re-Pair compression

			if ((ptrpdict+(size_t)(bucketsize*maxlength)) > reservedInts)
				reservedInts = Reallocate(&rpdict, reservedInts);

			// Stores the last position with 0 to avoid confusions with 0 values encoding VBytes.
			uint zero = pbeg-1;


			for (; pbeg < pend; pbeg++)
			{
				int c = (int)(dict->textStrings[pbeg]);

				if ((c != 0) || ((c == 0) && (pbeg == zero+1))) rpdict[ptrpdict] = c;
				else
				{
					zero = pbeg; ends++;

					rpdict[ptrpdict] = 255;
					ptrpdict++;
					rpdict[ptrpdict] = 0;
				}

				ptrpdict++;
			}
		}
	}

	delete dict;

	// Obtaining the Re-Pair encoding
	rp = new RePair(rpdict, ptrpdict, 255);
	bitsrp = rp->getBits();

	vector<size_t> intStrings;		// Encoded internal strings
	vector<size_t> beginnings(buckets+1);	// Bucket beginnings

	size_t ibytes = 0;
	uint io = 0, strings = 0;
	beginnings[0] = 0;
	uint zero = 0;

	while (io<ptrpdict)
	{
		if (rpdict[io] >= 0)
		{
			if ((rpdict[io] == 0) && (io > zero+1))
			{
				zero = io;
				strings++; io++;

				if ((strings % (bucketsize-1)) == 0)
					beginnings[strings/(bucketsize-1)] = ibytes;
			}
			else
			{
				intStrings.push_back(rpdict[io]);
				io++; ibytes++;
			}
		}
		else
		{
			if (io < ptrpdict) io = -(rpdict[io]+1);
		}
	}

	beginnings[1+strings/(bucketsize-1)] = ibytes;
	delete [] rpdict;

	// 3) Compressing the dictionary
	{
		vector<size_t> xblStrings;
		size_t ptrB = 0, ptrE = 0; uint offset = 0, bytes = 0;
		uchar *tmp = new uchar[4*maxlength];

		size_t reservedStrings = MEMALLOC*bucketsize;
		textStrings = new uchar[reservedStrings];
		bytesStrings = 0; textStrings[bytesStrings] = 0;

		xblStrings.push_back(bytesStrings);	

		for (bucket=1; bucket<=buckets; bucket++)
		{
			// Checking the available space in textStrings and 
			// realloc if required
			while ((bytesStrings+(bucketsize*1000)) > reservedStrings)
				reservedStrings = Reallocate(&textStrings, reservedStrings);

			bytes = 0; tmp[bytes] = 0; offset = 0;
			xblStrings.push_back(bytesStrings);
			ptrB = 0;

			do
			{
				// Copying the header
				textStrings[bytesStrings] = headers[bucket][ptrB];
				bytesStrings++; ptrB++;
			}
			while (headers[bucket][ptrB-1] != '\0');

			// Updating the ptr value to the beginning of the corresponding internal string
			ptrB = beginnings[bucket-1];
			ptrE = beginnings[bucket]-1;

			// Processing the internal strings
			offset = 0;
			textStrings[bytesStrings] = 0;

			for (; ptrB <= ptrE; ptrB++)
				bytesStrings += encodeSymbol(intStrings[ptrB], &(textStrings[bytesStrings]), &offset);

			if (offset > 0) bytesStrings++;
		}

		delete [] tmp;

		// bytesStrings++;
		xblStrings.push_back(bytesStrings+1);
		blStrings = new LogSequence(&xblStrings, bits(bytesStrings+1));
	}
}


uint 
StringDictionaryRPFC::locate(uchar *str, uint strLen)
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
				uint offset=0;
				int cmp=0;

				// Processing the first internal string
				sharedPrev = decodeString(decoded, &decLen, &ptr, &offset);
				cmp=longestCommonPrefix(decoded+sharedCurr, str+sharedCurr, decLen-sharedCurr, &sharedCurr);

				if (cmp != 0)
				{
					for (uint i=2; i<scanneable; i++)
					{
						// TODO: Esta operación se podría dividir en dos, leer primero
						// el VByte y si todo esta OK, seguir decodificando.
						// MIRAR lo que equivalente en PFC
						sharedPrev = decodeString(decoded, &decLen, &ptr, &offset);

						if  (sharedPrev < sharedCurr) break;

						cmp=longestCommonPrefix(decoded+sharedCurr, str+sharedCurr, decLen-sharedCurr, &sharedCurr);

						if (cmp == 0)
						{
							id = ((idbucket-1)*bucketsize)+i+1;
							break;
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
StringDictionaryRPFC::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uint idbucket = 1+((id-1)/bucketsize);
		uint pos = ((id-1)%bucketsize);

		uchar *decoded; uint decLen;
		uchar *ptr = getHeader(idbucket, &decoded, &decLen);

		if (pos > 0)
		{
			uint offset = 0;
			for (uint i=1; i<=pos; i++)
				decodeString(decoded, &decLen, &ptr, &offset);
			*strLen = decLen-1;
		}
		else *strLen = decLen;

		return decoded;
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

IteratorDictID*
StringDictionaryRPFC::locatePrefix(uchar *str, uint strLen)
{
	size_t leftBucket = 1, rightBucket = buckets;
	size_t leftID = 0, rightID = 0;
	uint offset=0;

	// Locating the candidate buckets for the prefix
	locateBoundaryBuckets(str, strLen, &leftBucket, &rightBucket);

	if (leftBucket > NORESULT)
	{
		uchar *decoded; uint decLen;
		uchar *ptr = getHeader(leftBucket, &decoded, &decLen);

		uint scanneable = bucketsize;
		if ((leftBucket == buckets) && ((elements % bucketsize) != 0))
			scanneable = (elements % bucketsize);

		if (leftBucket == rightBucket)
		{
			// All candidate results are in the same bucket
			offset = 0;
			leftID = searchPrefix(&ptr, scanneable, decoded, &decLen, str, strLen, &offset);

			// No strings use the required prefix
			if (leftID == NORESULT)
				return new IteratorDictIDContiguous(NORESULT, NORESULT);
			else
			{
				rightID = leftID+searchDistinctPrefix(ptr, scanneable-leftID+1, decoded, &decLen, str, strLen, &offset)-1;

				leftID += (leftBucket-1)*bucketsize;
				rightID += (rightBucket-1)*bucketsize;
			}
		}
		else
		{
			// All prefixes exceed (possibly) a single bucket
			{
				// Searching the left limit
				offset = 0;
				leftID = searchPrefix(&ptr, scanneable, decoded, &decLen, str, strLen, &offset);

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

				offset = 0;
				rightID = 	searchDistinctPrefix(ptr, scanneable, decoded, &decLen, str, strLen, &offset);
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

	return NULL;
}

IteratorDictID*
StringDictionaryRPFC::locateSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint 
StringDictionaryRPFC::locateRank(uint rank)
{
	return rank;
}

IteratorDictString*
StringDictionaryRPFC::extractPrefix(uchar *str, uint strLen)
{
	IteratorDictIDContiguous *it = (IteratorDictIDContiguous*)locatePrefix(str, strLen);
	size_t left = it->getLeftLimit();

	if (left != NORESULT)
	{
		// Positioning the LEFT Limit
		uint leftbucket = 1+((left-1)/bucketsize);
		uint leftpos = ((left-1)%bucketsize);

		// Positioning the RIGHT Limit
		size_t right = it->getRightLimit();
		delete it;

		size_t ptrS = blStrings->getField(leftbucket);

		return new IteratorDictStringRPFC(rp, bitsrp, textStrings+ptrS, leftpos, bucketsize, right-left+1, maxlength);
	}
	else return NULL;
}

IteratorDictString*
StringDictionaryRPFC::extractSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring extraction" << endl;
	return 0; 
}

uchar *
StringDictionaryRPFC::extractRank(uint rank, uint *strLen)
{
	return extract(rank, strLen);
}

IteratorDictString*
StringDictionaryRPFC::extractTable()
{
	return new IteratorDictStringRPFC(rp, bitsrp, textStrings, 0, bucketsize, elements, maxlength);
}

size_t 
StringDictionaryRPFC::getSize()
{
	return bytesStrings*sizeof(uchar)+blStrings->getSize()+rp->getSize()+sizeof(StringDictionaryRPFC);
}

void 
StringDictionaryRPFC::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	saveValue<uint32_t>(out, buckets);
	saveValue<uint32_t>(out, bucketsize);


	saveValue<uint64_t>(out, bytesStrings);
	saveValue<uchar>(out, textStrings, bytesStrings);
	blStrings->save(out);

	saveValue<uint32_t>(out, bitsrp);
	rp->save(out);
}

StringDictionary*
StringDictionaryRPFC::load(ifstream &in)
{
	size_t type = loadValue<uint32_t>(in);
	if(type != RPFC) return NULL;

	StringDictionaryRPFC *dict = new StringDictionaryRPFC();

	dict->type = RPFC;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);
	dict->buckets = loadValue<uint32_t>(in);
	dict->bucketsize = loadValue<uint32_t>(in);

	dict->bytesStrings = loadValue<uint64_t>(in);
	dict->textStrings = loadValue<uchar>(in, dict->bytesStrings);
	dict->blStrings = new LogSequence(in);

	dict->bitsrp = loadValue<uint32_t>(in);
	dict->rp = RePair::loadNoSeq(in);
	
	return dict;
}


inline uchar*
StringDictionaryRPFC::getHeader(size_t idbucket, uchar **str, uint *strLen)
{
	uchar *ptr = textStrings+blStrings->getField(idbucket);
	*strLen = strlen((char*)ptr);

	*str = new uchar[maxlength];
	strncpy((char*)*str, (char*)ptr, *strLen+1);

	return ptr+(*strLen)+1;
}

bool 
StringDictionaryRPFC::locateBucket(uchar *str, size_t *idbucket)
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
StringDictionaryRPFC::locateBoundaryBuckets(uchar *str, uint strLen, size_t *left, size_t *right)
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
StringDictionaryRPFC::searchPrefix(uchar **ptr, uint scanneable, uchar *decoded, uint *decLen, uchar *str, uint strLen, uint *offset)
{
	uint id=NORESULT;

	uint sharedCurr=0, sharedPrev=0;
	int cmp=0; (*decLen)++;

	uint i=1;
	while (true)
	{
		cmp=longestCommonPrefix(decoded+sharedCurr, str+sharedCurr, *decLen-sharedCurr-1, &sharedCurr);

		if (sharedCurr==strLen) { id = i; break; }
		else
		{
			if ((cmp > 0) || (i == scanneable)) break;

			sharedPrev = decodeString(decoded, decLen, ptr, offset);
			i++;

			if  (sharedPrev < sharedCurr) break;	
		}
	}

	return id;
}


uint
StringDictionaryRPFC::searchDistinctPrefix(uchar *ptr, uint scanneable, uchar *decoded, uint *decLen, uchar *str, uint strLen, uint *offset)
{
	uint id = 1;

	for (id=1; id<scanneable; id++) 
		if (decodeString(decoded, decLen, &ptr, offset) < strLen) break;

	return id;
}

inline uint
StringDictionaryRPFC::encodeSymbol(uint symbol, uchar *text, uint *offset)
{
	uint processed = 0;
	uint bytes = 0;

	while ((bitsrp-processed) >= (8-(*offset)))
	{
		uchar code = ((symbol << (W-bitsrp+processed)) >> (W-8+(*offset)));
		text[bytes] |= code;

		processed += 8-(*offset);
		*offset = 0; bytes++;
		text[bytes] = 0;
	}

	// Encoding the remaining bits
	if (bitsrp > processed)
	{
		uchar code = ((symbol << (W-bitsrp+processed)) >> (W-8+(*offset)));
		text[bytes] |= code;

		*offset += bitsrp-processed;
	}

	return bytes;
}

inline uint
StringDictionaryRPFC::decodeSymbol(uint *symbol, uchar *ptr, uint *offset)
{
	*symbol = 0;

	uint processed = 0;
	uint bytes = 0;

	while ((bitsrp-processed) >= (8-(*offset)))
	{
		*symbol = (*symbol << (8-(*offset))) | (*(ptr+bytes) & mask(8-(*offset)));

		processed += 8-(*offset);
		bytes++; *offset = 0;
	}

	if (bitsrp > processed)
	{
		*offset = bitsrp-processed;
		*symbol = (*symbol << (bitsrp-processed)) | (*(ptr+bytes) >> (8-*offset));
	}

	return bytes;
}

inline uint
StringDictionaryRPFC::decodeString(uchar *str, uint *strLen, uchar **ptr, uint *offset)
{
	uchar *vb = new uchar[maxlength];
	uint read = 0;

	uint rule;

	// The VByte is firstly extracted
	while (read < 2)
	{
		*ptr += decodeSymbol(&rule, *ptr, offset);

		if (rule >= rp->terminals) read += rp->expandRule(rule-rp->terminals, vb+read);
		else
		{
			vb[read] = (uchar)rule;
			read++;
		}
	}

	uint shared;
	uint advanced = VByte::decode(&shared, vb);
	*strLen = shared;

	for (uint i=advanced; i<read; i++)
	{
		str[*strLen] = vb[i];
		(*strLen)++;
	}
	delete [] vb;

	while (str[(*strLen)-1] != rp->maxchar)
	{
		*ptr += decodeSymbol(&rule, *ptr, offset);

		if (rule >= rp->terminals) *strLen += rp->expandRule(rule-rp->terminals, (str+(*strLen)));
		else
		{
			str[*strLen] = (uchar)rule;
			(*strLen)++;
		}
	}

	str[*strLen-1] = 0;


	return shared;
}


StringDictionaryRPFC::~StringDictionaryRPFC()
{
	delete [] textStrings; delete blStrings;
	delete rp;
}

