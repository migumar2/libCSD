/* DecodingTableBuilder.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class comprises some utilities for building the decoding table used in
 * Huffman and HuTucker codes.
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

#ifndef DECODINGTABLEBUILDER_H_
#define DECODINGTABLEBUILDER_H_

#define TABLEBITSO 16

#include <vector>
using namespace std;

#include <string.h>
#include <libcdsBasics.h>
using namespace cds_utils;

#include "DecodingTable.h"

#include "../../Huffman/Huffman.h"
#include "../../HuTucker/HuTucker.h"

class DecodingTableBuilder
{
	public:
		// Generic constructor
		// @maxv: maximum value used in the alphabet (all ASCII chars are assumed by default)
		DecodingTableBuilder(uint maxv=255);

		DecodingTableBuilder(Codeword *codews);

		// Initializes the manager from a Huffman code
		// @huff: the Huffman code.
		void initializeFromHuffman(Huffman *huff);

		// Initializes the manager from a Huffman code
		// @huff: the Huffman code.
		void initializeFromHuTucker(HuTucker *ht);

		/** Gets the codewords
		    @returns the set of codewords.
		 */
		Codeword* getCodewords();

		/** Gets the max value used in the alphabet
		    @returns the set of codewords.
		 */
		uint getMax();

		/** Gets the resulting table for decodification
		    @returns the set of codewords.
		 */
		DecodingTable* getTable();

		/** Gets the table of substrings
		    @returns the table of substrings.
		*/
		DecodeableSubstr* getTableSubstr();

		/** Processes the next encoded symbol and checks if the current
		    sequence encodes a decodeable substring. In this case,
		    information about this substring is stored for building the
		    decodeable table.
		    @param symbol: the symbol to be encoded.
		    @param seq: the current encoded sequence.
		    @param ptr: number of un-processed bits in the sequence.
		    @param substr: queue containing the symbols previosuly
		      processed.
		    @param lenSubstr: queue containing the lengths of the
		      symbol previosuly processed.
		*/
		void insertDecodeableSubstr(uchar symbol, uint *seq, ushort *ptr, vector<uchar> *substr, vector<ushort> *lenSubstr);

		/** Similar than insertDecodeableSubstr, but it fills the
		    sequence with 0s and inserts it in the table using as
		    decoding information that currently stored in the queues */
		void insertEndingSubstr(uint *seq, ushort *ptr, vector<uchar> *substr, vector<ushort> *lenSubstr);

		// Generic destructor
		~DecodingTableBuilder();

		Codeword *codewords;	
		DecodeableSubstr *tableSubstr;
		DecodingTable *table;	

	protected:
		uint maxv;	// Max value in the code
};

#endif /* DECODINGTABLEBUILDER_H_ */
