/* RePair.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class comprises some utilities for RePair compression and decompression.
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

#ifndef REPAIR_H_
#define REPAIR_H_

using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

#include "Coder/IRePair.h"
#include "../utils/LogSequence.h"
#include "../utils/DAC_VLS.h"
#include "../utils/Utils.h"


class RePair
{
	public:

		/** Generic constructor. */
		RePair();

		/** Constructor performing RePair compression over an integer sequence.
		 *  @param sequence: the sequence to be compressed.
		 *  @param length: the sequence length.
		 *  @param maxchar: the highest char used in the sequence.
		 */

		RePair(int *sequence, uint length, uchar maxchar);

		/** Returns the RePair representation size.
		 * @returns representation size.
		 */
		size_t getSize();

		/** Returns the number of bits required for encoding purposes */
		uint getBits() { return bits(rules+terminals); };

		/** Stores the dictionary into an ofstream.
		    @param out: the oftstream.
		    @param encoding: encoding mode (log bits or DAC)
		*/
		void save(ofstream &out, uint encoding);

		/** Stores the dictionary into an ofstream.
		    @param out: the oftstream.
		*/
		void save(ofstream &out);

		/** Loads a RePair encoding from an ifstream.
		    @param in: the ifstream.
		    @returns the loaded dictionary.
		*/
		static RePair *load(ifstream &in);

		/** Loads a RePair encoding (without sequence) from an ifstream.
		    @param in: the ifstream.
		    @returns the loaded dictionary.
		*/
		static RePair* loadNoSeq(ifstream &in);

		// Generic destructor
		~RePair();

	protected:
		uchar maxchar;			//! The highest char used in the dictionary
		LogSequence *Cls;		//! RePair sequence (in a sequence of log bits per symbols)
		DAC_VLS *Cdac;			//! RePair sequence (in a DAC-based representation)

		uint64_t terminals;		//! Number of terminals in the grammar G
		uint64_t rules;			//! Number of rules in the grammar G
		LogSequence *G;			//! RePair grammar (using 2*log(terminals+rules) bits per rule.

		/** Expands the required rule into str.
		    @param rule: the rule to be extracted.
		    @param str: the expanded string.
		    @returns the string length.
		*/
		uint expandRule(uint rule, uchar *str);

		/** Expands the required rule and compares it with respect to
		    the given string 'str'. Returns an integer value containing
		    the comparison result between the extracted rule and the
		    required string.
		    @param rule: the rule to be expanded and compared.
		    @param str: the string to be compared.
		    @param pos: pointer to the current position in the
		      comparison.
		    @returns the comparison result.
		*/
		int expandRuleAndCompareString(uint rule, uchar *str, uint *pos);

		/** Extracts the id-th string comparing it with respect to the
		    required str. Returns an integer value containing the
		    comparison result between the id-th string and the
		    required str one.
		    @param id: the string to be extracted.
		    @param str: the string to be compared.
		    @param strLen: the string length.
		    @returns the comparison result.
		*/
		int extractStringAndCompareRP(uint id, uchar* str, uint strLen);

		/** Similar than "extractStringAndCompareRP", extracts the string from
		 *  a DAC encoding.
		 */
		int extractStringAndCompareDAC(uint id, uchar* str, uint strLen);

		/** Expands the required rule and compares it with respect to
		    the given prefix 'prefix'. Returns an integer value containing
		    the comparison result between the extracted rule and the
		    required string.
		    @param rule: the rule to be expanded and compared.
		    @param str: the prefix to be compared.
		    @param pos: pointer to the current position in the
		      comparison.
		    @returns the comparison result.
		*/
		int expandRuleAndComparePrefixDAC(uint rule, uchar *str, uint *pos);


		/** Extracts the id-th string comparing it with respect to the
		    required prefix. Returns an integer value containing the
		    comparison result between the id-th string and the
		    required prefix one.
		    @param id: the string to be extracted.
		    @param prefix: the prefix to be compared.
		    @param strLen: prefix length.
		    @returns the comparison result.
		*/
		int extractPrefixAndCompareDAC(uint id, uchar* prefix, uint prefixLen);

		friend class StringDictionaryHASHRPF;
		friend class StringDictionaryHASHRPDAC;
		friend class StringDictionaryRPDAC;
		friend class StringDictionaryRPFC;
		friend class StringDictionaryRPHTFC;

		friend class IteratorDictStringRPFC;
		friend class IteratorDictStringRPHTFC;
};

#endif /* REPAIR_H_ */
