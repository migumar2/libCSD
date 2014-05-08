/* StringDictionaryFMINDEX.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String DictionaryFMIndex based on
 * the FM-index.
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

#include "StringDictionaryFMINDEX.h"

StringDictionaryFMINDEX::StringDictionaryFMINDEX()
{
	this->type = FMINDEX;
	this->elements = 0;
	this->maxlength = 0;

	this->fm_index = NULL;
	this->separators = NULL;
}

StringDictionaryFMINDEX::StringDictionaryFMINDEX(
		IteratorDictString *it, bool sparse_bitsequence, int bparam, size_t BWTsampling)
{
	this->type = FMINDEX;
	this->elements = 0;
	this->maxlength = 0;
	this->BWTsampling = BWTsampling;

	size_t len = it->size();
	uchar *text = new uchar[len+2];
	uint *bitmap=0;

	if(BWTsampling > 0)
	{
		bitmap = new uint[(len+1+W)/W];
		for(uint i=0; i<(len+1+W)/W; i++)
			bitmap[i]=0;
		bitset(bitmap, 0);
	}

	uchar *strCurrent=NULL;
	uint lenCurrent=0;
	size_t processed = 0;

	text[processed] = '\1';	 processed++;	//We suppose that \1 is not part of the text
	text[len+1]='\0'; 			//end of the text

	while (it->hasNext())
	{
		strCurrent = it->next(&lenCurrent);
		if (lenCurrent >= maxlength) maxlength = lenCurrent+1;

		strncpy((char*)(text+processed), (char*)strCurrent, lenCurrent+1);
		processed += lenCurrent;

		{
			text[processed] = '\1';
			if (BWTsampling > 0) bitset(bitmap, processed);
			processed++;
		}

		elements++;
	}

	len = len+2;

	//just one '\0' at the end
	while(text[len-2]==text[len-3])
	{
		text[len-2]='\0';
		len--;
	}

	this->separators = NULL;

	if(BWTsampling > 0)
	{
		separators = new BitSequenceRRR(bitmap, len);
		delete [] bitmap;
	}
	else separators = NULL;

	build_ssa((uchar *)text, len, sparse_bitsequence, bparam);

	delete [] text;
}

uint
StringDictionaryFMINDEX::locate(uchar *str, uint strLen)
{
	uchar *n_s = new uchar[strLen+2];
	uint o;
	n_s[0] = '\1';
	for(size_t i=1; i<=strLen; i++) n_s[i] = str[i-1];
	n_s[strLen+1] = '\1';

	o = fm_index->locate_id(n_s, strLen+2);
	delete [] n_s;

	if(o != NORESULT) return o-2;
	return NORESULT;
}

uchar*
StringDictionaryFMINDEX::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		size_t i;

		if (id == elements) i=2;
		else i=id+3;

		return fm_index->extract_id(i, strLen, maxlength);
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

IteratorDictID*
StringDictionaryFMINDEX::locatePrefix(uchar *str, uint strLen)
{
	uchar *prefix = new uchar[strLen+2];
	prefix[0] = '\1';
	strncpy((char*)&(prefix[1]), (char*)str, strLen+1);

	size_t left, right;
	uint num_occ = fm_index->locateP(prefix, (uint)(strLen+1), &left, &right, elements);
	delete [] prefix;

	if (num_occ > 0) return new IteratorDictIDContiguous(left, right);
	else return new IteratorDictIDContiguous(NORESULT, NORESULT);
}

IteratorDictID*
StringDictionaryFMINDEX::locateSubstr(uchar *str, uint strLen)
{
	if(BWTsampling == 0){
		cout << "This dictionary configuration does not provide substring location" << endl;
		return NULL;
	}

	size_t* occs; uint num_occ;
	num_occ = fm_index->locate(str, (uint)strLen, &occs);

	if (num_occ > 0)
	{
		sort(&(occs[0]), &(occs[num_occ]));
		occs[num_occ] = 0;

		return new IteratorDictIDDuplicates(occs, num_occ);
	}
	else return new IteratorDictIDContiguous(NORESULT, NORESULT);
}

uint
StringDictionaryFMINDEX::locateRank(uint rank)
{
	return rank;
}

IteratorDictString*
StringDictionaryFMINDEX::extractPrefix(uchar *str, uint strLen)
{
	uchar *prefix = new uchar[strLen+2];
	prefix[0] = '\1';
	strncpy((char*)&(prefix[1]), (char*)str, strLen+1);

	size_t left, right;
	uint num_occ = fm_index->locateP(prefix, (uint)(strLen+1), &left, &right, elements);
	delete [] prefix;

	if (num_occ > 0) return new IteratorDictStringFMINDEX(fm_index, left, right-left+1, elements, maxlength);
	else return NULL;
}

IteratorDictString*
StringDictionaryFMINDEX::extractSubstr(uchar *str, uint strLen)
{
	if(BWTsampling == 0)
	{
		cout << "This dictionary configuration does not provide substring extraction" << endl;
		return NULL;
	}

	size_t* occs; uint num_occ;
	num_occ = fm_index->locate(str, (uint)strLen, &occs);

	if (num_occ > 0)
	{
		sort(&(occs[0]), &(occs[num_occ]));
		occs[num_occ] = 0;

		return new IteratorDictStringFMINDEXDuplicates(fm_index, occs, num_occ, elements, maxlength);
	}
	else return NULL;
}

uchar*
StringDictionaryFMINDEX::extractRank(uint rank, uint *strLen)
{
	return extract(rank, strLen);
}

IteratorDictString*
StringDictionaryFMINDEX::extractTable()
{
	return new IteratorDictStringFMINDEX(fm_index, 1, elements+1, elements, maxlength);
}

size_t
StringDictionaryFMINDEX::getSize()
{
	size_t size = fm_index->size()+sizeof(StringDictionaryFMINDEX);
	return size;
}

void
StringDictionaryFMINDEX::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	fm_index->save(out);
}

StringDictionary *
StringDictionaryFMINDEX::load(ifstream &in)
{
	size_t type = loadValue<uint32_t>(in);
	if(type != FMINDEX) return NULL;

	StringDictionaryFMINDEX *dict = new StringDictionaryFMINDEX();

	dict->type = FMINDEX;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);
	dict->fm_index = SSA::load(in);
	dict->BWTsampling = dict->fm_index->samplesuff;

	return dict;
}

void
StringDictionaryFMINDEX::build_ssa(uchar *text, size_t len, bool sparse_bitsequence, int bparam)
{
	fm_index = new SSA((uchar *)text,len, false, BWTsampling);
	Mapper * am = new MapperNone();
	wt_coder * wc = new wt_coder_huff((uchar *)text,len,am);

	BitSequenceBuilder * sbb;
	if(sparse_bitsequence) sbb = new BitSequenceBuilderRRR(bparam);
	else sbb = new BitSequenceBuilderRG(bparam);
	fm_index->set_static_bitsequence_builder(sbb);

	SequenceBuilder * ssb = new SequenceBuilderWaveletTree(sbb, am, wc);
	fm_index->set_static_sequence_builder(ssb);
	fm_index->build_index();

	if (BWTsampling > 0)
	{
		uint samples = (len+1)/BWTsampling+1;

		for (uint i=0; i<samples; i++) fm_index->suff_sample[i] = separators->rank1(fm_index->suff_sample[i]);
	}
}

StringDictionaryFMINDEX::~StringDictionaryFMINDEX()
{
	if (fm_index != NULL) delete fm_index;
	if (separators != NULL) delete separators;
}

