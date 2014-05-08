/* StringDictionaryHASHUFFDAC.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a Compressed String Dictionary based on double hashing.
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

#include "StringDictionaryHASHUFFDAC.h"

StringDictionaryHASHUFFDAC::StringDictionaryHASHUFFDAC()
{
	this->type = HASHUFFDAC;
	this->elements = 0;
	this->maxlength = 0;
}

StringDictionaryHASHUFFDAC::StringDictionaryHASHUFFDAC(IteratorDictString *it, uint len, int overhead)
{
	this->type = HASHUFFDAC;
	this->elements = 0;
	this->maxlength = 0;

	{
		// Counting the elements in Tdict for building the hash table structure
		uint lenCurrent=0;

		while (it->hasNext())
		{
			it->next(&lenCurrent);
			if (lenCurrent >= maxlength) maxlength = lenCurrent+1;
			elements++;
		}
	}

	// Obtaining the Huffman code
	uchar *text = ((IteratorDictStringPlain*)it)->getPlainText();
	Huffman	*huff = new Huffman(text, len);

	// Initializing the hash table
	uint hash_size = (uint)(elements*(1+(overhead*1.0/100.0)));
	hash = new HashDAC(hash_size);

	// Initializing the builder for the decoding table and the coder for
	// Huffman compression
	DecodingTableBuilder *builder = new DecodingTableBuilder();
	builder->initializeFromHuffman(huff);
	delete huff;
	codewords = builder->getCodewords();
	coder = new StatCoder(codewords);

	// Auxiliar variables
	size_t ptr = 0; uint offset = 0, bytes = 0;
	uchar *tmp = new uchar[4*maxlength];
	vector<uchar> textSubstr; vector<ushort> lenSubstr;
	ushort ptrSubstr=0; uint codeSubstr=0;

	// Variables for the DAC representation
	uint tamCode = 0;			// DAC length
	uint nLevels = 0;			// DAC height
	uchar *dacseq;				// Byte sequence
	BitString * bS;				// Bistring drawing the DAC structure
	vector<uint> levelsIndex;		// Starting position for each level

	// Simulating the hash representation
	vector<SortString> sorting(elements);

	for (uint current=1; current<=elements; current++)
	{
		// Resetting variables for the next string
		bytes = 0; tmp[bytes] = 0; offset = 0;

		// Storing the sting position in Tdict
		sorting[current-1].original = ptr;

		// Encoding the string
		do
		{
			uchar symbol = (text[ptr]);
			bytes += coder->encodeSymbol(symbol, &(tmp[bytes]), &offset);
			ptr++;
		}
		while (text[ptr-1] != '\0');

		{
			// Padding the last byte (if neccesary)
			if (offset > 0) bytes++;

			// Simulating the string insertion in the hash table
			sorting[current-1].hash = hash->insert(tmp, bytes);

			{
				// Filling the DAC structures
				if (bytes > nLevels)
				{
					for (uint i=nLevels; i<bytes; i++)
						levelsIndex.push_back(0);

					nLevels = bytes;
				}
				tamCode += bytes;

				for (uint i=0; i<bytes; i++) levelsIndex[i]++;
			}
		}
	}

	bS = new BitString(tamCode);
	vector<uint> xLevels(nLevels);

	xLevels[0] = 0;
	for (uint i=1; i<nLevels; i++) xLevels[i] = xLevels[i-1] + levelsIndex[i-1];
	for (uint i=0; i<nLevels; i++) levelsIndex[i] = xLevels[i];

	vector<uint> rankLevels(nLevels+1,0);  	// Ranks until the level beginnings

	// Sorting Tdict into Tdict*
	std::sort(sorting.begin(), sorting.end(), sortTdict);

	// Building the Hash representation
	dacseq = new uchar[tamCode];
	uint64_t bytesStrings = 0;

	for (uint current=1; current<=elements; current++)
	{
		// Resetting variables for the next string
		{
			bytes = 0; tmp[bytes] = 0; offset = 0;
			ptrSubstr=0, codeSubstr=0;
			textSubstr.clear(); lenSubstr.clear();

			ptr = sorting[current-1].original;
		}

		// Encoding the string
		do
		{
			uchar symbol = (text[ptr]);
			bytes += coder->encodeSymbol(symbol, &(tmp[bytes]), &offset);
			ptr++;

			builder->insertDecodeableSubstr(symbol, &codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);
		}
		while (text[ptr-1] != '\0');

		{
			// Padding the last byte (if neccesary)
			if (offset > 0) bytes++;

			// Inserting the string in the hash table
			hash->setOffset(sorting[current-1].hash, bytesStrings);

			// Copying the encoded string into the compressed sequence
			for (uint i=0; i<bytes; i++)
			{
				dacseq[xLevels[i]] = tmp[i];

				if ((i+1) < bytes) { bS->setBit(xLevels[i], true); rankLevels[i]++;}
				else  bS->setBit(xLevels[i], false);

				(xLevels[i])++;
			}

			bytesStrings += bytes;
		}

		rankLevels[nLevels] = bytesStrings;

		// Adding an ending decodeable string
		if (textSubstr.size() > 0)
		{
			// It is necessary padding the substring
			codeSubstr = (codeSubstr << (TABLEBITSO-ptrSubstr));
			ptrSubstr = TABLEBITSO;

			builder->insertEndingSubstr(&codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);
		}
	}

	delete [] tmp;delete it;

	dac = new DAC_BVLS(tamCode, nLevels, &levelsIndex, &rankLevels, dacseq, bS);
	delete bS;

	bytesStrings++;

	table = builder->getTable();
	hash->finish(bytesStrings);

	delete builder;
}

uint
StringDictionaryHASHUFFDAC::locate(uchar *str, uint strLen)
{
	uint id = NORESULT;

	// Encoding the string
	uint encLen, offset;
	uchar *encoded = coder->encodeString(str, strLen+1, &encLen, &offset);
	id = hash->search(encoded, encLen)+1;

	delete [] encoded;
	return id;
}

uchar *
StringDictionaryHASHUFFDAC::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements)) return extractString(id, strLen);
	else { *strLen = 0; return NULL; }
}

IteratorDictID*
StringDictionaryHASHUFFDAC::locatePrefix(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide prefix location" << endl;
	return NULL;
}

IteratorDictID*
StringDictionaryHASHUFFDAC::locateSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint
StringDictionaryHASHUFFDAC::locateRank(uint rank)
{
	cout << "This dictionary does not provide rank location" << endl;
	return 0;
}

IteratorDictString*
StringDictionaryHASHUFFDAC::extractPrefix(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide prefix extraction" << endl;
	return NULL;
}

IteratorDictString*
StringDictionaryHASHUFFDAC::extractSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring extraction" << endl;
	return NULL;
}

uchar *
StringDictionaryHASHUFFDAC::extractRank(uint rank, uint *strLen)
{
	cout << "This dictionary does not provide rank extraction" << endl;
	return NULL;
}

IteratorDictString*
StringDictionaryHASHUFFDAC::extractTable()
{
	vector<uchar*> tabledec(elements);
	uint strLen;

	for (uint i=1; i<=elements; i++)
		tabledec[i-1] = extractString(i, &strLen);

	return new IteratorDictStringVector(&tabledec, elements);
}

uchar*
StringDictionaryHASHUFFDAC::extractString(size_t id, uint *strLen)
{
	uchar *dec = new uchar[4*maxlength];
	uchar *tmp = new uchar[4*maxlength];

	uint cid = id;
	uint level = 0;

	while(cid != (uint)-1)
	{
		dec[level] = dac->access_next(level, &cid);
		level++;
	}

	dec[level] = 0;

	ChunkScan chunk = {0, 0, dec, level, tmp, 0, 0, 1};

	while (chunk.b_remain > 0)
	{
		if (table->processChunk(&chunk))
		{
			chunk.c_valid = 0;
			break;
		}
	}

	delete [] dec;

	if (chunk.c_valid != 0)
	{
		do
		{
			if (chunk.c_valid < TABLEBITSO)
			{
				// Padding the substring
				chunk.c_chunk = chunk.c_chunk << (TABLEBITSO-chunk.c_valid);
				chunk.c_valid=TABLEBITSO;
			}
		}
		while (!table->getSubstring(&chunk));
	}

	tmp[chunk.strLen] = '\0';
	*strLen = chunk.strLen-1;

	return tmp;
}


size_t
StringDictionaryHASHUFFDAC::getSize()
{
	return dac->getSize()+hash->getSize()+256*sizeof(Codeword)+table->getSize()+sizeof(StringDictionaryHASHUFFDAC)+256*sizeof(bool);
}

void
StringDictionaryHASHUFFDAC::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);

	// Information for Hash and Huffman encoding
	hash->save(out);
	dac->save(out);

	// Decoding Table
	saveValue<Codeword>(out, codewords, 256);
	table->save(out);
}



StringDictionary*
StringDictionaryHASHUFFDAC::load(ifstream &in)
{
	size_t type = loadValue<uint32_t>(in);
	if (type != HASHUFFDAC) return NULL;

	StringDictionaryHASHUFFDAC *dict = new StringDictionaryHASHUFFDAC();

	dict->type = HASHUFFDAC;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);

	dict->hash = HashDAC::load(in);
	dict->dac = DAC_BVLS::load(in);
	dict->hash->setData(dict->dac);

	dict->codewords = loadValue<Codeword>(in, 256);
	dict->table = DecodingTable::load(in);
	dict->coder = new StatCoder(dict->table, dict->codewords);

	return dict;
}

StringDictionaryHASHUFFDAC::~StringDictionaryHASHUFFDAC()
{
	delete hash;
	delete [] codewords;
	delete table; delete coder;
	delete dac;
}

