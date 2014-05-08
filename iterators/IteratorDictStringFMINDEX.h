/* IteratorDictStringFMINDEX.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings (continuous range) in a FMIndex Dictionary.
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

#ifndef _ITERATORDICTSTRINGFMINDEXC_H
#define _ITERATORDICTSTRINGFMINDEXC_H

#include <string.h>

#include <iostream>
using namespace std;

#include "../FMIndex/SSA.h"

class IteratorDictStringFMINDEX : public IteratorDictString
{
	public:
		/** Iterator Constructor designed for scanning a FMIndex dictionary.
		    @fm_index: the self-index
		    @first: the first ID to be retrieved
		    @scanneable: the last to be retrieved
		    @last: the last valid id in the dictionary
		    @maxlength: the length of the largest string in the dictionary
		*/
	    	IteratorDictStringFMINDEX(SSA *fm_index, uint first, size_t scanneable, uint last, uint maxlength)
		{ 
		    	this->fm_index = fm_index;
		    	this->maxlength = maxlength;
		    	this->scanneable = scanneable;
		    	this->last = last;
		    	this->processed = first;
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
			size_t id;

			if (processed == last) id=2;
			else id = processed+3;
			processed++;

			return fm_index->extract_id(id, strLen, maxlength);
		}

		/** Generic destructor. */
		~IteratorDictStringFMINDEX() { }

	protected:
		SSA *fm_index;	// Self-index
		uint maxlength;	// String maxlength
		size_t last;	// Last id in the dictionary
};

#endif  
