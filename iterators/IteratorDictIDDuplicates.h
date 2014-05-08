/* IteratorDictIDDuplicates.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Iterator class for scanning streams of IDs containing duplicates.
 * The IDs must be sorted before scanning!!!
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

#ifndef _ITERATORDICTIDDUPLICATES_H
#define _ITERATORDICTIDDUPLICATES_H


#include <iostream>
using namespace std;

class IteratorDictIDDuplicates : public IteratorDictID
{
	public:
		/** ID Iterator Constructor for streams of duplicated elements.
		    @param ids: the array of ids
		    @param scanneable: the number of ids in the array
		*/
		IteratorDictIDDuplicates(size_t *ids, size_t scanneable)
		{
			this->ids = ids;
			this->scanneable = scanneable;

			this->processed = 0;
		}

		size_t next()
		{
			size_t next = ids[processed];

			// Jumping duplicates results
			do { processed++; } while (ids[processed-1] == ids[processed]);

			return next;
		}


		/** Generic destructor */
		~IteratorDictIDDuplicates() { delete [] ids; }

	protected:
		size_t *ids;	// Array of ids
};

#endif  
