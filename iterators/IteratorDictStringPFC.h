/* IteratorDictStringPFC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings in a Plain Front-Coding representation.
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
		    @param ptrI: pointer to the sequence of strings.
		    @param offset: number of internal strings to be initially
		      discarded.
		    @param bucketsize: general bucketsize value used for 
		      obtaining the Plain Front-Coding representation.
		    @param maxlength: largest string length.
 		    @param scanneable: bucket size.
		*/
	   	IteratorDictStringPFC(uchar* ptrS, uint offset, uint bucketsize, size_t scanneable, uint maxlength)
		{ 
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
			this->lenCurr = 0;

			// Updating pointers
			if (pos > 0)
			{
				this->lenCurr = strlen((char*)ptrS);
				strncpy((char*)this->strCurr, (char*)ptrS, this->lenCurr+1);
				this->ptrS += lenCurr+1;

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
				lenCurr = strlen((char*)ptrS);
				strncpy((char*)strCurr, (char*)ptrS, lenCurr+1);

				ptrS += lenCurr+1;
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
		~IteratorDictStringPFC() 
		{
			delete [] strCurr;
		}

	protected:
		uchar* ptrS;		//! Pointer to the sequence of internal strings

		uint pos;		//! Internal position in the bucket
		uint bucketsize;	//! Size of the current bucket

		uchar *strCurr;		//! Current string
		uint lenCurr;		//! Length of 'strCurr'

		uint lenPrefix;		//! Auxiliar storing the length of the common prefix
		uint lenSuffix;		//! Auxiliar storing the length of the remaining suffix


		/** Performs internal decoding operations for the next 
		    string. */
		inline void decodeNext()
		{
			ptrS += VByte::decode(&lenPrefix, ptrS);
			lenSuffix = strlen((char*)ptrS);

			strncpy((char*)(strCurr+lenPrefix), (char*)ptrS, lenSuffix+1);
			ptrS += lenSuffix+1;
			lenCurr = lenPrefix+lenSuffix;
		}		

};

#endif  

