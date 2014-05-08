/* IteratorDictStringHTFC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings in a Plain Front-Coding representation
 * compressed with Hu-Tucker.
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

#ifndef _ITERATORDICTSTRINGHTFC_H
#define _ITERATORDICTSTRINGHTFC_H

#include <string.h>

#include <iostream>
using namespace std;

#include "../utils/Coder/Codeword.h"
#include "../utils/Coder/DecodingTable.h"
#include "../utils/LogSequence.h"
#include "../utils/Utils.h"
#include "../utils/VByte.h"

class IteratorDictStringHTFC : public IteratorDictString
{
	public:
		/** HTFC Iterator Constructor designed for scanning a Hu-Tucker
		    Front-Coding representation.
		    @param table: decoding table for the Hu-Tucker code.
		    @param codewords: Hu-Tucker codewords.
		    @param ptr: pointer to the sequence of strings.
		    @param blStrings: positional index to the strings.
		    @param bucket: first bucket to be traversed.
		    @param offset: number of initial strings to be initially
		      discarded.
		    @param bucketsize: general bucketsize value used for 
		      obtaining the Plain Front-Coding representation.
 		    @param scanneable: bucket size.
		    @param maxlength: largest string length.
		    @param maxcomplength: largest compressed string length.
		*/
	        IteratorDictStringHTFC(DecodingTable *table, Codeword *codewords,
				       uchar* ptr, 
				       LogSequence *blStrings, size_t bucket, 
				       uint offset, uint bucketsize, 
				       size_t scanneable, uint maxlength, uint maxcomplength)
		{ 
			this->table = table;
			this->codewords = codewords;
			this->k = table->getK();

			this->blStrings = blStrings;
			this->bucket = bucket;
			this->pos = offset;
			this->bucketsize = bucketsize;

			this->scanneable = scanneable;
			this->maxlength = maxlength;
			this->maxcomplength = maxcomplength;
			this->processed = 0;

			this->ptr = ptr;

			{
				this->chunk.c_chunk = 0;
				this->chunk.c_valid = 0;
				this->chunk.b_ptr = ptr+blStrings->getField(bucket);
				this->chunk.b_remain = maxcomplength;
				this->chunk.str = new uchar[2*maxlength+k];
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
		~IteratorDictStringHTFC() 
		{
			delete [] chunk.str;
		}

	protected:
		DecodingTable *table;	//! Decoding table for Hu-Tucker
		Codeword *codewords;	//! Hu-Tucker codewords
		uint k;			//! Chunk size used in the table
		uint maxcomplength;	//! Largest compressed string length

		LogSequence *blStrings;	//! Positional index to the strings representation
		size_t bucket;		//! Bucket currently traversed
		uint pos;		//! Internal position in the bucket
		uint bucketsize;	//! Size of the current bucket

		uchar *ptr;		//! Pointer to the first global position

		ChunkScan chunk;	//! Chunk scanning structure

		inline void decodeHeader()
		{
			chunk.strLen = 0; chunk.advanced = 0; chunk.extracted = 1;
			chunk.c_chunk = 0; chunk.c_valid = 0;chunk.b_remain = maxcomplength;

			// Variables used for adjusting purposes
			uint plen = 0;
			uint pvalid = 0;
			uchar *pptr = chunk.b_ptr;

			while (true)
			{
				if (table->processChunk(&chunk)) break;

				plen = chunk.strLen;
				pvalid = chunk.c_valid;
				pptr = chunk.b_ptr;
			}

			uint bits = 0;

			for (uint i=1; i<=chunk.strLen-plen; i++)
			{
				uchar c = chunk.str[chunk.strLen-i];
				bits += codewords[c].bits;
			}

			chunk.c_valid = 8*(chunk.b_ptr-pptr) - bits + pvalid;
			chunk.b_ptr = chunk.b_ptr - (chunk.c_valid/8);

			bucket++;
			chunk.b_remain = ptr+blStrings->getField(bucket)-chunk.b_ptr;
			chunk.advanced = 0; chunk.extracted = 0;
			chunk.c_chunk = 0; chunk.c_valid = 0;
		}

		inline void decodeNextString()
		{
			chunk.extracted = 0;

			uint prevLen = chunk.strLen;
			uint nextLen = 0;
			bool end = false;

			// Checking if any char has been extracted in advance
			if (chunk.advanced != 0)
			{
				// Checking if a full string is encoded in these advanced chars
				chunk.str[prevLen+chunk.advanced] = 0;
				nextLen = strlen((char*)(chunk.str+prevLen));

				if ((nextLen < chunk.advanced) && (nextLen > 0))
				{
					uint read = prevLen+VByte::decode(&(chunk.strLen), chunk.str+prevLen);
					uint extracted = prevLen+nextLen;

					for (uint i=read; i<=extracted; i++)
					{
						chunk.str[chunk.strLen] = chunk.str[i];
						chunk.strLen++;
					}

					nextLen++;

					if (nextLen != chunk.advanced)
					{
						uint xadv = chunk.advanced-nextLen;
						extracted++;

						for (uint i=0; i<xadv; i++) chunk.str[chunk.strLen+i] = chunk.str[extracted+i];

						chunk.advanced = xadv;
					}
					else chunk.advanced = 0;

					return;
				}
				else
				{
					chunk.strLen = prevLen+chunk.advanced;
					chunk.extracted = chunk.advanced;
				}
			}

			// Extracts, at least, the two first bytes because represent the VByte
			// encoding of the prefix length
			while ((chunk.strLen-prevLen) < 2) end = table->processChunk(&chunk);

			// Appends the extracted chars before the common prefix
			uint extracted = chunk.strLen-prevLen;
			uint read = VByte::decode(&(chunk.strLen), chunk.str+prevLen);

			for (uint i=read; i<extracted; i++)
			{
				chunk.str[chunk.strLen] = chunk.str[prevLen+i];
				chunk.strLen++;
			}

			if (end && (chunk.advanced > 0))
			{
				// Copying advanced chars
				for (uint i=0; i<chunk.advanced; i++)
					chunk.str[chunk.strLen+i] = chunk.str[prevLen+extracted+i];
			}

			// Extracts the remaining suffix
			while (!end) end = table->processChunk(&chunk);
		}
};

#endif  

