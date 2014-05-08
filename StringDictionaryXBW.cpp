/* StringDictionaryXBW.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary combining Re-Pair 
 * compression and Directly Addresable Codes.z
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


#include "StringDictionaryXBW.h"

StringDictionaryXBW::StringDictionaryXBW()
{
	this->type = DXBW;
	this->elements = 0;
	this->maxlength = 0;

	this->mapping = NULL;
	this->alpha = NULL;
	this->last = NULL;
	this->A = NULL;
}

StringDictionaryXBW::StringDictionaryXBW(IteratorDictString *it)
{
	this->type = DXBW;
	this->elements = 0;
	this->maxlength = 0;

	this->xbw = NULL;

	vector<TrieNode*> nodes;
	int *occ = new int[257];

	for (uint i=0; i<256; i++) occ[i] = 0;

	TrieNode * root = new TrieNode((char) 0);
	occ[0]++;
	nodes.push_back(root);

	TrieNode * root2 = new TrieNode((char) 0);
	occ[0]++;
	nodes.push_back(root2);
	root2->children[0] = root;
	root->parent = root2;
	root->last = true;

	uchar *strCurrent=NULL;
	uint lenCurrent=0;

	while (it->hasNext())
	{
		strCurrent = it->next(&lenCurrent);
		if (lenCurrent >= maxlength) maxlength = lenCurrent+1;

		strCurrent[lenCurrent]=(uchar)255;
		root->insert(strCurrent, lenCurrent, occ, &nodes);

		elements++;
	}

	len = nodes.size();

	for(uint i=0;i<len;i++) assert(nodes[i]!=NULL);

	sort(nodes.begin(), nodes.end(), compare);

	assert(nodes[0] == root2);
	assert(nodes[1] == root);

	alpha = new uint[len];
	last = new uint[len / W + 1];
	A = new uint[len / W + 2];

	for (uint i = 0; i < len / W + 2; i++) A[i] = 0;

	uint sum = 0;
	bitset(A,sum);
	for (int i = 0; i < 256; i++) { sum += occ[i]; 	bitset(A, sum); }
	bitset(A, sum + 1);
	bitset(A, len+1);

	mapping = new uint[257];
	uint act = 1;
	for(uint i=0;i<256;i++)
	{
		if(occ[i]>0) mapping[i] = act++;
		else mapping[i] = 0;
	}
	mapping[255] = act;
	mapping[256] = act+1;

	for (uint i = 0; i < len / W + 1; i++) last[i] = 0;

	for (uint i = 0; i < len; i++)
	{
		alpha[i] = mapping[(uint)((uchar)nodes[i]->symbol)];
		if (nodes[i]->last) bitset(last, i);
	}

	for (uint i=0; i<len; i++) delete nodes[i];
	delete [] occ;
}

uint 
StringDictionaryXBW::locate(uchar *str, uint strLen)
{
	uchar *qry = new uchar[strLen+1];
	qry[0] = 0;
	strncpy((char*)qry+1, (char*)str, strLen);

	uint left, right;
	xbw->subPathSearch(qry, strLen+1, &left, &right);
	delete [] qry;

	// The last child (right) is the required string if it is ended by 'maxLabel'...
	if ((left > right) || (xbw->alpha->access(right) != xbw->maxLabel)) return NORESULT;
	else return xbw->alpha->rank(xbw->maxLabel, right);
}

uchar *
StringDictionaryXBW::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uchar *str;
		xbw->idToStr(xbw->alpha->select(xbw->maxLabel, id), strLen, &str, 0);
		(*strLen)--; str[*strLen] = 0;

		return str;
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

IteratorDictID*
StringDictionaryXBW::locatePrefix(uchar *str, uint strLen)
{
	uchar *qry = new uchar[strLen+1];
	qry[0] = 0;
	strncpy((char*)qry+1, (char*)str, strLen);

	uint left, right;
	xbw->subPathSearch(qry, strLen+1, &left, &right);
	delete [] qry;

	if (left <= right) return new IteratorDictIDXBW(left, right, xbw);
	else return new IteratorDictIDContiguous(NORESULT, NORESULT);

	return NULL;
}


IteratorDictID*
StringDictionaryXBW::locateSubstr(uchar *str, uint strLen)
{
	uint left, right;
	xbw->subPathSearch(str, strLen, &left, &right);

	if (left <= right) return new IteratorDictIDXBWDuplicates(left, right, xbw);
	else return new IteratorDictIDContiguous(NORESULT, NORESULT);
}

uint 
StringDictionaryXBW::locateRank(uint rank)
{
	return rank;
}

IteratorDictString*
StringDictionaryXBW::extractPrefix(uchar *str, uint strLen)
{
	uchar *qry = new uchar[strLen+2];
	qry[0] = 0;
	strncpy((char*)qry+1, (char*)str, strLen);
	qry[strLen+1] = 0;

	uint left, right;
	xbw->subPathSearch(qry, strLen+1, &left, &right);
	delete [] qry;

	return new IteratorDictStringXBW(str, strLen, left, right, xbw, maxlength);
}

IteratorDictString*
StringDictionaryXBW::extractSubstr(uchar *str, uint strLen)
{
	uint left, right;
	xbw->subPathSearch(str, strLen, &left, &right);

	if (left <= right) return new IteratorDictStringXBWDuplicates(left, right, xbw, maxlength);
	else return NULL;
}

uchar *
StringDictionaryXBW::extractRank(uint rank, uint *strLen)
{
	return extract(rank, strLen);
}

IteratorDictString*
StringDictionaryXBW::extractTable()
{
	cerr << "TO BE IMPLEMENTED" << endl;
	return NULL;
}

size_t 
StringDictionaryXBW::getSize()
{
	return xbw->size()+sizeof(StringDictionaryXBW);
}


void 
StringDictionaryXBW::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);

	out.write((char*) &len, sizeof(uint));
	out.write((char*) mapping, 257 * sizeof(uint));
	out.write((char*) alpha, len * sizeof(uint));
	out.write((char*) last, (len / W + 1) * sizeof(uint));
	out.write((char*) A, (len / W + 2) * sizeof(uint));
}

StringDictionary*
StringDictionaryXBW::load(ifstream &in)
{
	size_t type = loadValue<uint32_t>(in);
	if(type != DXBW) return NULL;

	StringDictionaryXBW *dict = new StringDictionaryXBW();

	dict->type = DXBW;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);

	dict->xbw = new XBW(in);

	return dict;
}


StringDictionaryXBW::~StringDictionaryXBW()
{
	if (xbw != NULL) delete xbw;
	if (mapping != NULL) delete [] mapping;
	if (alpha != NULL) delete [] alpha;
	if (last != NULL) delete [] last;
	if (A != NULL) delete [] A;
}

