/* IteratorDictStringRPHTFC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings in a Plain Front-Coding representation
 * compressed with Hu-Tucker and Re-Pair.
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

#ifndef _ITERATORDICTSTRINGRPHTFC_H
#define _ITERATORDICTSTRINGRPHTFC_H

#include <string.h>

#include <iostream>
using namespace std;

#include "../utils/Coder/DecodingTable.h"
#include "../utils/LogSequence.h"
#include "../utils/Utils.h"
#include "../utils/VByte.h"
#include "../RePair/RePair.h"

class IteratorDictStringRPHTFC : public IteratorDictString
{
	public:
		/** HHTFC Iterator Constructor designed for scanning a Hu-Tucker
		    Front-Coding representation.
		    @param tableHT: decoding table for the Hu-Tucker code.
		    @param codewordsHT: HuTucker codewords.
		    @param rp: the RePair structure used for encoding.
			@param bitsrp: number of bits used for encoding Re-Pair symbols.
		    @param ptr: pointer to the sequence of strings.
		    @param blStrings: poitional index to the strings.
		    @param bucket: first bucket to be traversed.
		    @param offset: number of initial strings to be initially
		      discarded.
		    @param bucketsize: general bucketsize value used for 
		      obtaining the Plain Front-Coding representation.
 		    @param scanneable: number of elements to be iterated.
		    @param maxlength: largest string length.
		    @param maxcomplength: largest compressed string length.
		*/
		IteratorDictStringRPHTFC(DecodingTable *tableHT, Codeword *codewordsHT,
				RePair *rp, uint bitsrp,
				uchar* ptr,
				LogSequence *blStrings, size_t bucket,
				uint offset, uint bucketsize,
				size_t scanneable, uint maxlength, uint maxcomplength)
		{ 
			this->tableHT = tableHT;
			this->codewordsHT = codewordsHT;
			this->kHT = tableHT->getK();

			this->rp = rp;
			this->bitsrp = bitsrp;

			this->blStrings = blStrings;
			this->bucket = bucket;
			this->pos = offset;
			this->bucketsize = bucketsize;

			this->scanneable = scanneable;
			this->maxlength = maxlength;
			this->maxcomplength = maxcomplength;
			this->processed = 0;
			this->offset = 0;

			this->ptr = ptr;

			{
				this->chunk.c_chunk = 0;
				this->chunk.c_valid = 0;
				this->chunk.b_ptr = ptr+blStrings->getField(bucket);
				this->chunk.b_remain = maxcomplength;
				this->chunk.str = new uchar[2*maxlength+kHT];
				this->chunk.strLen = 0;
				this->chunk.advanced = 0;
				this->chunk.extracted = 0;
			}

			// Jumping the corresponding strings
			if (pos > 0)
			{
				decodeHeader();
				for (uint i=1; i<pos; i++) decodeNextString();
			}
		}

		/** Checks for non-processed strings in the stream. 
		    @returns if remains non-processed strings. 
		*/
		bool hasNext()
		{
			return processed<scanneable; 
		}

		/** Extracts the next string in the stream. Note that a 
		    previous checking about next existence must be peformed 
		    using the 'hasNext' method.
		    @param strLen: pointer to the string length.
		    @returns the next string.
		*/
		unsigned char* next(uint *strLen) 
		{
			// Checking the bucket end
			if ((pos % bucketsize) == 0) decodeHeader();
			else decodeNextString();

			*strLen = chunk.strLen-1;
			uchar *str = new uchar[chunk.strLen+1];
			strncpy((char*)str, (char*)chunk.str, chunk.strLen);

			processed++;
			pos++;

			return str;
		}

		/** Generic destructor. */
		~IteratorDictStringRPHTFC()
		{
			delete [] chunk.str;
		}

	protected:
		DecodingTable *tableHT;	//! Decoding table for Hu-Tucker
		uint kHT;		//! Chunk size used in the tableHT
		Codeword *codewordsHT;	//! HuTucker codewords
		uint maxcomplength;	//! Largest compressed string length (HT)

		RePair *rp;		//! The RePair representation
		uint bitsrp;		//! Number of bits used for encoding Re-Pair symbols

		LogSequence *blStrings;	//! Positional index to the strings representation
		size_t bucket;		//! Bucket currently traversed
		uint pos;		//! Internal position in the bucket
		uint bucketsize;	//! Size of the current bucket

		uchar *ptr;		//! Pointer to the first global position

		ChunkScan chunk;	//! Chunk scanning structure
		uint offset;		//! Offset value within the current processed byte

		inline void decodeHeader()
		{
			chunk.strLen = 0;  chunk.advanced = 0; chunk.extracted = 1;
			chunk.c_chunk = 0; chunk.c_valid = 0;chunk.b_remain = maxcomplength;
			chunk.b_ptr = ptr+blStrings->getField(bucket); bucket++;
			offset = 0;

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
		}

		inline uint
		decodeSymbol()
		{
			uint symbol = 0;
			uint bitsproc= 0;

			while ((bitsrp-bitsproc) >= (8-offset))
			{
				symbol = (symbol << (8-offset)) | (*chunk.b_ptr & mask(8-offset));

				bitsproc += 8-offset;
				chunk.b_ptr++; offset = 0;
			}

			if (bitsrp > bitsproc)
			{
				offset = bitsrp-bitsproc;
				symbol = (symbol << (bitsrp-bitsproc)) | (*chunk.b_ptr >> (8-offset));
			}

			return symbol;
		}

		inline void decodeNextString()
		{
			uchar *vb = new uchar[maxlength];
			uint read = 0;

			uint rule;

			// The VByte is firstly extracted
			while (read < 2)
			{
				rule = decodeSymbol();

				if (rule >= rp->terminals) read += rp->expandRule(rule-rp->terminals, vb+read);
				else
				{
					vb[read] = (uchar)rule;
					read++;
				}
			}

			uint advanced = VByte::decode(&chunk.strLen, vb);

			for (uint i=advanced; i<read; i++)
			{
				chunk.str[chunk.strLen] = vb[i];
				chunk.strLen++;
			}
			delete [] vb;

			while (chunk.str[chunk.strLen-1] != rp->maxchar)
			{
				rule = decodeSymbol();

				if (rule >= rp->terminals) chunk.strLen += rp->expandRule(rule-rp->terminals, (chunk.str+chunk.strLen));
				else
				{
					chunk.str[chunk.strLen] = (uchar)rule;
					chunk.strLen++;
				}
			}

			chunk.str[chunk.strLen-1] = 0;
		}
};

#endif  

