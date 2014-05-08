/* HashBBdh.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Class implementating the HashBB dictionary from:
 *
 *   ==========================================================================
 *     "Compressed String Dictionaries"
 *     Nieves R. Brisaboa, Rodrigo Canovas, Francisco Claude,
 *     Miguel A. Martinez-Prieto and Gonzalo Navarro.
 *     10th Symposium on Experimental Algorithms (SEA'2011), p.136-147, 2011.
 *   ==========================================================================
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

#ifndef HASHBBDH_H
#define HASHBBDH_H

#include "Hash.h"
#include <BitSequence.h>

using namespace cds_static;

	
	class HashBBdh: public Hash{

		public:
			/* Generic constructor. */
			HashBBdh();

			/* Create a hash table of size the given size.
			 * @tsize: the hash table size.
			 */
			HashBBdh(uint tsize);

			/* Searches for a key in the hash table.
			 * @w: the key to be searched.
			 * @len: the key length.
			 * @returns the position in which the key is stored (or -1 if it does not exist).
			 */
			size_t search(uchar *w, size_t len);

			/* Retrieves the position in which the i-th key is stored.
			 * @i: key position in the hash table.
			 * @returns the position in the compressed sequence.
			 */
			size_t getValue(size_t i);

			/* Retrieves the i-th position in the hash table.
			 * @i: position in the hash table.
			 * @returns the position in the compressed sequence.
			 */
			size_t getValuePos(size_t i);

			/* Obtains the hash table size.
			 * @returns the hash table size.
			 */
			size_t getSize();

			/** Loads a hash from a file*/
			static HashBBdh * load(ifstream & fp);

			virtual ~HashBBdh();

		protected:
			BitSequence *offsets;

	};
#endif
