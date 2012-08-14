/* StringDictionaryPFC.h
 * Copyright (C) 2012, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a StringDictionary comprising strings differentially
 * encoded using (Plain) Front-Coding:
 * 
 *   ==========================================================================
 *     "Managing Gigabytes : Compressing and Indexing Documents and Images"
 *     Ian H. Witten, Alistair Moffat, and Timothy C. Bell.
 *     Morgan Kaufmann, 1999.
 *   ==========================================================================
 *
 * This technique is inspired in the Front-Coding approach from:
 *
 *   ==========================================================================
 *     "Compressed String Dictionaries"
 *     Nieves R. Brisaboa, Rodrigo Canovas, Francisco Claude, 
 *     Miguel A. Martinez-Prieto and Gonzalo Navarro.
 *     10th Symposium on Experimental Algorithms (SEA'2011), p.136-147, 2011.
 *   ==========================================================================
 *
 * The current approach makes several changes w.r.t. the original one:
 *   1) The plain representation, of all first strings, is isolated in a
 *      byte-vector pointed from a positional index.
 *   3) The differential representation, of all remaining strings, is stored
 *      in a byte-vector pointed from another positional index.
 *   4) The current implementation also provides efficient locate and extract
 *      operations ** by prefix **.
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
 *   Francisco Claude:  	fclaude@cs.uwaterloo.ca
 *   Rodrigo Canovas:  		rcanovas@student.unimelb.edu.au
 *   Miguel A. Martinez-Prieto:	migumar2@infor.uva.es
 */

#ifndef _STRINGDICTIONARY_PFC_H
#define _STRINGDICTIONARY_PFC_H

#include <iostream>
using namespace std;

#include <Array.h>
#include <libcdsBasics.h>
using namespace cds_utils;

#include "StringDictionary.h"


class StringDictionaryPFC : public StringDictionary 
{
	public:
		/** Generic Constructor. */
		StringDictionaryPFC();

		/** Class Constructor.
		    @param it: iterator scanning the original set of strings.
		    @param bucketsize: number of strings represented per bucket.
		*/
		StringDictionaryPFC(IteratorDictString *it, uint bucketsize);
		
		/** Retrieves the ID corresponding to the given string.
		    @param str: the string to be searched.
		    @param strLen: the string length.
		    @returns the ID (or NORESULT if it is not in the bucket).
		*/
		uint locate(uchar *str, size_t str_length);
		
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
		size_t locateRank(uint rank);
		
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
		
		/** Computes the size of the structure in bits. 
		    @returns the dictionary size in bits.
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
		~StringDictionaryPFC();

	protected:
		uint32_t buckets;	//! Number of total buckets in the dictionary
		uint32_t bucketsize;	//! Number of strings per bucket

		uchar *textHeaders;	//! Plain headers representation
		uint64_t bytesHeaders;	//! Length of the headers representation
		Array *blHeaders;	//! Positional index to the headers representation

		uchar *textStrings;	//! Front-Coding of all remaining strings
		uint64_t bytesStrings;	//! Length of the strings representation
		Array *blStrings;	//! Positional index to the strings representation


		/** Locates the candidate bucket in which the given string can
		    be represented.
		    @param str: the string to be located.
		    @param idbucket: pointer to the candidate bucket.
		    @returns a boolean value telling if the string is the 
		      header of the bucket.
		*/
		bool locateBucket(uchar *str, size_t *idbucket);

		/** Locates the buckets which delimits the representation of
		    all possible elments prefixed by the given string.
		    @param str: the prefix to be searched.
		    @param strLen: the prefix length.
		    @param left: pointer to the left boundary bucket.
		    @param right: pointer to the right boundary bucket.
		*/
		void locateBoundaryBuckets(uchar *str, uint strLen, size_t *left, size_t *right);

		/** Creates an iterator initialished for scanning the given 
		    bucket.
		    @param idbucket: the bucket to be scanned.
		    @returns the iterator
		*/
		inline IteratorDictStringBucketPFC createBucketIterator(size_t idbucket);
}; 

#endif  /* _STRINGDICTIONARY_PFC_H */

