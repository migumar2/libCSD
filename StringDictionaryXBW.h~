/* StringDictionaryXBW.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary using the XBW [1]:
 * 
 *   ==========================================================================
 *     [1] P. Ferragina, F. Luccio, G. Manzini, and S. Muthukrishnan.
 *         Structuring labeled trees for optimal succinctness, and beyond
 *         Proc. FOCS, p.184–196, 2005.
 *   ==========================================================================
 *
 * This technique is inspired in the XBW approach from:
 *
 *   ==========================================================================
 *     "Compressed String Dictionaries"
 *     Nieves R. Brisaboa, Rodrigo Canovas, Francisco Claude, 
 *     Miguel A. Martinez-Prieto and Gonzalo Navarro.
 *     10th Symposium on Experimental Algorithms (SEA'2011), p.136-147, 2011.
 *   ==========================================================================
 *
 * The current approach enhances the original one with efficient locate and
 * extract operations ** by substring **.
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

#ifndef _STRINGDICTIONARY_XBW_H
#define _STRINGDICTIONARY_XBW_H

#include <iostream>
#include <algorithm>
#include <set>
using namespace std;

#include "StringDictionary.h"

#include "XBW/TrieNode.h"
#include "XBW/XBW.h"

#include "iterators/IteratorDictID.h"
#include "iterators/IteratorDictString.h"

class StringDictionaryXBW : public StringDictionary
{
	public:
		/** Generic Constructor. */
		StringDictionaryXBW();

		/** Class Constructor.
		    @param it: iterator containing the original set of strings.
		*/
		StringDictionaryXBW(IteratorDictString *it);
		
		/** Retrieves the ID corresponding to the given string.
		    @param str: the string to be searched.
		    @param strLen: the string length.
		    @returns the ID (or NORESULT if it is not in the bucket).
		*/
		uint locate(uchar *str, uint strLen);
		
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
		~StringDictionaryXBW();

	protected:
		XBW *xbw;

		// Auxiliars for building the XBW inputs
		uint len;
		uint *mapping;
		uint *alpha;
		uint *last;
		uint *A;


		bool static compare(TrieNode * n1, TrieNode * n2)
		{
			return (*n1).cmp(*n2);
		}


};

#endif  /* _STRINGDICTIONARY_XBW_H */

