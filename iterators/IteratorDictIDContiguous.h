/* IteratorDictIDContiguous.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning streams of contiguous IDs.
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

#ifndef _ITERATORDICTIDCONTIGUOUS_H
#define _ITERATORDICTIDCONTIGUOUS_H


#include <iostream>
using namespace std;

class IteratorDictIDContiguous : public IteratorDictID
{
	public:
		/** ID Iterator Constructor for streams of contiguous elements.
		    @param left: the first element ID (left limit)
		    @param right: the last element ID (right limit)
		*/
		IteratorDictIDContiguous(size_t left, size_t right)
		{
			this->leftLimit = left;
			this->rightLimit = right;
			this->scanneable = right;

			this->processed = left-1;
		}

		/** Extracts the next ID in the stream. 
		    @returns the next ID.
		*/
	    	size_t next() { return ++processed; }

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

		/** Generic destructor */
		~IteratorDictIDContiguous() {}

	protected:
		size_t leftLimit;	// Left stream limit
		size_t rightLimit;	// Right stream limit
};

#endif  
