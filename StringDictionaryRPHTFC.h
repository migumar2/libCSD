/* StringDictionaryRPHTFC.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary which differentially
 * encodes the strings using (Plain) Front-Coding and the resulting 
 * representation is finally compressed with a combination of Hu-Tucker [1]
 * (string headers) and Re-Pair code [2] (internal strings).
 * 
 *   ==========================================================================
 *     [1] "The Art of Computer Programming, volume 3: Sorting and Searching"
 *     Donald E. Knuth.
 *     Addison Wesley, 1973.
 *
 *     [2] N. Jesper Larsson and Alistair Moffat.
 *     Off-Line Dictionary-Based Compression.
 *     Proceedings of the IEEE, 88: 1722-1732, 2000.
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
 * extract operations ** by prefix **, and performs Re-Pair encoding on the
 * internal string instead of Hu-Tucker one.
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

#ifndef _STRINGDICTIONARY_RPHTFC_H
#define _STRINGDICTIONARY_RPHTFC_H

#include <iostream>
#include <vector>
using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

#include "StringDictionary.h"
#include "utils/LogSequence.h"
#include "HuTucker/HuTucker.h"
#include "Huffman/Huffman.h"
#include "utils/Coder/StatCoder.h"
#include "utils/Coder/DecodingTable.h"
#include "utils/Coder/DecodingTableBuilder.h"

#define MEMALLOC 32768

class StringDictionaryRPHTFC : public StringDictionary
{
	public:
		/** Generic Constructor. */
		StringDictionaryRPHTFC();

		/** Class Constructor.
		    @param it: iterator scanning the original set of strings.
		    @param bucketsize: number of strings represented per bucket.
		*/
		StringDictionaryRPHTFC(IteratorDictString *it, uint bucketsize);


		StringDictionaryRPHTFC(uchar *file);

		
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
		~StringDictionaryRPHTFC();

	protected:
		uint32_t maxcomplength;	//! The largest compressed header

		uint32_t buckets;	//! Number of total buckets in the dictionary
		uint32_t bucketsize;	//! Number of strings per bucket

		uint64_t bytesStrings;	//! Length of the strings representation
		uchar *textStrings;	//! Hu-Tucker over the Front-Coding representation of all remaining strings
		LogSequence *blStrings;	//! Positional index to the strings representation

		StatCoder *coderHT;	//! Coder for Hu-Tucker operations
		Codeword *codewordsHT;	//! Hu-Tucker codeword assignment
		DecodingTable *tableHT;	//! Decoding table for HT

		RePair *rp;		//! The RePair representation
		uint bitsrp;		//! Number of bits used for encoding Re-Pair symbols

		/** Obtaining the (encoded) header string for the given bucket.
		    @param idbucket: the bucket.
		    @returns the encoded header.
		*/
		inline uchar *getHeader(size_t idbucket);

		/** Decodes the first string in the bucket. 
		    @idbucket: the bucket storing the required header.
		    @returns the scanning data
		*/
		inline ChunkScan decodeHeader(size_t idbucket);

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
		    @c: pointer to the scanning data
		    @scanneable: the number of string to be scanned
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @param offset: the number of bits to be jumped.
		    @returns the internal ID (of 0 if it is not in the bucket).
		*/
		inline uint searchPrefix(ChunkScan* c, uint scanneable, uchar *str, uint strLen, uint *offset);

		/** Searches the first string non-prefixed for the given one.
		    @c: pointer to the scanning data
		    @scanneable: the number of string to be scanned
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @param offset: the number of bits to be jumped.
		    @returns the internal ID (of 0 if it is not in the bucket).
		*/
		inline uint searchDistinctPrefix(ChunkScan* c, uint scanneable, uchar *str, uint strLen, uint *offset);

		/** Encodes the Re-Pair symbol in the text.
		    @param symbol: the symbol to be encoded.
		    @param text: the text in which the symbol is encoded.
		    @param offset: the number of bits to be jumped.
		    @returns the number of advanced bytes in this operation.
		*/
		inline uint encodeSymbol(uint symbol, uchar *text, uint *offset);

		/** Decodes the next symbol.
		 	@param symbol: the symbol to be decoded.
		    @param ptr: pointer to the next byte to be decoded.
		    @param offset: the number of bits to be jumped in the byte.
		    @returns the number of advanced bytes in this operation.
		*/
		inline uint decodeSymbol(uint *symbol, uchar *ptr, uint *offset);

		/** Decodes the next string.
			@param str: the string to be decoded.
		    @param strLen: the string length.
		    @param ptr: pointer to the next byte to be decoded.
		    @param offset: the number of bits to be jumped in the byte.
		    @returns the number of shared chars with the previous string.
		 */
		inline uint decodeString(uchar *str, uint *strLen, uchar **ptr, uint *offset);
}; 

#endif  /* _STRINGDICTIONARY_RPHTFC_H */

