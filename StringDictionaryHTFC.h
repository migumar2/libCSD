/* StringDictionaryHTFC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary which differentially
 * encodes the strings using (Plain) Front-Coding and the resulting 
 * representation is finally compressed with Hu-Tucker:
 * 
 *   ==========================================================================
 *     "The Art of Computer Programming, volume 3: Sorting and Searching"
 *     Donald E. Knuth.
 *     Addison Wesley, 1973.
 *   ==========================================================================
 *
 * This technique is inspired in the Hu-Tucker Front-Coding approach from:
 *
 *   ==========================================================================
 *     "Compressed String Dictionaries"
 *     Nieves R. Brisaboa, Rodrigo Canovas, Francisco Claude, 
 *     Miguel A. Martinez-Prieto and Gonzalo Navarro.
 *     10th Symposium on Experimental Algorithms (SEA'2011), p.136-147, 2011.
 *   ==========================================================================
 *
 * The current approach enhances the original one with efficient locate and 
 * extract operations ** by prefix **.
 *
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

#ifndef _STRINGDICTIONARY_HTFC_H
#define _STRINGDICTIONARY_HTFC_H

#include <iostream>
#include <vector>
using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

#include "StringDictionary.h"
#include "utils/LogSequence.h"
#include "HuTucker/HuTucker.h"
#include "utils/Coder/StatCoder.h"
#include "utils/Coder/DecodingTable.h"
#include "utils/Coder/DecodingTableBuilder.h"

#define MEMALLOC 32768

class StringDictionaryHTFC : public StringDictionary 
{
	public:
		/** Generic Constructor. */
		StringDictionaryHTFC();

		/** Class Constructor.
		    @param it: iterator scanning the original set of strings.
		    @param bucketsize: number of strings represented per bucket.
		*/
		StringDictionaryHTFC(IteratorDictString *it, uint bucketsize);
		
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
		~StringDictionaryHTFC();

	protected:
		uint32_t maxcomplength;	//! The largest compressed header

		uint32_t buckets;	//! Number of total buckets in the dictionary
		uint32_t bucketsize;	//! Number of strings per bucket

		uint64_t bytesStrings;	//! Length of the strings representation
		uchar *textStrings;	//! Hu-Tucker over the Front-Coding representation for all remaining strings
		LogSequence *blStrings;	//! Positional index to the strings representation

		StatCoder *coder;	//! Coder for Hu-Tucker operations
		Codeword *codewords;	//! Hu-Tucker codeword assignment
		DecodingTable *table; 	//! Decoding table

		/** Obtaining the (encoded) header string for the given bucket.
		    @param idbucket: the bucket.
		    @returns the encoded header.
		*/
		inline uchar *getHeader(size_t idbucket);

		/** Performs the Hu-Tucker encoding of the string.
		    @param str: the string to be encoded.
		    @param strLen: the string length.
		    @param encLen: pointer to the encoded string length.
		    @param offset: pointer to number of bits used in the last
		      byte.
		    @returns the encoded string.
		*/
		inline uchar* encodeString(uchar *str, uint strLen, uint *encLen, uint *offset);

		/** Performs the Hu-Tucker encoding of the symbol in the text
		    jumping, in the current byte, the given number of bits.
		    @param symbol: the symbol to be encoded.
		    @param text: the text in which the symbol is encoded.
		    @param offset: the number of bits to be jumped.
		    @returns the number of advanced bytes in this operation.
		*/
		inline uint encodeSymbol(uchar symbol, uchar *text, uint *offset);

		/** Decodes the first string in the bucket. 
		    @idbucket: the bucket storing the required header.
		    @returns the scanning data
		*/
		inline ChunkScan decodeHeader(size_t idbucket);

		/** Decodes the next internal string according to the 
		    scanning data
		    @c: pointer to the scanning data
		    returns the number of chars shared with the previous string
		*/
		inline uint decodeNextString(ChunkScan* c);

		/** Decodes the next internal string according to the
		    scanning data
		    @c: pointer to the scanning data
		    returns the number of chars shared with the previous string
		*/
		inline uint decodeNextString2(ChunkScan* c);

		/** Locates the candidate bucket in which the given string can
		    be represented.
		    @param str: the encoded string to be located.
		    @param strLen: the encoded string length.
		    @param idbucket: pointer to the candidate bucket.
		    @returns a boolean value telling if the string is the 
		      header of the bucket.
		*/
		inline bool locateBucket(uchar *str, uint strLen, size_t *idbucket);

		/** Locates the buckets which delimits the representation of
		    all possible elments prefixed by the given string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @param offset: number of bits used in the last byte.
		    @param left: pointer to the left boundary bucket.
		    @param right: pointer to the right boundary bucket.
		*/
		inline void locateBoundaryBuckets(uchar *str, uint strLen, uint offset, size_t *left, size_t *right);

		/** Searches the first string prefixed for the given one.
		    @param c: pointer to the scanning data
		    @param scanneable: the number of string to be scanned
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns the internal ID (of 0 if it is not in the bucket).
		*/
		inline uint searchPrefix(ChunkScan* c, uint scanneable, uchar *str, uint strLen);

		/** Searches the first string non-prefixed for the given one.
		    @param c: pointer to the scanning data
		    @param scanneable: the number of string to be scanned
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @returns the internal ID (of 0 if it is not in the bucket).
		*/
		inline uint searchDistinctPrefix(ChunkScan* c, uint scanneable, uchar *str, uint strLen);

		/** Resets the scanning value for the given bucket.
		    @c: pointer to the scanning data
		    @param idbucket: pointer to the current bucket.
		*/
		inline void resetScan(ChunkScan *c, size_t idbucket);

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
		    @param tableSubstr: table containing all substrings 
		      decodeable substrings.
		*/
		inline void insertDecodeableSubstr(uchar symbol, uint *seq, ushort *ptr, vector<uchar> *substr, vector<ushort> *lenSubstr, DecodeableSubstr *tableSubstr);

		/** Similar than insertDecodeableSubstr, but it fills the 
		    sequence with 0s and inserts it in the table using as
		    decoding information that currently stored in the queues */
		inline void insertEndingSubstr(uint *seq, ushort *ptr, vector<uchar> *substr, vector<ushort> *lenSubstr, DecodeableSubstr *tableSubstr);
}; 

#endif  /* _STRINGDICTIONARY_HTFC_H */

