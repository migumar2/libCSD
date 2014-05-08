/* Hash.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Abstract class for implementing compressed hash dictionaries.
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

#ifndef HASH_H
#define HASH_H

#include <iostream>
using namespace std;

#include <BitSequence.h>
#include <libcdsBasics.h>
using namespace cds_static;
using namespace cds_utils;

#include "HashUtils.h"
#include "../utils/LogSequence.h"
#include "../utils/Utils.h"


class Hash
{
		public:
			/* Inserts a new string in the hash table.
			 * @w: the string to be inserted.
			 * @len: the string length.
			 * @offset: position of the string in the compressed sequence.
			 * @returns the position in which the string is inserted in the hash table.
			 */
			size_t insert(uchar *w, size_t len, size_t offset=0);

			/* Updates the offset position for the string in hash[pos]
			 * @pos: position in which is stored the string to be updated.
			 * @offset: new offset position.
			 */
			void setOffset(size_t pos, size_t offset);

			/* Post-processes the hash table for storing it.
			 *  @length: length of the compressed sequence in which the keys are stored.
			 */
			void finish(size_t length);

			/* Set a pointer to the compressed sequence.
			 * @seq: pointer the compressed sequence.
			 */
			void setData(uchar *data);

			/* Abstract method for searching a key in the hash table.
			 * @w: the key to be searched.
			 * @len: the key length.
			 * @returns the position in which the key is stored (or -1 if it does not exist).
			 */
			virtual size_t search(uchar *w, size_t len)=0;

			/* Abstract method which retrieves the position in which the i-th key is stored.
			 * @i: key position in the hash table.
			 * @returns the position in the compressed sequence.
			 */
			virtual size_t getValue(size_t i)=0;

			/* Abstract method which retrieves the i-th position in the hash table.
			 * @i: position in the hash table.
			 * @returns the position in the compressed sequence.
			 */
			virtual size_t getValuePos(size_t i)=0;

			/* Abstract method which obtains the hash table size.
			 * @returns the hash table size.
			 */
			virtual size_t getSize()=0;

			/** Saves the hash to a file */
			void save(ofstream & fp);

			/** Loads a hash from a file*/
			static Hash * load(ifstream & fp, int r);

			virtual ~Hash() {};

		protected:
			size_t tsize; 		// size of the hashtable
			size_t n; 		// number of elements into the hashtable

			LogSequence *hash;	// hashtable
			BitSequence *b_ht;	// bitsequence pointing string beginnings

			uchar *data; 		// Reference to the Huffman-compressed sequence

			size_t *hashtable;	// auxiliar table used for encoding purposes
			size_t *enclength;	// auxiliar structure storing the lengths of the encoded strings

			/*compare the information in data from offset with w*/
			virtual int scmp(size_t offset, uchar *w, size_t len);

		friend class StringDictionaryHASHRPF;
};

#include "Hashdh.h"
#include "HashBdh.h"
#include "HashBBdh.h"

#endif  /* _HASHUFF_H */

