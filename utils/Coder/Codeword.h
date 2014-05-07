/* Codeword.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Simple bit-based codeword.
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


#ifndef _CODEWORD_H
#define _CODEWORD_H

class Codeword
{		
	public:
		/** Generic Constructor */
		Codeword() { this->codeword = 0; this->bits = 0; }

		/** Class Constructor.
		    @param codeword: the codeword
		    @param bits: the codeword length (in bits)
		*/
		Codeword(uint codeword, uint bits)
		{
			this->codeword = codeword;
			this->bits = bits;
		}

		/** Generic Destructor */
		~Codeword() {};

	protected:
		uint codeword;	// Codeword (right-aligned)
		uint bits;	// Codeword length in number of bits

	friend class StatCoder;
	friend class DecodingTableBuilder;

	friend class HuTucker;
	friend class Huffman;

	friend class StringDictionaryHASHHF;
	friend class StringDictionaryHASHUFFDAC;
	friend class StringDictionaryHTFC;
	friend class StringDictionaryHHTFC;
	friend class StringDictionaryRPHTFC;

	friend class IteratorDictStringHTFC;
	friend class IteratorDictStringHHTFC;
	friend class IteratorDictStringRPHTFC;
};


#endif  /* _CODEWORD_H */

