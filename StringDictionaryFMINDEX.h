/* StringDictionaryFMINDEX.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary based on self-indexing.
 * It follows the foundations described in the technique FM-Index approach from:
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

#ifndef _STRINGDICTIONARYFMINDEX_H_
#define _STRINGDICTIONARYFMINDEX_H_

#include <iostream>
using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

#include <SequenceBuilder.h>
#include <Sequence.h>
#include <BitSequenceBuilder.h>
#include <BitSequence.h>
#include "FMIndex/SSA.h"
using namespace cds_static;

#include "StringDictionary.h"

class StringDictionaryFMINDEX : public StringDictionary
{
	public:
		/** Generic Constructor. */
		StringDictionaryFMINDEX();

		/** Class Constructor.
	    	@param it: iterator scanning the original set of strings.
	    	@sparse_bitsequence: tell which rank/select implementation will be
	    	 use into the FMIndex (false->BitSequenceRG and true->BitSequenceRRR)
	    	@bparam: bparam can be (2,3,4,20,40) is BitSequenceRG is chosen.
	    	 Otherwise it is the sample rate of BitSequenceRRR.
	    	@bwt_sample: sample range that will used for the bwt (0 for no sampling)
		 */
		StringDictionaryFMINDEX(IteratorDictString *it, bool sparse_bitsequence, int bparam, size_t bwt_sample);

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
		~StringDictionaryFMINDEX();


	protected:
		SSA *fm_index;
		BitSequence *separators;
		uint BWTsampling;			//! BWT sampling (0 if no sampling)

		void build_ssa(uchar *text, size_t len, bool sparse_bitsequence, int bparam);
};

#endif /* STRINGDICTIONARYFMINDEX_H_ */
