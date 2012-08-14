/* IteratorDictStringBucketPFC.h
 * Copyright (C) 2012, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings in a Plain Front-Coding bucket.
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

#ifndef _ITERATORDICTSTRINGBUCKETPFC_H
#define _ITERATORDICTSTRINGBUCKETPFC_H

#include <string.h>

#include <iostream>
using namespace std;

#include "../utils/Utils.h"
#include "../utils/VByte.h"

class IteratorDictStringBucketPFC : public IteratorDictString
{
	public:
		/** PFC Iterator Constructor designed for scanning a single bucket
		    @param header: pointer to the HEADER string.
		    @param ptrI: pointer to the first internal string.
 		    @param scanneable: bucket size.
		    @param maxlength: largest string length.
		*/
	        IteratorDictStringBucketPFC(uchar* header, uchar* ptrS, size_t scanneable, uint maxlength)
		{ 
			this->ptrS = ptrS;

			this->scanneable = scanneable;
			this->maxlength = maxlength;
			this->processed = 0;

			this->lenPrefix = 0;
			this->lenSuffix = 0;

			// Setting up the iterator
			this->strCurr = new uchar[this->maxlength];
			this->lenCurr = strlen((char*)header);
			strncpy((char*)this->strCurr, (char*)header, this->lenCurr+1);

			this->strPrev = new uchar[this->maxlength];
			this->lenPrev = 0;
			this->strPrev[this->lenPrev] = '\0';
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

			if (hasNext()) decodeNext();
			return str;
		}

		/** Searches the given string and returns its internal ID.
		    @pre { 1) The search operation assumes that strCurr 
			   is previous to the string to be searched. 
		    }
		    @param str: the string to be searched.
		    @param strLen: the string length.
		    @returns the internal ID (of 0 if it is not in the bucket).
		*/
		uint search(uchar *str, uint strLen) 
		{
			uint sharedCurr=0, sharedPrev=0;
			int cmp=0;

			while (true)
			{
				processed++;

				if (!hasNext()) return 0;

				lenPrev = lenCurr;
				strncpy((char*)strPrev, (char*)strCurr, lenCurr+1);
				decodeNext();
				sharedPrev = lenPrefix;
		
				if  (sharedPrev < sharedCurr) return 0;

				cmp=longestCommonPrefix(strCurr+sharedCurr, str+sharedCurr, lenCurr-sharedCurr, &sharedCurr);

				if ((sharedCurr==strLen) && (lenCurr==strLen))
				{
					return processed+1;
				}
				else if (cmp > 0) return 0;
			}
		}

		/** Searches the first string prefixed for the given one.
		    @pre { 1) The search operation assumes that strCurr points 
			   to the header string.
		    }
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns the internal ID (of 0 if it is not in the bucket).
		*/
		uint searchPrefix(uchar *str, uint strLen) 
		{
			assert(processed == 0);

			uint sharedCurr=0, sharedPrev=0;
			int cmp;

			while (true)
			{
				cmp=longestCommonPrefix(strCurr+sharedCurr, str+sharedCurr, lenCurr-sharedCurr, &sharedCurr);

				if (sharedCurr==strLen) return processed+1;
				else
				{
					processed++;

					if ((cmp > 0) || (!hasNext())) return 0;

					lenPrev = lenCurr;
					strncpy((char*)strPrev, (char*)strCurr, lenCurr+1);

					decodeNext();
					sharedPrev = lenPrefix;

					if  (sharedPrev < sharedCurr) return 0;		
				}
			}
		}

		/** Searches the first string non-prefixed for the given one.
		    @pre { 1) The search operation assumes that strCurr points 
			   to a string prefixed by the given one.
		    }
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns the internal ID (of 0 if it is not in the bucket).
		*/
		uint searchDistinctPrefix(uchar *str, uint strLen) 
		{
			processed++;

			while (hasNext())
			{
				decodeNext();

				if (lenPrefix < strLen) break;
				processed++;
			}

			return processed;
		}


		/** Retrieves the id-th string in the bucket.
		    @pre { 1) The retrieval operation assumes that strCurr is 
			   pointed to the header string, so the processed value
		           must be equals to 0.
			   2) The ID required for retrieval must be > 1.
		    }
		    @param id: the internal string ID.
		    @param strLen: the string length.
		    @returns the retrieved string.
		*/
		uchar* retrieve(uint id, uint *strLen) 
		{
			assert(id > 1);

			for (processed=1; processed<=id; processed++) decodeNext();

			*strLen = lenPrefix+lenSuffix;
			uchar *str = new uchar[*strLen+1];
			strncpy((char*)str, (char*)strCurr, *strLen+1);

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
		~IteratorDictStringBucketPFC() 
		{
			delete [] strCurr; delete [] strPrev;
		}

	protected:
		uchar* ptrS;		// Pointer to the first internal string

		uchar *strCurr;		// Current string
		uint lenCurr;		// Length of 'strCurr'

		uchar *strPrev;		// String previously processed
		uint lenPrev;		// Length of 'strPrev'

		uint lenPrefix;		// Auxiliar storing the length of the common prefix
		uint lenSuffix;		// Auxiliar storing the length of the remaining suffix
};

#endif  

