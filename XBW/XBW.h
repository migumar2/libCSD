/* XBW.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * XBW implementation
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

#ifndef _XBW_H_
#define _XBW_H_

#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

extern bool printing;

#include <libcdsBasics.h>
#include <Sequence.h>
#include <BitSequence.h>
#include <Mapper.h>

using namespace cds_utils;
using namespace cds_static;


/** Implementation of the XBW-Index proposed by Ferragina, Luccio, Manzini and Muthukrishnan
 * in FOCS 2005. The idea is patented, this code can't be redistributed nor used for
 * commercial purposes.
 *
 * @author Francisco Claude
 * @email  fclaude@recoded.cl
 */
class XBW {
public:
	/** Builds an XBW object reading it from a file, it uses ssb to create the \alpha sequence
	 * and sbb to create the bit-sequences.
	 * @param input the input stream
	 */
	XBW(ifstream &input);

	/** Destroys the object */
	~XBW();

	uint getId(const uchar * qry, const uint ql) const;

	void idToStr(uint id, uint *pos, uchar ** v, uint cnt) const;

	/** Searchs for path query in the tree, produces an array with the node ids that
	 * satisfy the query.
	 * @param qry query, qry[0]=a and qry[1]=b means /a/b
	 * @param ql length of the query
	 * @param left first node in the query result
	 * @param right last node in the query result
	 */
	void subPathSearch(const uchar * qry, const uint ql, uint *left, uint *right) const;

	/** Returns the size of the XBW structure */
	uint size() const;



protected:
	/** number of nodes in the tree */
	uint nodesCount;

	/** max label */
	uint maxLabel;

	/** labels of the nodes in the tree */
	Sequence * alpha;

	/** marks the rightmost child of every node */
	BitSequence * last;

	/** marks the beginning of each symbol run in \pi */
	BitSequence * A;

	uint mapping[257];
	uint unmap[257];
	uint select_A[257];


	uint inline getCharChild(const uint n, const uint l) const;

	/** Computes the k-th child of node n
	 * @param n node id
	 * @param k child number
	 * @return commputes the k-th child of node n
	 */
	uint inline getRankedChild(const uint n, const uint k) const;

	/** gets the range where the children of node n are
	 *  @param n node id
	 *  @param ini pointer to where to store the initial position of the range
	 *  @param fin pointer to where to store the final position of the range
	 */
	void getChildren(const uint n, uint * ini, uint * fin) const;

	/** Computes the parent of a node
	 * @param n node id
	 * @return parent of n
	 */
	uint getParent(const uint n) const;

	/** Computes how many children does node n have
	 * @param n node id
	 * @return degree of n
	 */
	uint inline getDegree(const uint n) const;

	/** Searchs for path query in the tree. Computes the range of nodes that satisfy the
	 * query up to the last item.
	 * @param qry query, qry[0]=a and qry[1]=b means /a/b
	 * @param ql length of the query
	 * @param ini pointer for storing the initial position of the range answering the query
	 * @param fin pointer for storing the final position of the range answering the query
	 */
	/*
	void subPathSearch2(const uchar * query, const uint ql, uint * ini,
			uint * fin) const;
	*/

	friend class StringDictionaryXBW;
	friend class IteratorDictIDXBW;
	friend class IteratorDictIDXBWDuplicates;
	friend class IteratorDictStringXBW;
	friend class IteratorDictStringXBWDuplicates;
};


#endif /* XBW_H_ */
