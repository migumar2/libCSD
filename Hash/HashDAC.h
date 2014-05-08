/* HashDAC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Class adapting the HashBB dictionary for managing DAC:
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

#ifndef HASHDAC_H
#define HASHDAC_H

#include <iostream>
using namespace std;

#include <BitSequence.h>
#include <libcdsBasics.h>
using namespace cds_static;
using namespace cds_utils;

#include "HashUtils.h"
#include "../utils/DAC_BVLS.h"
#include "../utils/LogSequence.h"
#include "../utils/Utils.h"


class HashDAC
{
	public:
		/* Generic constructor. */
		HashDAC();

		/* Creates a hash table of size the given size.
		 * @param tsize: the hash table size.
		 */
		HashDAC(size_t tsize);

		/* Inserts a new string in the hash table.
		 * @param w: the string to be inserted.
		 * @param len: the string length.
		 * @param offset: position of the string in the compressed sequence.
		 * @returns the position in which the string is inserted in the hash table.
		 */
		size_t insert(uchar *w, size_t len, size_t offset=0);

		/* Updates the offset position for the string in hash[pos]
		 * @param pos: position in which is stored the string to be updated.
		 * @param offset: new offset position.
		 */
		void setOffset(size_t pos, size_t offset);

		/* Post-processes the hash table for storing it.
		 * @param length: length of the compressed sequence in which the keys are stored.
		 */
		void finish(size_t length);

		/* Set a pointer to the DAC-encoded sequence.
		 * @param data: pointer the DAC-encoded sequence.
		 */
		void setData(DAC_BVLS *data);

		/* Searches for a key in the hash table.
		 * @param w: the key to be searched.
		 * @param len: the key length.
		 * @returns the position in which the key is stored (or -1 if it does not exist).
		 */
		size_t search(uchar *w, size_t len);

		/* Obtains the hash table size.
		 * @returns the hash table size.
		 */
		size_t getSize();

		/** Saves the hash to a file */
		void save(ofstream & fp);

		/** Loads a hash from a file*/
		static HashDAC * load(ifstream & fp);

		virtual ~HashDAC();


	protected:
		size_t tsize; 		// size of the hashtable
		size_t n; 			// number of elements into the hashtable

		BitSequence *b_ht;	// bitsequence pointing string beginnings
		DAC_BVLS *data; 	// Reference to the DAC-encoded sequence

		size_t *hashtable;	// auxiliar table used for encoding purposes


		/*compare the information in data from offset with w*/
		virtual int scmp(size_t pos, uchar *w, size_t len);

	friend class StringDictionaryHASHRPDAC;
};

#endif  /* _HASHDAC_H */

