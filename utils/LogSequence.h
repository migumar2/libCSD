/*
 * File: LogSequence.hpp
 * Last modified: $Date: 2011-08-21 05:35:30 +0100 (dom, 21 ago 2011) $
 * Revision: $Revision: 180 $
 * Last modified by: $Author: mario.arias $
 *
 * Copyright (C) 2012, Mario Arias, Javier D. Fernandez, Miguel A. Martinez-Prieto
 * All rights reserved.
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
 *   Mario Arias:               mario.arias@gmail.com
 *   Javier D. Fernandez:       jfergar@infor.uva.es
 *   Miguel A. Martinez-Prieto: migumar2@infor.uva.es
 *
 */

#ifndef LOGSEQUENCE_H
#define LOGSEQUENCE_H

#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

class LogSequence
{
public:
	LogSequence();

	/** Loads a stream from an InputStream
	 *  @param input: the InputStream to load from
	 */
	LogSequence(ifstream &in);

	/** Create a data structure where each entry needs numbits, reserve 
	    enough space to store capacity entries.
	 */
	LogSequence(unsigned int numbits, size_t capacity);

	/** Create a data structure for storing the given vector using numbits
	    for each entry.
	 */
	LogSequence(vector<size_t> *v, unsigned int numbits);

	/** Gets the element in a specific position
	 *  @param position: the position of the element to be returned
	 *  @return int
	 */
	size_t getField(size_t position);

	/** Gets the total number of elements in the stream
	 *  @return int
	 */
	size_t getNumberOfElements();

	/** Sets the element in a specific position
	 *  @param position: the position of the new element
	 *  @param value Value to be inserted.
	 */
	void setField(size_t position, size_t value);

	/** Gets the total size in bytes of the data structure. */
	size_t getSize();

	/** Saves the stream 
	 *  @param output: the OutputStream to be saved to
	 */
	void save(ofstream &out);

	/** Returns the number of bits effectively used for each position
	 */
	uint getNumbits() { return numbits; }

	/** Destructor. */
	virtual ~LogSequence();

private:
	unsigned char numbits;
	size_t arraysize;
	size_t numentries;
	size_t maxval;
	size_t *array;

	static const unsigned int WLS = sizeof(size_t)*8;

	/** size_t's required to represent n integers of e bits each */
	inline size_t numElementsFor(const size_t bitsField, const size_t numEntries) {
		return (((uint64_t)bitsField*numEntries+WLS-1)/WLS);
	}

	/** Number of bytes required to represent n integers of e bits each */
	inline size_t numBytesFor(const size_t bitsField, const size_t numEntries) {
		return ((uint64_t)bitsField*numEntries+7)/8;
	}

	/** Retrieve a given index from array A where every value uses len bits
	 * @param data Array
	 * @param bitsField Length in bits of each field
	 * @param index Position to store in
	 */
	inline size_t get_field(const size_t *data, const size_t bitsField, const size_t index) {
		size_t bitPos = index*bitsField;
		size_t i=bitPos/WLS;
		size_t j=bitPos%WLS;
		size_t result;

		if (j+bitsField <= WLS) {
			result = (data[i] << (WLS-j-bitsField)) >> (WLS-bitsField);
		} else {
			result = data[i] >> j;
			result = result | (data[i+1] << ( (WLS<<1) -j-bitsField)) >> (WLS-bitsField);
		}
		return result;
	}

	/** Store a given value in index into array A where every value uses len bits
	 * @param data Array
	 * @param bitsField Length in bits of each field
	 * @param index Position to store in
	 * @param value Value to be stored
	 */
	inline void set_field(size_t *data, const size_t bitsField, const size_t index, const size_t value) {
		size_t bitPos = index*bitsField;
		size_t i=bitPos/WLS;
		size_t j=bitPos%WLS;

		size_t mask = ~(~((size_t)0) << bitsField) << j;
		data[i] = (data[i] & ~mask) | (value << j);

		if (j+bitsField>WLS) {
			mask = (~((size_t)0) << (bitsField+j-WLS));
			data[i+1] = (data[i+1] & mask)| value >> (WLS-j);
		}
	}

	inline unsigned int bits(size_t n) {
		size_t b = 0;
		while (n) { b++; n >>= 1; }
		return b;
	}

	inline size_t maxVal(unsigned int numbits) {
	    // The << operator is undefined for the number of bits of the word,
	    // Therefore we need to check for corner cases.
	    if(numbits==32) {
		return 0xFFFFFFFFU;
	    } else if(numbits==64) {
		return (size_t)0xFFFFFFFFFFFFFFFFULL;
	    } else {
		return ~((size_t)-1<<numbits);
	    }
	}

};

#endif /* LOGSEQUENCE_H */

