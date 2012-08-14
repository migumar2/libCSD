/* IteratorDictStringPlain.h
 * Copyright (C) 2012, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning a plain array of strings delimited by '\0' symbols.
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

#ifndef _ITERATORDICTSTRINGPLAIN_H
#define _ITERATORDICTSTRINGPLAIN_H

#include <string.h>

#include <iostream>
using namespace std;

class IteratorDictStringPlain : public IteratorDictString 
{
	public:
		/** Constructor for the Plain Iterator: 
		    @arr: the array of strings.
		    @ptrI: stream size (in bytes).
		*/
	        IteratorDictStringPlain(uchar *arr, size_t scanneable) 
		{ 
			this->arr = arr; 
			this->scanneable = scanneable; 
			this->pos = 0;

			this->processed = 0;
		}

		/** Extracts the next string in the stream. 
		    @param strLen pointer to the string length.
		    @returns the next string.
		*/
		unsigned char* next(uint *str_length) 
		{
			processed++;

			size_t aux = pos;
			*str_length = strlen((char*)(&arr[aux]));
			pos += (*str_length)+1;

			return &arr[aux];
		}

		/** Checks for non-processed strings in the stream. 
		    @returns if remains non-processed strings. 
		*/
	        bool hasNext() 
		{
			return pos<scanneable; 
		}

		/** Generic destructor. */
		~IteratorDictStringPlain() 
		{
			delete [] arr; 
		}

	protected:
	        uchar *arr;		// The array of strings
		size_t pos;		// Pointer to the current position in the scanning
};

#endif  
