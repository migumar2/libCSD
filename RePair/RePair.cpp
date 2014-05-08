/* RePair.cpp
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

#include "RePair.h"

RePair::RePair()
{
	this->G = NULL;
	this->Cls = NULL;
	this->Cdac = NULL;
	this->maxchar = 0;
}

RePair::RePair(int *sequence, uint length, uchar maxchar)
{
	this->G = NULL;
	this->Cls = NULL;
	this->Cdac = NULL;
	this->maxchar = maxchar;

	Tdiccarray *dicc;
	IRePair compressor;

	compressor.compress(sequence, length, (size_t*)&terminals, (size_t*)&rules, &dicc);

	// Building the array for the dictionary
	G = new LogSequence(bits(rules+terminals), 2*rules);

	for (uint i=0; i<rules; i++)
	{
		G->setField(2*i, dicc->rules[i].rule.left);
		G->setField((2*i)+1, dicc->rules[i].rule.right);
	}
	Dictionary::destroyDicc(dicc);
}

uint
RePair::expandRule(uint rule, uchar* str)
{
	uint pos = 0;
	uint left = G->getField(2*rule);
	uint right = G->getField((2*rule)+1);

	if (left >= terminals) pos += expandRule(left-terminals, str+pos);
	else { str[pos] = (char)left; pos++; }

	if (right >= terminals) pos += expandRule(right-terminals, str+pos);
	else { str[pos] = (char)right; pos++; }

	return pos;
}

int
RePair::expandRuleAndCompareString(uint rule, uchar *str, uint *pos)
{
	int cmp = 0;

	uint left = G->getField(2*rule);
	if (left >= terminals)
	{
		cmp = expandRuleAndCompareString(left-terminals, str, pos);
		if (cmp != 0) return cmp;
	}
	else
	{
		if ((uchar)left != str[*pos]) return (int)((uchar)left-str[*pos]);
		(*pos)++;
	}

	uint right = G->getField((2*rule)+1);
	if (right >= terminals)
	{
		cmp = expandRuleAndCompareString(right-terminals, str, pos);
		if (cmp != 0) return cmp;
	}
	else
	{
		if ((uchar)right != str[*pos]) return (int)((uchar)right-str[*pos]);
		(*pos)++;
	}

	return cmp;
}

int
RePair::extractStringAndCompareRP(uint id, uchar* str, uint strLen)
{
	str[strLen] = maxchar;

	uint l = 0, pos = 0, next;
	int cmp = 0;

	while (pos <= strLen)
	{
		next = Cls->getField(id+l);

		if  (next >= terminals)
		{
			cmp = expandRuleAndCompareString(next-terminals, str, &pos);
			if (cmp != 0) break;
		}
		else
		{
			if ((uchar)next != str[pos]) return (int)((uchar)next-str[pos]);
			pos++;
		}

		l++;
	}

	str[strLen] = 0;

	return cmp;
}

int
RePair::extractStringAndCompareDAC(uint id, uchar* str, uint strLen)
{
	uint l = 0, pos = 0, next;
	int cmp = 0;

	while(id != (uint)-1)
	{
		next = Cdac->access_next(l, &id);

		if (next >= terminals)
		{
			cmp = expandRuleAndCompareString(next-terminals, str, &pos);
			if (cmp != 0) return cmp;
		}
		else
		{
			if ((uchar)next != str[pos]) return (int)((uchar)next-str[pos]);
			pos++;
		}

		l++;
	}

	if (pos == strLen) return cmp;
	else return -str[pos];
}

int
RePair::expandRuleAndComparePrefixDAC(uint rule, uchar *str, uint *pos)
{
	int cmp = 0;

	uint left = G->getField(2*rule);
	if (left >= terminals)
	{
		cmp = expandRuleAndComparePrefixDAC(left-terminals, str, pos);
		if (cmp != 0) return cmp;
	}
	else
	{
		if ((uchar)left != str[*pos]) return (int)((uchar)left-str[*pos]);
		(*pos)++;
	}

	if (str[*pos] == '\0') return cmp;

	uint right = G->getField((2*rule)+1);
	if (right >= terminals)
	{
		cmp = expandRuleAndComparePrefixDAC(right-terminals, str, pos);
		if (cmp != 0) return cmp;
	}
	else
	{
		if ((uchar)right != str[*pos]) return (int)((uchar)right-str[*pos]);
		(*pos)++;
	}

	return cmp;
}

int
RePair::extractPrefixAndCompareDAC(uint id, uchar* prefix, uint prefixLen)
{
	uint l = 0, pos = 0, next;
	int cmp = 0;

	while(id != (uint)-1)
	{
		next = Cdac->access_next(l, &id);

		if (next >= terminals)
		{
			cmp = expandRuleAndComparePrefixDAC(next-terminals, prefix, &pos);
			if (cmp != 0) return cmp;
		}
		else
		{
			if ((uchar)next != prefix[pos]) return (int)((uchar)next-prefix[pos]);
			pos++;
		}

		if (prefix[pos] == '\0') return 0;

		l++;
	}

	if (pos == prefixLen) return cmp;
	else return -prefix[pos];
}

void
RePair::save(ofstream &out,  uint encoding)
{
	saveValue<uchar>(out, maxchar);
	saveValue<uint64_t>(out, terminals);
	saveValue<uint64_t>(out, rules);
	G->save(out);

	saveValue<uint32_t>(out, encoding);

	if ((encoding == HASHRPDAC) || (encoding == RPDAC)) Cdac->save(out);
	else Cls->save(out);
}

void
RePair::save(ofstream &out)
{
	saveValue<uchar>(out, maxchar);
	saveValue<uint64_t>(out, terminals);
	saveValue<uint64_t>(out, rules);
	G->save(out);
}

RePair*
RePair::load(ifstream &in)
{
	RePair *dict = new RePair();

	dict->maxchar = loadValue<uchar>(in);
	dict->terminals = loadValue<uint64_t>(in);
	dict->rules = loadValue<uint64_t>(in);
	dict->G = new LogSequence(in);

	uint encoding = loadValue<uint32_t>(in);

	if ((encoding == HASHRPDAC) || (encoding == RPDAC)) dict->Cdac = DAC_VLS::load(in);
	else dict->Cls = new LogSequence(in);

	return dict;
}

RePair*
RePair::loadNoSeq(ifstream &in)
{
	RePair *dict = new RePair();

	dict->maxchar = loadValue<uchar>(in);
	dict->terminals = loadValue<uint64_t>(in);
	dict->rules = loadValue<uint64_t>(in);
	dict->G = new LogSequence(in);

	return dict;
}


size_t
RePair::getSize()
{
	if (Cdac != NULL) return G->getSize()+Cdac->getSize()+sizeof(RePair);
	if (Cls != NULL) return G->getSize()+Cls->getSize()+sizeof(RePair);
	return G->getSize()+sizeof(RePair);
}

RePair::~RePair()
{
	delete G;
	if (Cls != NULL) delete Cls;
	if (Cdac != NULL) delete Cdac;
}
