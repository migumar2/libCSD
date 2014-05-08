/* IteratorDictStringRPFC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings in a Plain Front-Coding representation
 * compressed with Re-Pair (internal strings).
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

#ifndef _ITERATORDICTSTRINGRPFC_H
#define _ITERATORDICTSTRINGRPFC_H

#include <string.h>

#include <iostream>
using namespace std;

#include "../utils/LogSequence.h"
#include "../utils/Utils.h"
#include "../utils/VByte.h"
#include "../RePair/RePair.h"

class IteratorDictStringRPFC : public IteratorDictString
{
	public:
		/** RPFC Iterator Constructor designed for scanning a Plain
		    Front-Coding representation with RePair.
		    @param rp: the RePair structure used for encoding.
		    @param bitsrp: number of bits used for encoding Re-Pair symbols.
		    @param ptr: pointer to the sequence of strings.
		    @param blStrings: poitional index to the strings.
		    @param offset: number of initial strings to be initially
		      discarded.
		    @param bucketsize: general bucketsize value used for 
		      obtaining the Plain Front-Coding representation.
 		    @param scanneable: number of elements to be iterated.
		    @param maxlength: largest string length.
		*/
		IteratorDictStringRPFC(RePair *rp, uint bitsrp,
				uchar* ptr,
				uint offset, uint bucketsize,
				size_t scanneable, uint maxlength)
		{ 

			this->ptr = ptr;
			this->pos = offset;
			this->bucketsize = bucketsize;

			this->scanneable = scanneable;
			this->maxlength = maxlength;
			this->processed = 0;

			this->rp = rp;
			this->bitsrp = bitsrp;
			this->offset = 0;

			this->lenPrefix = 0;

			// Setting up the iterator
			this->strCurr = new uchar[this->maxlength];
			this->lenCurr = 0;

			// Updating pointers
			if (pos > 0)
			{
				this->lenCurr = strlen((char*)ptr);
				strncpy((char*)this->strCurr, (char*)ptr, this->lenCurr+1);
				this->ptr += lenCurr+1;

				for (uint i=1; i<pos; i++) { decodeNext(); }
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
			if ((pos % bucketsize) == 0)
			{
				if (offset != 0) ptr++;
				offset = 0;

				lenCurr = strlen((char*)ptr);
				strncpy((char*)strCurr, (char*)ptr, lenCurr+1);

				ptr += lenCurr+1;
				pos = 0;
			}
			else decodeNext();

			*strLen = lenCurr;
			uchar *str = new uchar[lenCurr+1];
			strncpy((char*)str, (char*)strCurr, lenCurr+1);

			processed++;
			pos++;

			return str;
		}

		/** Generic destructor. */
		~IteratorDictStringRPFC()
		{
			delete [] strCurr;
		}

	protected:
		uchar *ptr;		//! Pointer to the first global position

		uint pos;		//! Internal position in the bucket
		uint bucketsize;	//! Size of the current bucket

		uchar *strCurr;		//! Current string
		uint lenCurr;		//! Length of 'strCurr'

		uint lenPrefix;		//! Auxiliar storing the length of the common prefix

		RePair *rp;		//! The RePair representation
		uint bitsrp;		//! Number of bits used for encoding Re-Pair symbols

		uint offset;		//! Offset value within the current processed byte

		inline uint
		decodeSymbol()
		{
			uint symbol = 0;
			uint bitsproc= 0;

			while ((bitsrp-bitsproc) >= (8-offset))
			{
				symbol = (symbol << (8-offset)) | (*ptr & mask(8-offset));

				bitsproc += 8-offset;
				ptr++; offset = 0;
			}

			if (bitsrp > bitsproc)
			{
				offset = bitsrp-bitsproc;
				symbol = (symbol << (bitsrp-bitsproc)) | (*ptr >> (8-offset));
			}

			return symbol;
		}

		inline
		void decodeNext()
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

			uint advanced = VByte::decode(&lenPrefix, vb);
			lenCurr = lenPrefix;

			for (uint i=advanced; i<read; i++)
			{
				strCurr[lenCurr] = vb[i];
				lenCurr++;
			}
			delete [] vb;

			while (strCurr[lenCurr-1] != rp->maxchar)
			{
				rule = decodeSymbol();

				if (rule >= rp->terminals) lenCurr += rp->expandRule(rule-rp->terminals, (strCurr+lenCurr));
				else
				{
					strCurr[lenCurr] = (uchar)rule;
					lenCurr++;
				}
			}

			strCurr[lenCurr-1] = 0;
		}
};

#endif  

