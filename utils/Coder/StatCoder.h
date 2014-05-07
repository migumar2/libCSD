/* StatCoder.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a general encoder/decoder for dealing with Huffman
 * and Hu-Tucker codes.
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

#ifndef STATCODER_H_
#define STATCODER_H_

#include <libcdsBasics.h>
using namespace cds_utils;

#include "Codeword.h"
#include "DecodingTable.h"

class StatCoder
{
	public:
		// Generic constructor
		StatCoder() { };

		// Constructor for encoding purposes
		StatCoder(Codeword *codewords)  { this->codewords = codewords; };

		// Constructor for decoding purposes
		StatCoder(DecodingTable *table, Codeword *codewords)
		{
			this->table = table;
			this->codewords = codewords;
		};

		/** Performs the Hu-Tucker encoding of the symbol in the text
		    jumping, in the current byte, the given number of bits.
		    @param symbol: the symbol to be encoded.
		    @param text: the text in which the symbol is encoded.
		    @param offset: the number of bits to be jumped.
		    @returns the number of advanced bytes in this operation.
		*/
		uint encodeSymbol(uchar symbol, uchar *text, uint *offset);

		/** Performs the Hu-Tucker encoding of the string.
		    @param str: the string to be encoded.
		    @param strLen: the string length.
		    @param encLen: pointer to the encoded string length.
		    @param offset: pointer to number of bits used in the last
		      byte.
		    @returns the encoded string.
		*/
		uchar* encodeString(uchar *str, uint strLen, uint *encLen, uint *offset);

		uint decodeString(ChunkScan* c);

		~StatCoder() { };

	protected:
		Codeword *codewords;
		DecodingTable *table;
};

#endif /* STATCODER_H_ */
