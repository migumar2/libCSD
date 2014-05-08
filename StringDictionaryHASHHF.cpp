/* StringDictionaryHASHHF.cpp
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

#include "StringDictionaryHASHHF.h"

StringDictionaryHASHHF::StringDictionaryHASHHF()
{
	this->type = HASHHF;
	this->elements = 0;
	this->maxlength = 0;
	this->maxcomplength = 0;
}

StringDictionaryHASHHF::StringDictionaryHASHHF(IteratorDictString *it, uint len, int overhead)
{
	this->type = HASHHF;
	this->elements = 0;
	this->maxlength = 0;
	this->maxcomplength = 0;

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
	hash = new Hashdh(hash_size);

	// Initializing the builder for the decoding table and the coder for
	// Huffman compression
	DecodingTableBuilder *builder = new DecodingTableBuilder();
	builder->initializeFromHuffman(huff);
	delete huff;
	codewords = builder->getCodewords();
	coder = new StatCoder(codewords);

	// Auxiliar variables
	size_t ptr = 0; uint offset = 0, bytes = 0;
	uchar *tmp = new uchar[6*maxlength];
	vector<uchar> textSubstr; vector<ushort> lenSubstr;
	ushort ptrSubstr=0; uint codeSubstr=0;


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

			if (bytes > maxcomplength) maxcomplength = bytes;
		}
	}

	// Sorting Tdict into Tdict*
	std::sort(sorting.begin(), sorting.end(), sortTdict);

	// Building the Hash representation
	size_t reservedStrings = MEMALLOC;
	textStrings = new uchar[reservedStrings];
	bytesStrings = 0; textStrings[bytesStrings] = 0;

	for (uint current=1; current<=elements; current++)
	{
		// Checking the available space in textStrings and
		// realloc if required
		while ((bytesStrings+(2*maxlength)) > reservedStrings)
			reservedStrings = Reallocate(&textStrings, reservedStrings);

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
			memcpy(textStrings+bytesStrings, tmp, bytes);
			bytesStrings += bytes;
		}

		// Adding an ending decodeable string
		if (textSubstr.size() > 0)
		{
			// #######################
			// It is necessary to read up to TABLEBITO bits for indexing the
			// substring in the DecodingTable
			if (offset > 0)
			{
				// The substring is also padded
				codeSubstr = (codeSubstr << (8-offset));
				ptrSubstr += (8-offset); offset = 0;
			}

			if (ptrSubstr > TABLEBITSO)
			{
				codeSubstr = codeSubstr >> (ptrSubstr - TABLEBITSO);
				ptrSubstr = TABLEBITSO;
			}
			else
			{
				if (current == elements)
				{
					// The last element is directly padded
					codeSubstr = (codeSubstr << (TABLEBITSO-ptrSubstr));
					ptrSubstr = TABLEBITSO;
					break;
				}

				uint read = 0;
				ptr = sorting[current].original;

				while (TABLEBITSO > ptrSubstr)
				{
					uint symbol = text[ptr+read]; read++;
					uint bits = codewords[(int)symbol].bits; 
					uint codeword = codewords[(int)symbol].codeword;

					if ((bits+ptrSubstr) <= TABLEBITSO)
					{
						codeSubstr = (codeSubstr << bits) | codeword;
						ptrSubstr += bits;
						offset += bits;
						if (offset > 8) offset -= 8;

						// The next string has fully read!
						if (symbol == 0)
						{
							if (((ptrSubstr+(8-offset)) <= TABLEBITSO))
							{
								// The next string must be parsed...
								codeSubstr = (codeSubstr << (8-offset));
								ptrSubstr += (8-offset); offset = 0;
								ptr = sorting[current+1].original;
								read = 0;
							}
							else
							{
								offset = offset % 8;

								// The padding bits are enough...
								codeSubstr = (codeSubstr << (TABLEBITSO-ptrSubstr));
								ptrSubstr = TABLEBITSO;
							}
						}
					}
					else
					{
						uint remaining = TABLEBITSO-ptrSubstr;

						codeSubstr = (codeSubstr << remaining) | (codeword >> (bits-remaining));
						ptrSubstr = TABLEBITSO;
					}
				}
			}

			builder->insertEndingSubstr(&codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);
		}
	}

	delete it;
	delete [] tmp;

	textStrings[bytesStrings] = 0; bytesStrings++;
	textStrings[bytesStrings] = 0; bytesStrings++;

	if (textSubstr.size() > 0)
		builder->insertEndingSubstr(&codeSubstr, &ptrSubstr, &textSubstr, &lenSubstr);

	bytesStrings++;

	table = builder->getTable();
	hash->finish(bytesStrings);

	delete builder;
}

uint
StringDictionaryHASHHF::locate(uchar *str, uint strLen)
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
StringDictionaryHASHHF::extract(size_t id, uint *strLen)
{
	if ((id > 0) && (id <= elements))
	{
		uchar *tmp = new uchar[4*maxlength+table->getK()];
		uint remain = maxcomplength+4;
		uint pos = hash->getValue(id);

		ChunkScan chunk = {0, 0, textStrings+pos, remain, tmp, 0, 0, 1};
		while (!(table->processChunk(&chunk)));


		tmp[chunk.strLen] = '\0';

		*strLen = chunk.strLen-1;

		return tmp;
	}
	else
	{
		*strLen = 0;
		return NULL;
	}
}

IteratorDictID*
StringDictionaryHASHHF::locatePrefix(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide prefix location" << endl;
	return NULL;
}

IteratorDictID*
StringDictionaryHASHHF::locateSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring location" << endl;
	return NULL;
}

uint
StringDictionaryHASHHF::locateRank(uint rank)
{
	cout << "This dictionary does not provide rank location" << endl;
	return 0;
}

IteratorDictString*
StringDictionaryHASHHF::extractPrefix(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide prefix extraction" << endl;
	return NULL;
}

IteratorDictString*
StringDictionaryHASHHF::extractSubstr(uchar *str, uint strLen)
{
	cout << "This dictionary does not provide substring extraction" << endl;
	return NULL;
}

uchar *
StringDictionaryHASHHF::extractRank(uint rank, uint *strLen)
{
	cout << "This dictionary does not provide rank extraction" << endl;
	return NULL;
}

IteratorDictString*
StringDictionaryHASHHF::extractTable()
{
	vector<uchar*> tabledec(elements);
	uchar *tmp = new uchar[4*maxlength+table->getK()];

	for (uint i=1; i<=elements; i++)
	{
		uint remain = maxlength;
		uint pos = hash->getValue(i);
		ChunkScan chunk = {0, 0, textStrings+pos, remain, tmp, 0, 0, 1};

		while (!(table->processChunk(&chunk)));

		tabledec[i-1] = new uchar[chunk.strLen+1];
		strncpy((char*)tabledec[i-1], (char*)chunk.str, chunk.strLen);
		tabledec[i-1][chunk.strLen] = '\0';
	}

	delete [] tmp;

	return new IteratorDictStringVector(&tabledec, elements);
}

size_t
StringDictionaryHASHHF::getSize()
{
	return bytesStrings*sizeof(uchar)+hash->getSize()+256*sizeof(Codeword)+table->getSize()+sizeof(StringDictionaryHASHHF)+256*sizeof(bool);
}

void
StringDictionaryHASHHF::save(ofstream &out)
{
	saveValue<uint32_t>(out, type);
	saveValue<uint64_t>(out, elements);
	saveValue<uint32_t>(out, maxlength);
	saveValue<uint32_t>(out, maxcomplength);

	// Information for Hash and Huffman encoding
	hash->save(out);
	saveValue<uint64_t>(out, bytesStrings);
	saveValue<uchar>(out, textStrings, bytesStrings);

	// Decoding Table
	saveValue<Codeword>(out, codewords, 256);
	table->save(out);
}

StringDictionary*
StringDictionaryHASHHF::load(ifstream &in, uint technique)
{
	size_t type = loadValue<uint32_t>(in);

	if (type != HASHHF) return NULL;
	if ((technique != HASHUFF) && (technique != HASHBHUFF) && (technique != HASHBBHUFF)) return NULL;


	StringDictionaryHASHHF *dict = new StringDictionaryHASHHF();
	dict->type = technique;
	dict->elements = loadValue<uint64_t>(in);
	dict->maxlength = loadValue<uint32_t>(in);
	dict->maxcomplength = loadValue<uint32_t>(in);

	dict->hash = Hash::load(in, dict->type);
	dict->bytesStrings = loadValue<uint64_t>(in);
	dict->textStrings = loadValue<uchar>(in, dict->bytesStrings);
	dict->hash->setData(dict->textStrings);

	dict->codewords = loadValue<Codeword>(in, 256);
	dict->table = DecodingTable::load(in);
	dict->coder = new StatCoder(dict->table, dict->codewords);

	return dict;
}

StringDictionaryHASHHF::~StringDictionaryHASHHF()
{
	delete hash;
	delete [] textStrings;
	delete [] codewords;
	delete table; delete coder;
}

