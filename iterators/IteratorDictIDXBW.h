/* IteratorDictIDXBW.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning ID in a XBW representation.
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

#ifndef _ITERATORDICTIDXBW_H
#define _ITERATORDICTIDXBW_H


#include <iostream>
#include <vector>
using namespace std;

#include "../XBW/XBW.h"

class IteratorDictIDXBW : public IteratorDictID
{
	public:
		/** ID Iterator Constructor for streams of contiguous elements.
	    	@param left: the first element ID (left limit)
	    	@param right: the last element ID (right limit)
	    	@param xbw: the XBW
		 */
		IteratorDictIDXBW(size_t left, size_t right, XBW *xbw)
		{
			this->leftLimit = left;
			this->rightLimit = right;
			this->xbw = xbw;

			this->scanneable = right+1;
			this->processed = left;

			queue.push_back(processed);
		}

		/** Extracts the next ID in the stream. 
	    	@returns the next ID.
		 */
		size_t next()
		{
			while (xbw->maxLabel != xbw->alpha->access(queue[0]))
			{
				uint left, right;
				xbw->getChildren(queue[0], &left, &right);
				queue.erase(queue.begin(), queue.begin()+1);

				for (uint j=left; j<=right; j++) queue.push_back(j);
			}

			uint next = xbw->alpha->rank(xbw->maxLabel, queue[0]);
			queue.erase(queue.begin(), queue.begin()+1);

			if (queue.size() == 0)
			{
				processed++;
				queue.push_back(processed);
			}

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

	protected:
		size_t leftLimit;	// Left stream limit
		size_t rightLimit;	// Right stream limit

		XBW *xbw;		// The XBW

		vector<uint> queue;	// Vector storing data about non-processed nodes
};

#endif  
