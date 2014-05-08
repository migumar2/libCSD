/* IteratorDictStringXBW.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning strings (continuous range) in a XBW representation.
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

#ifndef _ITERATORDICTSTRINGXBW_H
#define _ITERATORDICTSTRINGXBW_H


#include <iostream>
#include <vector>
using namespace std;

#include "../XBW/XBW.h"

class IteratorDictStringXBW : public IteratorDictString
{
	public:
		/** ID Iterator Constructor for streams of contiguous elements.
		@param prefix: the requested prefix
		@param prefixLen: the prefix len
	    	@param left: the first element ID (left limit)
	    	@param right: the last element ID (right limit)
	    	@param xbw: the XBW
	    	@param maxlength: largest string length.
		 */
		IteratorDictStringXBW(uchar *prefix, uint prefixLen, size_t left, size_t right, XBW *xbw, uint maxlength)
		{
			this->strLen = prefixLen;
			this->leftLimit = left;
			this->rightLimit = right;
			this->xbw = xbw;

			this->maxlength = maxlength;
			this->scanneable = right+1;
			this->processed = left;

			this->str = new uchar[maxlength+1];

			if (prefixLen > 0)
				strncpy((char*)this->str, (char*)prefix, this->strLen);
			else
				this->str[0] = 0;

			queue.push_back(processed);
		}

		/** Checks for non-processed strings in the stream. 
		    @returns if remains non-processed strings. 
		*/
		bool hasNext() { return processed<scanneable; };

		/** Extracts the next ID in the stream. 
	    	@returns the next ID.
		 */
		unsigned char* next(uint *str_length)
		{
			while (xbw->maxLabel != xbw->alpha->access(queue[0]))
			{
				uint left, right;
				xbw->getChildren(queue[0], &left, &right);
				queue.erase(queue.begin(), queue.begin()+1);

				for (uint j=left; j<=right; j++) queue.push_back(j);
			}

			idToStr(queue[0], 0);
			uchar *next = new uchar[maxlength+1];
			strcpy((char*)next, (char*)str);
			queue.erase(queue.begin(), queue.begin()+1);

			if (queue.size() == 0)
			{
				processed++;
				queue.push_back(processed);
			}

			*str_length = pos-1;

			return next;
		}

		/** Obtains the left limit of the stream. That is, the ID of
	    	the first element in the stream.
	    	@returns the left limit.
		 */
		size_t getLeftLimit() { return leftLimit; }

		/** Obtains the right limit of the stream. That is, the ID of
	    	the last element in the stream.
	    	@returns the right limit.
		 */
		size_t getRightLimit() { return rightLimit; }

		/** Generic destructor. */
		~IteratorDictStringXBW() { delete [] str; }

	protected:
		uchar *str;		// Auxiliar string storing the requested prefix
	 	uint strLen;		// The prefix len
		size_t leftLimit;	// Left stream limit
		size_t rightLimit;	// Right stream limit

		XBW *xbw;		// The XBW

		vector<uint> queue;	// Vector storing data about non-processed nodes
		uint pos;		// Auxiliar pos

		void idToStr(uint id, uint cnt)
		{
			if(id == processed)
			{
				pos = strLen;
				if (cnt > 0) str[pos] = xbw->unmap[xbw->alpha->access(id)];
				else str[pos] = 0;

				cnt--; pos++;
				return;
			}

			cnt++;
			idToStr(xbw->getParent(id), cnt);

			if (cnt > 1) str[pos] = xbw->unmap[xbw->alpha->access(id)];
			else str[pos] = 0;

			pos++;
		}
};

#endif  
