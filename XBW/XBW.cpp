/* XBW.cpp
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

#include "XBW.h"

XBW::XBW(ifstream &input)
{
	// Read the number of nodes
	input.read((char*) &nodesCount, sizeof(uint));

	input.read((char*) mapping, 257 * sizeof(uint));
	// Read the alpha sequence
	uint * alphaInt = new uint[nodesCount];
	input.read((char*) alphaInt, nodesCount * sizeof(uint));

	// Read bitmap last
	uint * lastInt = new uint[nodesCount / W + 1];
	input.read((char*) lastInt, (nodesCount / W + 1) * sizeof(uint));

	// Initialise and compute the max label
	maxLabel = 0;
	for (uint i = 0; i < nodesCount; i++) {
		maxLabel = max(alphaInt[i], maxLabel);
	}

	uint * AInt = new uint[nodesCount / W + 2];
	input.read((char*) AInt, (nodesCount / W + 2) * sizeof(uint));

	// Create the data structures
	Mapper * am = new MapperNone();
	wt_coder * wcc = new wt_coder_huff(alphaInt, nodesCount, am);
	//BitSequenceBuilder * sbb = new BitSequenceBuilderRG(2);		// 2, 4, 20
	BitSequenceBuilder * sbb = new BitSequenceBuilderRRR(128);		// 16, 64, 128
	SequenceBuilder * ssb = new SequenceBuilderWaveletTree(sbb, am, wcc);
	alpha = ssb->build(alphaInt, nodesCount);
	last = sbb->build(lastInt, nodesCount);
	A = sbb->build(AInt, nodesCount + 1);

	for(uint i=0; i<257; i++)
		if(mapping[i]>0) {
			select_A[mapping[i]] = A->select1(mapping[i]);
			unmap[mapping[i]] = i;
		}

	input.close();
	delete ((SequenceBuilderWaveletTree*)sbb);

	// Free the temporary arrays
	delete[] alphaInt;
	delete[] lastInt;
	delete[] AInt;
}

XBW::~XBW() {
	delete ((WaveletTree*)alpha);
	delete last;
	delete A;
}

uint XBW::size() const {
	uint s=0;
	s += alpha->getSize();
	s += last->getSize();
	s += A->getSize();
	s += sizeof(uint)*257*3;

	return s;
}

uint XBW::getId(const uchar * qry, const uint ql) const {
	uint id = 1;
	for(uint i=0;i<ql;i++) {
		if (mapping[qry[i]] == 0)
		{
			// The char is not in the alphabet
			return 0;
		}

		id = this->getCharChild(id, mapping[qry[i]]);
		if(id==(uint)-1) return 0;
	}

	return alpha->rank(maxLabel, id);
}

void XBW::idToStr(uint id, uint *pos, uchar ** v, uint cnt) const {
	if(id==1) {
		*v = new uchar[cnt];
		(*v)[cnt-1] = 0;
		*pos = 0;
		return;
	}

	idToStr(getParent(id),pos,v,cnt+1);

	(*v)[*pos] = unmap[alpha->access(id)];
	(*pos)++;
}

void XBW::subPathSearch(const uchar * qry, const uint ql, uint *left, uint *right) const {
	if (ql <= 1)
	{
		*left = 0;
		*right = nodesCount - 1;
		return;
	}

	uint i = 0;
	uint s = mapping[qry[i]];

	// The char is not in the alphabet
	if (s == 0) { *left = 1; *right = 0; return; }

	*left = select_A[s];
	*right = select_A[s+1]-1;

	uint y, z, k1, k2;

	while ((*left <= *right) && (i < ql-1))
	{
		i++;
		s = mapping[qry[i]];

		if (s == 0) { *left = 1; *right = 0; break; }

		y = select_A[s];
		z = last->rank1(y - 1);

		if (*left == 0) k1 = 0;
		else k1 = alpha->rank(s, *left-1);

		*left = last->select1(z + (uint)k1) + 1;
		k2 = alpha->rank(s, *right);
		*right = last->select1(z + (uint)k2);
	}
}

uint inline XBW::getCharChild(const uint n, const uint l) const {
	uint ini, fin;
	getChildren(n, &ini, &fin);
	if (ini > fin)
		return (uint) -1;
	uint y1 = alpha->rank(l, ini-1);
	uint ret = alpha->select(l, y1 + 1);
	if(ret>fin) return (uint)-1;
	return ret;
}

uint inline XBW::getRankedChild(const uint n, const uint k) const {
	uint ini, fin;
	getChildren(n, &ini, &fin);
	if (k > fin - ini + 1)
		return (uint) -1;
	return ini + k - 1;
}

void XBW::getChildren(const uint i, uint * ini, uint * fin) const
{
	size_t k = 0;
	uint c = alpha->access(i, k);

	if (maxLabel == c) {
		*fin = 0;
		*ini = 1;
		return;
	}

	uint y = select_A[c];
	uint z = 0;
	if (y != 0) z = last->rank1(y - 1);

	*ini = last->select1(z + (uint)k - 1) + 1;
	*fin = last->select1(z + (uint)k);
}

uint XBW::getParent(const uint n) const {
	if (n == 0) return (uint) -1;
	uint c = A->rank1(n);
	uint y = select_A[c];

	if (y == 0) return 1;
	uint k = last->rank1(n - 1) - last->rank1(y - 1);
	uint p = alpha->select(c, k + 1);
	return p;
}

uint inline XBW::getDegree(const uint n) const {
	uint ini, fin;
	getChildren(n, &ini, &fin);
	return fin - ini + 1;
}

