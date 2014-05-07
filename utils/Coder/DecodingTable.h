/* DecodingTable.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a compact table for decoding prefix-based codes. It 
 * comprises two main structures:
 *
 * 1) The decoding table (of 2^k entries) implemented as a sequence of log(x)
 *    bits per entry.
 * 2) A byte-stream (of size x bytes) listing all different decoding 
 *    possibilities in the compressed text.
 *
 * This decoding structure is used as follows. The compressed text is read in
 * chunks of k bits and these are used as keys for accessing the table. That 
 * is, the k-bit chunk is transformed into a number in the range [0, 2^k-1] and
 * its corresponding entry in the table references the decoding information in 
 * the stream. It is worth noting that not all k-bit possible chunks are really
 * used in the text. In these cases, the corresponding entries store the value 0.
 *
 * The stream concatenates all decodeable substrings according to the following
 * features:
 *
 * 1) The first byte encodes the control information. It is divided in two 
 *    halves:
 *    - The first 4 bits encodes the number of characters: 'c' decoded in the
 *	current step.
 *    - The second half represents the number of bits minus 1: 'b', consumed in
 *	the current decodification step. Thus, the last 'k-b' bits in the 
 *	current chunk will be the first ones in the next chunk..
 *
 * 2) The c next bytes represent the decoded characters in the substring.
 *
 *
 * It is worth noting the exception arising when the k-bit chunk does not 
 * represent a full codeword (this means that the length of this codeword is
 * greater than k bits), so the decoding must be implemented over the code 
 * tree. In these cases, 'c=0' and 'b' tells the number of bytes used for 
 * representing the pointer to the corresponding node in the code tree. These
 * pointer is encoded (using VByte) in the following c bytes.
 *
 * This structure must be used with values of k <= 16.
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


#ifndef _DECTABLE_H
#define _DECTABLE_H

#include <assert.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>
using namespace std;

#include <BitString.h>
#include <libcdsBasics.h>
using namespace cds_utils;

#include "DecodingTree.h"
#include "../LogSequence.h"
#include "../VByte.h"
#include "../Utils.h"

#define MAXK 16
#define CHNK 16

class DecodeableSubstr
{
	public:
		/** Generic constructor. */
		DecodeableSubstr()
		{ 
			this->length = 0;
			this->dbits = 0;  
			this->ptr = 0; 
			this->ending = false;
			this->special = false;
			this->encoded = false;
			this->position = 0;
		}

		/** Set the substring information.
		    @param substr: the decodeable substring.
		    @param length: the substring length.
		    @param dbits: the number of decoded bits
		*/
		void setSubstr(vector<uchar> *substr, uint length, uint dbits)
		{
			this->length = length;
			this->dbits = dbits;
			this->ptr = 0;
			this->ending = false;
			this->special = false;
			this->encoded = false;
			this->position = 0;

			this->substr.resize(this->length+1);
			for (uint i=0; i<this->length; i++) 
			{
				if ((*substr)[i] == '\0') this->ending = true;
				this->substr[i] = (*substr)[i];
			}
		}

		/** Set the information for a *special* substring (e.g. those
		    ones padded at the end of a bucket).
		    @param substr: the decodeable substring.
		    @param length: the substring length.
		*/
		void setSpecialSubstr(vector<uchar> *substr, uint length)
		{
			this->length = length;
			this->dbits = 1;
			this->ptr = 0;
			this->ending = false;
			this->special = true;
			this->encoded = false;
			this->position = 0;

			this->substr.resize(this->length+1);
			for (uint i=0; i<this->length; i++) 
			{
				if ((*substr)[i] == '\0') this->ending = true;
				this->substr[i] = (*substr)[i];
			}
		}

		/** Set the information for a large substring. Note that length
		    is set to 0 and the dbits value is obtained according to the
		    pointer referencing the decoding subtree.
		    @param ptr: pointer to the decoding subtree.
		*/
		void setLargeSubstr(uint ptr)
		{
			this->length = 0;
			this->ptr = ptr;
			this->dbits = 1+bits(ptr)/8;
			this->ending = false;
			this->special = false;
			this->encoded = false;
			this->position = 0;
		}

		/** Generic destructor. */
		~DecodeableSubstr() { }

	protected:
		vector<uchar> substr;	//! The decodeable susbtring

		uint length;		//! The substring length
		uint dbits;		//! The number of decoded bits
		uint ptr;		//! Pointer to the possible decoding subtree
		bool ending;		//! Tells if the substring contains the '\0'
		bool special;		//! Tells if it is a *special* substring

		bool encoded;		//! Tells if the substring has been encoded in the table
		uint position;		//! The position in which the substring is encoded

	friend class DecodingTable;
	friend class DecodingTableBuilder;
	friend class StringDictionaryHTFC;
	friend class StringDictionaryHHTFC;
};


struct ChunkScan {
	uint c_chunk;		// Bytes to be currently processed
	ushort c_valid;		// Number of 'valid' bits in the current chunk

	uchar *b_ptr;		// Pointer to the current byte in the dictionary chunk
	uint b_remain;		// Number of remaining bytes in the dictionary chunk

	uchar *str;		// Extracted string
	uint strLen;		// String length
	uint advanced;		// Number of bytes extracted in advance
	uint extracted;		// Tells the number of extracted chars
};

class DecodingTable
{	
	struct Entry {
	   ushort length; 	// Number of symbols
	   ushort bits;		// Number of processed bits
	};

	public:
		/** Generic constructor. */
		DecodingTable() { };

		/** Simple constructor allocating memory for the subtrees. */
		DecodingTable(uint sigma) { this->nodes = 0; this->subtrees = new DecodingTree*[sigma]; };

		/** @returns the chunk length used in the table */
		uint getK() { return k; }

		/** Set the decoding table.
		    @param k: number of bits used in the chunk used for 
		     decodification.
		    @param substrs: table containing the information about all
		      possible decodeable subtrings
		*/
		void setDecodingTable(uint k, DecodeableSubstr* substrs);

		/** Inserts a new decoding subtree.
		    @param subtree: a decoding tree for a given codeword.
		    @returns the tree position in the structure of subtrees.
		*/
		uint setDecodingSubtree(DecodingTree *tree);


		/** Processes the next chunk and retrieves the corresponding
		    substring.
 		    @c: pointer to the scanning data
		    @returns a boolean value telling if the susbtring contains '\0'.
		*/
		bool processChunk(ChunkScan *c);

		/** Computes the size of the structure in bytes. 
		    @returns the dictionary size in bytes.
		*/
		size_t getSize();

		/** Stores the decoding table structure into an ofstream.
		    @param out: the oftstream.
		*/
		void save(ofstream &out);

		/** Loads a decoding table structure from an ifstream.
		    @param in: the ifstream.
		    @returns the loaded decoding table.
		*/
		static DecodingTable *load(ifstream &in);

		/** Generic destructor. */
		~DecodingTable();

	protected:
		uint32_t k;			//! Chunk length
		uint32_t nodes;			//! Number of subtrees

		uint64_t bytesStream;		//! The stream length (in bytes)
		uchar *stream;			//! The stream encoding the decodeable substrings

		uint *table;			//! The table pointing to the substring information
		DecodingTree **subtrees;	//! Decoding subtrees for large codewords
		BitString *endings;		//! Bitstring setting the streams with '\0'

		Entry ventry[256];

		/** Encodes in a byte the information used for decoding a
		    given susbtring.
		    @param length: the substring length.
		    @param bits: the number of decoded bits.
		    @return the information encoded in a byte.
		*/
		inline uchar encodeInfo(uint length, uint bits);

		/** Decodes the information used for decoding a given susbtring.
		    @param code: the encoded information about the substring.
		    @param length: the substring length.
		    @param bits: the number of decoded bits.
		*/
		inline void decodeInfo(uchar code, uint *length, uint *bits);

		/** Returns the substring and updates the information used
		    for the next decodification step.
 		    @c: pointer to the scanning data
		    @returns a boolean value telling if the susbtring contains '\0'.
		*/
		bool getSubstring(ChunkScan *c);

		friend class StringDictionaryHASHUFFDAC;
};

#endif  /* _DECTABLE_H */

