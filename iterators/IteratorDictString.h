/* IteratorDictString.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning streams of strings.
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

#ifndef _ITERATORDICTSTRING_H
#define _ITERATORDICTSTRING_H

#include <string.h>

#include <cassert>
#include <iostream>
using namespace std;


class IteratorDictString
{
	public:
		/** Checks for non-processed strings in the stream. 
		    @returns if remains non-processed strings. 
		*/
		virtual bool hasNext()=0;

		/** Extracts the next string in the stream. 
		    @param strLen pointer to the string length.
		    @returns the next string.
		*/
	   	virtual unsigned char* next(uint *str_length)=0;

		/** Generic destructor. */
		virtual ~IteratorDictString() {} ;

		/** Returns the stream size */
		uint size() { return scanneable; }


	protected:
		size_t processed;	// Number of processed strings
		size_t scanneable;	// Upper limit of the stream
		uint maxlength;		// Largest string length
};

#include "IteratorDictStringPlain.h"
#include "IteratorDictStringVector.h"

#include "IteratorDictStringPFC.h"
#include "IteratorDictStringRPFC.h"
#include "IteratorDictStringHTFC.h"
#include "IteratorDictStringHHTFC.h"
#include "IteratorDictStringRPHTFC.h"
#include "IteratorDictStringRPDAC.h"
#include "IteratorDictStringXBW.h"
#include "IteratorDictStringXBWDuplicates.h"
#include "IteratorDictStringFMINDEX.h"
#include "IteratorDictStringFMINDEXDuplicates.h"


#endif  
