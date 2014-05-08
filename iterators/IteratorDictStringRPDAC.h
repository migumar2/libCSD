/* IteratorDictStringRPDAC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings in a Re-Pair+DAC representation.
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

#ifndef _ITERATORDICTSTRINGRPDAC_H
#define _ITERATORDICTSTRINGRPDAC_H

#include <string.h>

#include <iostream>
using namespace std;

#include "../utils/DAC_VLS.h"
#include "../utils/LogSequence.h"

class IteratorDictStringRPDAC : public IteratorDictString
{
	public:
		/** RPDAC Iterator Constructor designed for scanning a 
		    RePair+DAC representation.
		    @param G: Re-Pair grammar.
		    @terminals:	number of terminals in the grammar.
		    @param C: Re-Pair sequence.
		    @param offset: number of elements to be discarded.
 		    @param scanneable: bucket size.
		    @param maxlength: largest string length.
		*/
	        IteratorDictStringRPDAC(LogSequence *G, uint64_t terminals, DAC_VLS *C, size_t offset, size_t scanneable, uint maxlength)
		{ 
			this->G = G;
			this->terminals = terminals;
			this->C = C;

			this->scanneable = scanneable;
			this->maxlength = maxlength;
			this->processed = offset;

			this->strCurr = new uchar[2*this->maxlength];
			this->lenCurr = 0;
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
			processed++;

			uint *rules;
			uint len = C->access(processed, &rules);	

			lenCurr = 0;
			
			for (uint j=0; j<len; j++)
			{
				if (rules[j] >= terminals) expandRule(rules[j]-terminals);
				else
				{
					strCurr[lenCurr] = (uchar)rules[j];
					lenCurr++;
				}
			}
			
			strCurr[lenCurr] = '\0';

			*strLen = lenCurr;
			uchar *str = new uchar[lenCurr+1];
			strncpy((char*)str, (char*)strCurr, lenCurr+1);

			delete [] rules;

			return str;
		}


		/** Generic destructor. */
		~IteratorDictStringRPDAC() 
		{
			delete [] strCurr;
		}

	protected:
		LogSequence *G;		//! RePair grammar
		uint64_t terminals;	//! Number of terminals in the grammar
		DAC_VLS *C;		//! RePair sequence

		uchar *strCurr;		//! Current string
		uint lenCurr;		//! Length of 'strCurr'

		void 
		expandRule(uint rule)
		{
			uint left = G->getField(2*rule);
			uint right = G->getField((2*rule)+1);

			if (left >= terminals) expandRule(left-terminals);
			else { strCurr[lenCurr] = (char)left; lenCurr++; }
		
			if (right >= terminals) expandRule(right-terminals);
			else { strCurr[lenCurr] = (char)right; lenCurr++; }
		}
};

#endif  

