/* DecodingTableBuilder.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class comprises some utilities for building the decoding table used in
 * Huffman and HuTucker codes.
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

#include "DecodingTableBuilder.h"

DecodingTableBuilder::DecodingTableBuilder(uint maxv)
{
	this->maxv = maxv;
}

DecodingTableBuilder::DecodingTableBuilder(Codeword *codews)
{
	this->maxv = 255;
	codewords = codews;
	tableSubstr = new DecodeableSubstr[(int)pow((float)2,(int)TABLEBITSO)];
	table = new DecodingTable(maxv);

	uint i=0;

	while (i<maxv)
	{
		if (codewords[i].bits > TABLEBITSO)
		{
			// The first k bits are used as key for
			// traversing the Huffman tree
			uint index = codewords[i].codeword >> (codewords[i].bits-TABLEBITSO);

			// Check if the tree is now stored
			if (tableSubstr[index].dbits == 0)
			{
				tableSubstr[index].dbits = 0;
			}
		}

		i++;
	}
}

void
DecodingTableBuilder::initializeFromHuffman(Huffman *huff)
{
	codewords = huff->obtainCodewords();
	tableSubstr = new DecodeableSubstr[(int)pow((float)2,(int)TABLEBITSO)];
	table = new DecodingTable(maxv);

	uint i=0;

	while (i<maxv)
	{
		if (codewords[i].bits > TABLEBITSO)
		{
			// The first k bits are used as key for
			// traversing the Huffman tree
			uint index = codewords[i].codeword >> (codewords[i].bits-TABLEBITSO);

			// Check if the tree is now stored
			if (tableSubstr[index].dbits == 0)
			{
				// Obtaining the subtree and inserting it in the
				// decoding table
				DecodingTree *tree = huff->obtainSubtree(index, TABLEBITSO);

				uint pos = table->setDecodingSubtree(tree);
				tableSubstr[index].setLargeSubstr(pos);
			}
		}

		i++;
	}
}

void
DecodingTableBuilder::initializeFromHuTucker(HuTucker *ht)
{
	maxv++;

	codewords = ht->obtainCodewords();
	tableSubstr = new DecodeableSubstr[(int)pow((float)2,(int)TABLEBITSO)];
	table = new DecodingTable(maxv);

	uint i=0;

	while (i<maxv)
	{
		if (codewords[i].bits > TABLEBITSO)
		{
			// The first k bits are used as key for
			// traversing the Hu-Tucker tree
			uint index = codewords[i].codeword >> (codewords[i].bits-TABLEBITSO);

			// Obtaining the subtree and inserting it in the
			// decoding table
			DecodingTree *tree = ht->obtainSubtree(index, TABLEBITSO);

			uint pos = table->setDecodingSubtree(tree);
			tableSubstr[index].setLargeSubstr(pos);

			i = tree->getLastSymbol();
		}
		else i++;
	}
}


void
DecodingTableBuilder::insertDecodeableSubstr(uchar symbol, uint *seq, ushort *ptr, vector<uchar> *substr, vector<ushort> *lenSubstr)
{
	uint bits = codewords[(int)symbol].bits;
	uint codeword = codewords[(int)symbol].codeword;

	if (bits <= TABLEBITSO)
	{
		*seq = (*seq << bits) | codeword;
		*ptr += bits;
	}
	else
	{
		*seq = (*seq << TABLEBITSO) | (codeword >> (bits-TABLEBITSO));
		*ptr += TABLEBITSO;
	}

	if (*ptr >= TABLEBITSO)
	{
		uint index = (*seq >> (*ptr-TABLEBITSO)) & mask(TABLEBITSO);

		if (bits > TABLEBITSO) *ptr += (bits-TABLEBITSO);

		if ((substr->size() > 0) && (tableSubstr[index].dbits <= 1))
		{
			// This substring has not been previously indexed
			if ((*ptr == TABLEBITSO) && (bits <= TABLEBITSO))
			{
				// The encoded symbol is fully represented in
				// the current chunk
				substr->push_back(symbol); lenSubstr->push_back(bits);
				tableSubstr[index].setSubstr(substr, substr->size(), *ptr);

				*ptr = 0;
				substr->clear(); lenSubstr->clear();
			}
			else
			{
				// The encoded symbol is not represented in
				// the current chunk
				tableSubstr[index].setSubstr(substr, substr->size(), *ptr-bits);

				substr->clear(); lenSubstr->clear();
				*ptr = bits;
				substr->push_back(symbol); lenSubstr->push_back(bits);

				if (*ptr >= TABLEBITSO)
				{
					// The encoded symbol fills the chunk
					// and must be indexed if its length is
					// equals to TABLEBITSO. Otherwise, its
					// corresponding subtree is already in
					// the table.

					if (*ptr == TABLEBITSO)
					{
						index = codeword;
						if (tableSubstr[index].length == 0)
							tableSubstr[index].setSubstr(substr, 1, TABLEBITSO);
					}

					*ptr = 0;
					substr->clear(); lenSubstr->clear();
				}
			}
		}
		else
		{
			if (substr->size() == 0)
			{
				substr->clear(); lenSubstr->clear();

				*ptr = bits;
				substr->push_back(symbol); lenSubstr->push_back(bits);

				if (*ptr == TABLEBITSO)
				{
					index = codeword;

					if (tableSubstr[index].length == 0)
						tableSubstr[index].setSubstr(substr, 1, TABLEBITSO);
				}

				*ptr = 0;
				substr->clear(); lenSubstr->clear();
			}
			else
			{
				substr->clear(); lenSubstr->clear();

				if ((*ptr == TABLEBITSO) && (bits <= TABLEBITSO)) *ptr = 0;
				else
				{
					*ptr = bits;
					substr->push_back(symbol); lenSubstr->push_back(bits);

					if (*ptr >= TABLEBITSO)
					{
						if (*ptr == TABLEBITSO)
						{
							index = codeword;
							if (tableSubstr[index].length == 0)
								tableSubstr[index].setSubstr(substr, 1, TABLEBITSO);
						}

						*ptr = 0;
						substr->clear(); lenSubstr->clear();
					}
				}
			}
		}
	}
	else
	{
		substr->push_back(symbol);
		lenSubstr->push_back(bits);
	}
}

void
DecodingTableBuilder::insertEndingSubstr(uint *seq, ushort *ptr, vector<uchar> *substr, vector<ushort> *lenSubstr)
{
	uint bits = TABLEBITSO-(*ptr);
	*seq = (*seq << bits);
	uint index = (*seq) & mask(TABLEBITSO);

	if (tableSubstr[index].length == 0)
		tableSubstr[index].setSpecialSubstr(substr, substr->size());

	*ptr = 0;
	substr->clear();
	lenSubstr->clear();
}

Codeword*
DecodingTableBuilder::getCodewords()
{
	return codewords;
}

uint
DecodingTableBuilder::getMax()
{
	return maxv;
}

DecodingTable*
DecodingTableBuilder::getTable()
{
	table->setDecodingTable(TABLEBITSO, tableSubstr);
	return table;
}

DecodeableSubstr*
DecodingTableBuilder::getTableSubstr()
{
	return tableSubstr;
}

DecodingTableBuilder::~DecodingTableBuilder()
{
	delete [] tableSubstr;
}
