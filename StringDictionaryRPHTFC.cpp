/* StringDictionaryRPHTFC.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a compressed StringDictionaryRPHTFC combining Re-Pair
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


#include "StringDictionaryRPHTFC.h"

StringDictionaryRPHTFC::StringDictionaryRPHTFC()
{
	this->type = RPHTFC;
	this->elements = 0;
	this->maxlength = 0;
	this->maxcomplength = 0;

	this->buckets = 0;
	this->bucketsize = 0;

	this->bytesStrings = 0;
}

StringDictionaryRPHTFC::StringDictionaryRPHTFC(IteratorDictString *it, uint bucketsize)
{
	this->type = RPHTFC;

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
	this->maxcomplength = 0;

	// 2) Obtaining the char frequencies and building the Hu-Tucker tree
	//    and the Re-Pair encoding of the internal strings.
	uint* freqs = new uint[256];

	vector< vector<uchar> > headers(dict->buckets+1);
	size_t reservedInts = elements;
	int* rpdict = new int[reservedInts];

	// Initializing counters
	for (uint i=0; i<256; i++) freqs[i]=1;

	size_t pbeg = 0, pend = 0;
	uint bucket = 1; uint buckets = dict->buckets;
	size_t ptrpdict = 0;

	uint ends = 0;

	while (bucket <= buckets)
	{
		// Recollecting statistics for the headers and extracting them for Hu-Tucker compression
		pbeg = dict->blStrings->getField(bucket);
		pend = dict->blStrings->getField(bucket+1);

		uint i=0;
		for (; dict->textStrings[pbeg] != 0; pbeg++)
		{
			freqs[(int)(dict->textStrings[pbeg])]++;
			(headers[bucket]).push_back(dict->textStrings[pbeg]);
			i++;
		}

		(headers[bucket]).push_back(0);

		freqs[0]++; pbeg++;
		bucket++;

		if ((ptrpdict+(size_t)(bucketsize*maxlength)) > reservedInts)
			reservedInts = Reallocate(&rpdict, reservedInts);

		// Stores the last position with 0 to avoid confusions with 0 values encoding VBytes.
		uint zero = pbeg-1;

		// Extracting the internal strings for Re-Pair compression
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

	// Obtaining the HuTucker code
	HuTucker *ht = new HuTucker(freqs);

	// Initializing the HuTucker builder
	DecodingTableBuilder *builderHT = new DecodingTableBuilder();
	builderHT->initializeFromHuTucker(ht);
	codewordsHT = builderHT->getCodewords();
	delete [] freqs; delete ht;
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

	// 3) Compressing the dictionary and building the decoding table
	{
		vector<size_t> xblStrings;
		size_t ptrB = 0, ptrE = 0; uint offset = 0, bytes = 0;
		uchar *tmp = new uchar[4*maxlength];

		size_t reservedStrings = MEMALLOC*bucketsize;
		textStrings = new uchar[reservedStrings];
		bytesStrings = 0; textStrings[bytesStrings] = 0;

		xblStrings.push_back(bytesStrings);	

		// Auxiliar variables for managing the substrings indexed in 
		// the Decoding Table.
		vector<uchar> textSubstr; vector<ushort> lenSubstr;
		ushort ptrSubstr=0; uint codeSubstr=0;

		coderHT = new StatCoder(codewordsHT);

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
				// Encoding the header in tmp
				uchar symbol = headers[bucket][ptrB];
				bytes += coderHT->encodeSymbol(symbol, &(tmp[bytes]), &offset);
				ptrB++;

				builderHT->insertDecodeableSubstr(symbol, &codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);

				if (bytes > maxcomplength) maxcomplength = bytes;
			}
			while (headers[bucket][ptrB-1] != '\0');

			{
				// Encoding the compressed header length
				if (offset > 0) bytes++;

				memcpy(textStrings+bytesStrings, tmp, bytes);
				bytesStrings += bytes;

				if (bytes > maxcomplength) maxcomplength = bytes;
			}

			// Updating the ptr value to the beginning of the corresponding internal string
			ptrB = beginnings[bucket-1];
			ptrE = beginnings[bucket]-1;

			// Adding an ending decodeable string  (if required)
			if (textSubstr.size() > 0)
			{
				// #######################
				// It is necessary to read up to TABLEBITSO bits for indexing the
				// substring in the DecodingTable

				if (offset > 0)
				{
					// The substring is also padded
					codeSubstr = (codeSubstr << (8-offset));
					ptrSubstr += (8-offset); offset = 0;
				}

				if (ptrSubstr > TABLEBITSO)
				{
					codeSubstr = codeSubstr >> (ptrSubstr - TABLEBITSO);
					ptrSubstr = TABLEBITSO;
				}
				else
				{
					if ((bucket == buckets) && (elements % bucketsize == 0))
					{
						// The last element is directly padded
						codeSubstr = (codeSubstr << (TABLEBITSO-ptrSubstr));
						ptrSubstr = TABLEBITSO;
					}
					else
					{
						// The first symbol encoding the internal strings is enough for padding because it uses, at least, 16 bits
						uint remaining = TABLEBITSO-ptrSubstr;
						uint codeword =  intStrings[ptrB];

						if (remaining < bitsrp)
						{
							codeSubstr = (codeSubstr << remaining) | (codeword >> (bitsrp-remaining));
						}
						else
						{
							codeSubstr = (codeSubstr << bitsrp) | codeword;
							codeSubstr = codeSubstr << (remaining-bitsrp);
						}

						ptrSubstr = TABLEBITSO;
					}
				}
			}

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

		maxcomplength += 4; // The value is increased because advanced readings in decoding...
	}

	tableHT = builderHT->getTable(); delete builderHT;
}


uint 
StringDictionaryRPHTFC::locate(uchar *str, uint strLen)
{
	uint id = NORESULT;

	// Encoding the string
	uint encLen, offset;
	uchar *encoded = coderHT->encodeString(str, strLen+1, &encLen, &offset);

	// Locating the candidate bucket for the string
	size_t idbucket;
	bool cmp = locateBucket(encoded, encLen, &idbucket);
	delete [] encoded;

	// The string is the header of the bucket
	if (cmp) id = ((idbucket-1)*bucketsize)+1;
	else
	{
		// The string is previous to any other one in the dictionary
		if (idbucket != NORESULT)
		{
			// The bucket is sequentially scanned to find the string
			ChunkScan c = decodeHeader(idbucket);

			uint scanneable = bucketsize;
			if ((idbucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

			if (scanneable > 1)
			{
				uint sharedCurr=0, sharedPrev=0;
				uint offset=0;
				int cmp=0;

				// Processing the first internal string
				sharedPrev = decodeString(c.str, &c.strLen, &c.b_ptr, &offset);
				cmp=longestCommonPrefix(c.str+sharedCurr, str+sharedCurr, c.strLen-sharedCurr, &sharedCurr);

				if (cmp != 0)
				{
					for (uint i=2; i<scanneable; i++)
					{
						sharedPrev = decodeString(c.str, &c.strLen, &c.b_ptr, &offset);

						if  (sharedPrev < sharedCurr) break;

						cmp=longestCommonPrefix(c.str+sharedCurr, str+sharedCurr, c.strLen-sharedCurr, &sharedCurr);

						if (cmp==0)
						{
							id = ((idbucket-1)*bucketsize)+i+1;
							break;
						}
						else if (cmp > 0) break;
					}
				}
				else id = ((idbucket-1)*bucketsize)+2;
			}

			delete [] c.str;
		}
	}

	return id;
}

uchar *
StringDictionaryRPHTFC::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uint idbucket = 1+((id-1)/bucketsize);
		uint pos = ((id-1)%bucketsize);

		ChunkScan c = decodeHeader(idbucket);

		if (pos > 0)
		{
			uint offset = 0;
			for (uint i=1; i<=pos; i++)
				decodeString(c.str, &c.strLen, &c.b_ptr, &offset);
		}

		*strLen = c.strLen-1;
		return c.str;
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

IteratorDictID*
StringDictionaryRPHTFC::locatePrefix(uchar *str, uint strLen)
{
	// Encoding the string
	uint encLen, offset=0;
	uchar *encoded = coderHT->encodeString(str, strLen, &encLen, &offset);

	size_t leftBucket = 1, rightBucket = buckets;
	size_t leftID = 0, rightID = 0;

	// Locating the candidate buckets for the prefix
	locateBoundaryBuckets(encoded, encLen, offset, &leftBucket, &rightBucket);

	IteratorDictIDContiguous *itResult;

	if (leftBucket > NORESULT)
	{
		ChunkScan c = decodeHeader(leftBucket);

		uint scanneable = bucketsize;
		if ((leftBucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

		if (leftBucket == rightBucket)
		{
			// All candidate results are in the same bucket
			offset = 0;
			leftID = searchPrefix(&c, scanneable, str, strLen, &offset);

			// No strings use the required prefix
			if (leftID == NORESULT) itResult = new IteratorDictIDContiguous(NORESULT, NORESULT);
			else
			{
				rightID = leftID+searchDistinctPrefix(&c, scanneable-leftID+1, str, strLen, &offset)-1;

				leftID += (leftBucket-1)*bucketsize;
				rightID += (rightBucket-1)*bucketsize;
			}
		}
		else
		{
			// All prefixes exceeds (possibly) a single bucket
			{
				// Searching the left limit
				offset = 0;
				leftID = searchPrefix(&c, scanneable, str, strLen, &offset);

				// The first prefix is the next bucket header
				if (leftID == NORESULT) leftID = leftBucket*bucketsize+1;
				// The first prefix is an internal string of the leftBucket
				else leftID += (leftBucket-1)*bucketsize;

				delete [] c.str;
			}

			{
				// Searching the right limit
				c = decodeHeader(rightBucket);

				scanneable = bucketsize;
				if ((rightBucket == buckets) && ((elements%bucketsize) != 0)) scanneable = (elements%bucketsize);

				offset = 0;
				rightID = searchDistinctPrefix(&c, scanneable, str, strLen, &offset);
				rightID += (rightBucket-1)*bucketsize;
			}
		}

		itResult = new IteratorDictIDContiguous(leftID, rightID);
		delete [] c.str;
	}
	else
	{
		// No strings use the required prefix
		itResult = new IteratorDictIDContiguous(NORESULT, NORESULT);
	}

	delete [] encoded;
	return itResult;
}

IteratorDictID*
StringDictionaryRPHTFC::locateSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint 
StringDictionaryRPHTFC::locateRank(uint rank)
{
	return rank;
}

IteratorDictString*
StringDictionaryRPHTFC::extractPrefix(uchar *str, uint strLen)
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

		return new IteratorDictStringRPHTFC(tableHT, codewordsHT, rp, bitsrp, textStrings, blStrings, leftbucket, leftpos, bucketsize, right-left+1, maxlength, maxcomplength);
	}
	else return NULL;
}

IteratorDictString*
StringDictionaryRPHTFC::extractSubstr(uchar *str, uint strLen)
{
	cerr << "This dictionary does not provide substring extraction" << endl;
	return 0; 
}

uchar *
StringDictionaryRPHTFC::extractRank(uint rank, uint *strLen)
{
	return extract(rank, strLen);
}

IteratorDictString*
StringDictionaryRPHTFC::extractTable()
{
	return new IteratorDictStringRPHTFC(tableHT, codewordsHT, rp, bitsrp, textStrings, blStrings, 1, 0, bucketsize, elements, maxlength, maxcomplength);
}

size_t 
StringDictionaryRPHTFC::getSize()
{
	return bytesStrings*sizeof(uchar)+blStrings->getSize()+256*sizeof(Codeword)+tableHT->getSize()+rp->getSize()+sizeof(StringDictionaryRPHTFC);
}

void 
StringDictionaryRPHTFC::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	saveValue<uint32_t>(out, maxcomplength);
	saveValue<uint32_t>(out, buckets);
	saveValue<uint32_t>(out, bucketsize);

	saveValue<uint64_t>(out, bytesStrings);
	saveValue<uchar>(out, textStrings, bytesStrings);
	blStrings->save(out);

	saveValue<Codeword>(out, codewordsHT, 256);
	tableHT->save(out);

	saveValue<uint32_t>(out, bitsrp);
	rp->save(out);
}

StringDictionary*
StringDictionaryRPHTFC::load(ifstream &in)
{
	size_t type = loadValue<uint32_t>(in);
	if(type != RPHTFC) return NULL;

	StringDictionaryRPHTFC *dict = new StringDictionaryRPHTFC();

	dict->type = RPHTFC;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);
	dict->maxcomplength = loadValue<uint32_t>(in);
	dict->buckets = loadValue<uint32_t>(in);
	dict->bucketsize = loadValue<uint32_t>(in);

	dict->bytesStrings = loadValue<uint64_t>(in);
	dict->textStrings = loadValue<uchar>(in, dict->bytesStrings);
	dict->blStrings = new LogSequence(in);

	dict->codewordsHT = loadValue<Codeword>(in, 256);
	dict->tableHT = DecodingTable::load(in);
	dict->coderHT = new StatCoder(dict->tableHT, dict->codewordsHT);

	dict->bitsrp = loadValue<uint32_t>(in);
	dict->rp = RePair::loadNoSeq(in);
	
	return dict;
}

uchar*
StringDictionaryRPHTFC::getHeader(size_t idbucket)
{
	size_t ptrH = blStrings->getField(idbucket);
	uchar *header = textStrings+ptrH;

	return header;
}

ChunkScan
StringDictionaryRPHTFC::decodeHeader(size_t idbucket)
{
	uchar* ptr = textStrings+blStrings->getField(idbucket);
	ChunkScan chunk = {0, 0, ptr, maxcomplength, new uchar[4*maxlength+tableHT->getK()], 0, 0, 1};

	// Variables used for adjusting purposes
	uint plen = 0;
	uint pvalid = 0;
	uchar *pptr = chunk.b_ptr;

	while (true)
	{
		if (tableHT->processChunk(&chunk)) break;

		plen = chunk.strLen;
		pvalid = chunk.c_valid;
		pptr = chunk.b_ptr;
	}

	uint bits = 0;

	for (uint i=1; i<=chunk.strLen-plen; i++)
	{
		uchar c = chunk.str[chunk.strLen-i];
		bits += codewordsHT[c].bits;
	}

	chunk.c_valid = 8*(chunk.b_ptr-pptr) - bits + pvalid;
	chunk.b_ptr = chunk.b_ptr - (chunk.c_valid/8);

	return chunk;
}

bool 
StringDictionaryRPHTFC::locateBucket(uchar *str, uint strLen, size_t *idbucket)
{
	size_t left = 1, right = buckets, center = 0;
	int cmp = 0;

	uchar *header;

	while (left <= right)
	{
		center = (left+right)/2;
		header = getHeader(center);

		cmp = memcmp(header, str, strLen);
		// The string is in any preceding bucket
		if (cmp > 0) right = center-1;
		// The string is in any subsequent bucket
		else if (cmp < 0) left = center+1;
		else 
		{ 
			// The string is the first one in the c-th bucket
			*idbucket = center;
			return true;
		}
	}

	// c is the candidate bucket for the string
	if (cmp < 0) *idbucket = center;
	// c-1 is the candidate bucket for the string
	else *idbucket = center-1;

	return false;
}

void
StringDictionaryRPHTFC::locateBoundaryBuckets(uchar *str, uint strLen, uint offset, size_t *left, size_t *right)
{
	size_t center = 0;
	int cmp = 0;

	uchar *header = new uchar[4*strLen];

	uchar cmask = (uchar)(~(mask(8) >> offset));

	while (*left <= *right)
	{
		center = (*left+*right)/2;

		memcpy(header, getHeader(center), strLen);
		if (offset != 0) header[strLen-1] = header[strLen-1] & cmask;
		cmp = memcmp(header, str, strLen);

		if (cmp > 0) *right = center-1;
		else if (cmp < 0) *left = center+1;
		else break;
	}

	if (cmp != 0)
	{
		// All prefixes are in the same block
		if (cmp < 0) { *left = center; *right = center;}
		else { *left = center-1; *right = center-1; }

		delete [] header;
		return;
	}

	if (center > 1)
	{
		// Looking for the left boundary
		uint ll = *left, lr = center-1, lc;

		while (ll <= lr)
		{
			lc = (ll+lr)/2;

			memcpy(header, getHeader(lc), strLen);
			if (offset != 0) header[strLen-1] = header[strLen-1] & cmask;
		    	cmp = memcmp(header, str, strLen);

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

			memcpy(header, getHeader(rc), strLen);
			if (offset != 0) header[strLen-1] = header[strLen-1] & cmask;
		    	cmp = memcmp(header, str, strLen);

			if (cmp == 0) rl = rc;
		    	else rr = rc;
		}

		*right = rl;
	}

	delete [] header;
}

uint 
StringDictionaryRPHTFC::searchPrefix(ChunkScan* c, uint scanneable, uchar *str, uint strLen, uint *offset)
{
	uint id = NORESULT;

	uint sharedCurr=0, sharedPrev=0;
	int cmp=0;

	uint i=1;
	while (true)
	{
		cmp=longestCommonPrefix(c->str+sharedCurr, str+sharedCurr, c->strLen-sharedCurr-1, &sharedCurr);

		if (sharedCurr==strLen) { id = i; break; }
		else
		{
			if ((cmp > 0) || (i == scanneable)) break;

			sharedPrev =  decodeString(c->str, &c->strLen, &c->b_ptr, offset);
			i++;

			if  (sharedPrev < sharedCurr) break;	
		}
	}

	return id;
}


uint
StringDictionaryRPHTFC::searchDistinctPrefix(ChunkScan* c, uint scanneable, uchar *str, uint strLen, uint *offset)
{
	uint id = 1;

	for (id=1; id<scanneable; id++) 
		if (decodeString(c->str, &c->strLen, &c->b_ptr, offset) < strLen) break;

	return id;
}

uint
StringDictionaryRPHTFC::encodeSymbol(uint symbol, uchar *text, uint *offset)
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
StringDictionaryRPHTFC::decodeSymbol(uint *symbol, uchar *ptr, uint *offset)
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
StringDictionaryRPHTFC::decodeString(uchar *str, uint *strLen, uchar **ptr, uint *offset)
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

StringDictionaryRPHTFC::~StringDictionaryRPHTFC()
{
	delete [] textStrings; delete blStrings;
	delete coderHT;
	delete [] codewordsHT; delete tableHT;
	delete rp;
}

