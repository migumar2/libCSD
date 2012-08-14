/* IteratorDictStringPFC.h
 * Copyright (C) 2012, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings in a Plain Front-Coding representation.
 * 
 * This library is free software; you can redistrCurribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distrCurributed in the hope that it will be useful,
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

#ifndef _ITERATORDICTSTRINGPFC_H
#define _ITERATORDICTSTRINGPFC_H

#include <string.h>

#include <iostream>
using namespace std;

#include "../utils/Utils.h"
#include "../utils/VByte.h"

class IteratorDictStringPFC : public IteratorDictString
{
	public:
		/** PFC Iterator Constructor designed for scanning a Plain 
		    Front-Coding representation.
		    @param ptrH: pointer to the sequence of headers.
		    @param ptrI: pointer to the sequence of internal strings.
		    @param offset: number of internal strings to be initially
		      discarded.
		    @param bucketsize: general bucketsize value used for 
		      obtaining the Plain Front-Coding representation.
		    @param maxlength: largest string length.
 		    @param scanneable: bucket size.
		*/
	        IteratorDictStringPFC(uchar* ptrH, uchar* ptrS, uint offset, uint bucketsize, size_t scanneable, uint maxlength)
		{ 
			this->ptrH = ptrH;
			this->ptrS = ptrS;
			this->pos = offset;
			this->bucketsize = bucketsize;

			this->scanneable = scanneable;
			this->maxlength = maxlength;
			this->processed = 0;

			this->lenPrefix = 0;
			this->lenSuffix = 0;

			// Setting up the iterator
			this->strCurr = new uchar[this->maxlength];
			this->lenCurr = strlen((char*)this->ptrH);
			strncpy((char*)this->strCurr, (char*)this->ptrH, this->lenCurr+1);

			// Updating pointers
			this->ptrH += this->lenCurr+1;
			for (uint i=0; i<offset; i++) decodeNext();
		}

		/** Checks for non-processed strings in the stream. 
		    @returns if remains non-processed strings. 
		*/
	        bool hasNext() 
		{
			return processed<scanneable; 
		}

		/** Extracts the next string in the stream. 
		    @param strLen: pointer to the string length.
		    @returns the next string.
		*/
		unsigned char* next(uint *strLen) 
		{
			*strLen = lenCurr;
			uchar *str = new uchar[lenCurr+1];
			strncpy((char*)str, (char*)strCurr, lenCurr+1);

			processed++;
			pos++;

			if (hasNext())
			{
				// Checking the bucket end
				if ((pos % bucketsize) == 0)
				{
					lenCurr = strlen((char*)ptrH);
					strncpy((char*)strCurr, (char*)ptrH, lenCurr+1);

					ptrH += lenCurr+1;
					pos = 0;	
				}
				else decodeNext();
			}

			return str;
		}

		/** Performs internal decoding operations for the next string. 
		    @param str: buffer for decoding the string. 	
		*/
		inline void decodeNext()
		{
			ptrS += VByte::decode(&lenPrefix, ptrS);
			lenSuffix = strlen((char*)ptrS);

			strncpy((char*)(strCurr+lenPrefix), (char*)ptrS, lenSuffix+1);
			ptrS += lenSuffix+1;
			lenCurr = lenPrefix+lenSuffix;
		}		

		/** Generic destructor. */
		~IteratorDictStringPFC() 
		{
			delete [] strCurr;
		}

	protected:
		uchar* ptrH;		// Pointer to the sequence of headers
		uchar* ptrS;		// Pointer to the sequence of internal strings

		uint pos;		// Internal position in the bucket
		uint bucketsize;	// Size of the current bucket

		uchar *strCurr;		// Current string
		uint lenCurr;		// Length of 'strCurr'

		uint lenPrefix;		// Auxiliar storing the length of the common prefix
		uint lenSuffix;		// Auxiliar storing the length of the remaining suffix
};

#endif  

