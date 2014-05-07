/* DecodingTable.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a compact table for decoding prefix-based codes. 
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


#include "DecodingTable.h"

void
DecodingTable::setDecodingTable(uint k, DecodeableSubstr* substrs)
{
	assert (k <= MAXK);

	this->k = k;
	this->bytesStream = 0;

	// Scanning the table and building its compact representation
	uint entries = pow(2, this->k);
	endings = new BitString(entries);

	map<vector<uchar>, uint> tmp;
	map<vector<uchar>, uint>::iterator it;

	// Collecting all different decodeable substrings in the representation
	for (uint i=0; i<entries; i++)
	{
		if (substrs[i].length > 0)
		{
			// Check if the entry has been previously found
			it = tmp.find(substrs[i].substr);

			if (it == tmp.end())
			{
				// New substring
				tmp.insert(pair<vector<uchar>, uint> (substrs[i].substr, i));
			}
			else
			{
				// Although the substring was found, we must
				// check its jumping information because an
				// end-substring can be similar than a regular
				// one and its information must prevail
				if (substrs[it->second].special)
				{
					tmp.erase(substrs[i].substr);
					tmp.insert(pair<vector<uchar>, uint> (substrs[i].substr, i));
				}
			}
		}
	}

	table = new uint[entries];

	size_t reservedStream = entries;
	stream = new uchar[reservedStream];
	stream[bytesStream] = (uchar)0; bytesStream++;

	// Serializing the table
	for (uint i=0; i<entries; i++)
	{
		if (substrs[i].dbits > 0)
		{
			// Checking the available space in the stream 
			// and realloc if required
			while ((bytesStream+substrs[i].dbits+1) > reservedStream)
				reservedStream = Reallocate(&stream, reservedStream);

			if (substrs[i].length > 0)
			{
				// Regular entry
				it = tmp.find(substrs[i].substr);

				if (!substrs[it->second].encoded)
				{
					table[i] = bytesStream;
					substrs[it->second].position = table[i];
					substrs[it->second].encoded = true;

					stream[bytesStream] = encodeInfo(substrs[i].length, substrs[it->second].dbits);
					bytesStream++;

					for (uint j=0; j<substrs[i].length; j++)
						stream[bytesStream+j] = substrs[i].substr[j];
					bytesStream += substrs[i].length;
				}
				else table[i] = substrs[it->second].position;

				if (substrs[it->second].ending) endings->setBit(i);
			}
			else
			{
				// Entry pointing to a decoding subtree
				table[i] = bytesStream;

				stream[bytesStream] = encodeInfo(substrs[i].length, substrs[i].dbits);
				bytesStream++;

				bytesStream += VByte::encode(substrs[i].ptr, stream+bytesStream);
			}
		}
		else table[i] = 0;
	}
}

uint
DecodingTable::setDecodingSubtree(DecodingTree *tree)
{
	subtrees[nodes] = tree;
	return nodes++;
}

bool
DecodingTable::getSubstring(ChunkScan *c)
{
	uint index = (uint)((c->c_chunk >> (c->c_valid-k)) & mask(k));

	uint position = table[index];
	Entry x = ventry[stream[position]];

	if (x.length != 0)
	{
		c->c_valid -= x.bits;
		position++;

		for (uint i=0; i<x.length; i++) { c->str[c->strLen+i] = stream[position+i]; }
		c->extracted += x.length;

		if (c->extracted <= 2)
		{
			c->strLen += x.length;
			c->advanced += x.length;
			return false;
		}
		else
		{
			if (endings->getBit(index))
			{
				uint substrLen = strlen((char*)&(stream[position]))+1;

				c->strLen += substrLen;
				c->advanced = x.length-substrLen;
				return true;
			}
			else
			{
				c->strLen += x.length;
				return false;
			}
		}

	}
	else
	{
		// The current index prefixes a large codeword: the 
		// corresponding subtree must be traversed for decoding the 
		// symbol
		position++;

		uint idTree;
		VByte::decode(&idTree, &(stream[position]));

		DecodingTree *tree = subtrees[idTree];
		TreeNode node = tree->tree[0];

		c->c_valid -= k;

		while (node.symbol == -1)
		{
			if (c->c_valid == 0)
			{
				c->c_chunk = (c->c_chunk << 8) | *(c->b_ptr);
				c->c_valid += 8; c->b_ptr++; c->b_remain--;
			}

			c->c_valid--;

			bool bit = (c->c_chunk >> (c->c_valid)) & 1;
			node = tree->tree[node.children[bit]];
		}

		c->str[c->strLen] = node.symbol; 
		c->strLen++;
		c->extracted++;

		if (node.symbol == 0) return true;
		else return false;
	}
}


bool
DecodingTable::processChunk(ChunkScan *c)
{
	while (c->c_valid < k)
	{
		if (c->b_remain == 0)
		{
			if (c->c_valid < k )
				c->c_chunk = c->c_chunk << (k-c->c_valid);
			else
				c->c_chunk = c->c_chunk >> (c->c_valid-k);

			c->c_valid=k;
		}
		else
		{
			c->c_chunk = (c->c_chunk << 8) | *(c->b_ptr);
			c->c_valid += 8; c->b_ptr++; c->b_remain--;
		}
	}

	return getSubstring(c);
}

size_t 
DecodingTable::getSize()
{
	uint treesize = 0;
	for (uint i=0; i<nodes; i++) treesize += subtrees[i]->getSize();

	return pow(2,k)*sizeof(uint)+bytesStream*sizeof(uchar)+endings->getSize()+treesize+sizeof(DecodingTable);
}

void 
DecodingTable::save(ofstream &out)
{
	saveValue<uint32_t>(out, k);

	saveValue<uint64_t>(out, bytesStream);
	saveValue<uchar>(out, stream, bytesStream);

	saveValue<uint>(out, table, pow(2, k));
	endings->save(out);
	saveValue<uint32_t>(out, nodes);

	for (uint i=0; i<nodes; i++) subtrees[i]->save(out);
}


DecodingTable*
DecodingTable::load(ifstream &in)
{
	DecodingTable *table = new DecodingTable();

	table->k = loadValue<uint32_t>(in);
	table->bytesStream = loadValue<uint64_t>(in);
	table->stream = loadValue<uchar>(in, table->bytesStream);

	table->table = loadValue<uint>(in, pow(2, table->k));
	table->endings = new BitString(in);

	table->nodes = loadValue<uint32_t>(in);
	table->subtrees = new DecodingTree*[table->nodes];
	for (uint i=0; i<table->nodes; i++) table->subtrees[i]=DecodingTree::load(in);

	for (uint i=0; i<255; i++)
	{
		table->ventry[i].length = ((i & 240) >> 4);
		table->ventry[i].bits = ((i & 15)+1);;
	}

	return table;
}


inline uchar 
DecodingTable::encodeInfo(uint length, uint bits)
{
	return (uchar)((length << 4) | (bits-1));
}

inline void
DecodingTable::decodeInfo(uchar code, uint *length, uint *bits)
{
	*length = ((code & 240) >> 4);
	*bits = ((code & 15)+1);
}

DecodingTable::~DecodingTable()
{
	delete [] stream;
	delete [] table; 
	delete endings;

	for (uint i=0; i<nodes; i++) delete subtrees[i];
	delete [] subtrees;
}
