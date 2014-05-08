/* IteratorDictIDXBWDuplicates.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning IDs from a substring search in a XBW.
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

#ifndef _ITERATORDICTIDXBWDUPLICATES_H
#define _ITERATORDICTIDXBWDUPLICATES_H

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

#include "../XBW/XBW.h"

class IteratorDictIDXBWDuplicates : public IteratorDictID
{
	public:
		/** ID Iterator Constructor for streams of contiguous elements.
	    	@param left: the first element ID (left limit)
	    	@param right: the last element ID (right limit)
	    	@param xbw: the XBW
		 */
		IteratorDictIDXBWDuplicates(size_t left, size_t right, XBW *xbw)
		{
			this->leftLimit = left;
			this->rightLimit = right;
			this->xbw = xbw;

			this->scanneable = right+1;
			this->processed = left;

			queue.push_back(processed);

			while (hasNext()) results.push_back(advance());
			sort(results.begin(), results.end());

			this->scanneable = results.size();
			this->processed = 0;

			results.push_back(0);
		}

		size_t next()
		{
			size_t next = results[processed];

			// Jumping repeated results
			do { processed++; } while (results[processed-1] == results[processed]);

			return next;
		}

	protected:
		size_t leftLimit;	// Left stream limit
		size_t rightLimit;	// Right stream limit

		XBW *xbw;		// The XBW

		vector<uint> queue;	// Vector storing data about non-processed nodes
		vector<uint> results;	// Vector storing results


		/** Extracts the next ID in the stream. 
	    	@returns the next ID.
		 */
		inline size_t advance()
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
};

#endif  
