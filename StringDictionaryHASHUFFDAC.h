/* StringDictionaryHASHUFFDAC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary based on double hashing.
 * It is inspired ont the foundations described in the technique HashBB from:
 *
 *   ==========================================================================
 *     "Compressed String Dictionaries"
 *     Nieves R. Brisaboa, Rodrigo Canovas, Francisco Claude,
 *     Miguel A. Martinez-Prieto and Gonzalo Navarro.
 *     10th Symposium on Experimental Algorithms (SEA'2011), p.136-147, 2011.
 *   ==========================================================================
 *
 * The well-known Huffman code (in its canonical form) is used for compressiong the
 * strings, and the resulting byte stream is loaded into a Directly Adressable
 * Codes (DAC) structure.
 *
 *   ==========================================================================
 *     Nieves R. Brisaboa, Susana Ladra, and Gonzalo Navarro.
 *     DACs: Bringing Direct Access to Variable-Length Codes.
 *     Information Processing and Management 49(1):392-404, 2013.
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

#ifndef _STRINGDICTIONARY_HASHHFDAC_H
#define _STRINGDICTIONARY_HASHHFDAC_H

#include <iostream>
using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

#include "StringDictionary.h"
#include "Hash/HashDAC.h"
#include "Huffman/Huffman.h"
#include "utils/DAC_BVLS.h"
#include "utils/Coder/StatCoder.h"
#include "utils/Coder/DecodingTable.h"
#include "utils/Coder/DecodingTableBuilder.h"

class StringDictionaryHASHUFFDAC : public StringDictionary
{
	public:
		/** Generic Constructor. */
		StringDictionaryHASHUFFDAC();

		/** Class Constructor.
		    @param text: the array of strings.
		    @param len: stream size (in bytes).
		    @param overhead: overhead that the hash table will use over
		      the size of the dictionary
		*/
		StringDictionaryHASHUFFDAC(IteratorDictString *it, uint len, int overhead);

		/** Retrieves the ID corresponding to the given string.
		    @param str: the string to be searched.
		    @param strLen: the string length.
		    @returns the ID (or NORESULT if it is not in the bucket).
		*/
		uint locate(uchar *str, uint str_length);

		/** Obtains the string associated with the given ID.
		    @param id: the ID to be extracted.
		    @param strLen: pointer to the extracted string length.
		    @returns the requested string (or NULL if it is not in the
		      dictionary).
		 */
		uchar* extract(size_t id, uint *strLen);

		/** Locates all IDs of those elements prefixed by the given
		    string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns an iterator for direct scanning of all the IDs.
		*/
		IteratorDictID* locatePrefix(uchar *str, uint strLen);

		/** Locates all IDs of those elements containing the given
		    substring.
		    @param str: the substring to be searched.
		    @param strLen: the substring length.
		    @returns an iterator for direct scanning of all the IDs.
		*/
		IteratorDictID* locateSubstr(uchar *str, uint strLen);

		/** Retrieves the ID with rank k according to its alphabetical order.
		    @param rank: the alphabetical ranking.
		    @returns the ID.
		*/
		uint locateRank(uint rank);

		/** Extracts all elements prefixed by the given string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns an iterator for direct scanning of all the strings.
		*/
		IteratorDictString* extractPrefix(uchar *str, uint strLen);

		/** Extracts all elements containing by the given substring.
		    @param str: the substring to be searched.
		    @param strLen: the substring length.
		    @returns an iterator for direct scanning of all the strings.
		*/
		IteratorDictString* extractSubstr(uchar *str, uint strLen);

		/** Obtains the string  with rank k according to its
		    alphabetical order.
		    @param id: the ID to be extracted.
		    @param strLen: pointer to the extracted string length.
		    @returns the requested string (or NULL if it is not in the
		      dictionary).
		*/
		uchar* extractRank(uint rank, uint *strLen);

		/** Extracts all strings in the dictionary sorted in
		    alphabetical order.
		    @returns an iterator for direct scanning of all the strings.
		*/
		IteratorDictString* extractTable();

		/** Computes the size of the structure in bytes.
		    @returns the dictionary size in bytes.
		*/
		size_t getSize();

		/** Stores the dictionary into an ofstream.
		    @param out: the oftstream.
		*/
		void save(ofstream &out);

		/** Loads a dictionary from an ifstream.
		    @param in: the ifstream.
		    @returns the loaded dictionary.
		*/
		static StringDictionary *load(ifstream &in);

		/** Generic destructor. */
		~StringDictionaryHASHUFFDAC();

	protected:
		HashDAC *hash;		//! Hash table
		DAC_BVLS *dac;		//! DAC structure

		StatCoder *coder;	//! Coder for Hu-Tucker operations
		Codeword *codewords;	//! Huffman codeword assignment
		DecodingTable *table;	//! Decoding table for Huffman

		bool static sortTdict(SortString i, SortString j)
		{
			if (i.hash <  j.hash) return true;
			else return false;
		}

		/* Uses the decoding table for extracting the string from the DAC
		 * representation.
		 *  @param id: the ID to be extracted.
		 *  @param strLen: pointer to the extracted string length.
		 *  @returns the requested string (or NULL if it is not in the
		      dictionary).
		 */
		inline uchar* extractString(size_t id, uint *strLen);
};

#endif  /* _STRINGDICTIONARY_HASHHUFFDAC_H */


