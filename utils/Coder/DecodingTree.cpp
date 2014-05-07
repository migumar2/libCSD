/* DecodingTree.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a (sub)tree representation which encodes a set of
 * codeword from a root prefix.
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


#include "DecodingTree.h"

DecodingTree::DecodingTree(uint codeword, BitString *partree, vector<uint> *symbols)
{
	this->codeword = codeword;
	this->partree = partree;
	this->symbols = *symbols;
	this->leaves = 0;

	buildTree(partree->getLength());
}

size_t 
DecodingTree::getSize()
{
	return nodes*sizeof(TreeNode)+sizeof(DecodingTree)+leaves*sizeof(uint);
}

void 
DecodingTree::save(ofstream &out)
{
	saveValue<uint>(out, codeword);
	saveValue<uint>(out, leaves);
	partree->save(out);
	for (uint i=0; i<leaves; i++) saveValue<uint>(out, symbols[i]);

	delete partree;
}


DecodingTree*
DecodingTree::load(ifstream &in)
{
	DecodingTree *table = new DecodingTree();

	table->codeword = loadValue<uint>(in);
	table->leaves = loadValue<uint>(in);

	table->partree = new BitString(in);
	for (uint i=0; i<table->leaves; i++) table->symbols.push_back(loadValue<uint>(in));
	table->buildTree(table->partree->getLength());
	delete table->partree;

	return table;
}


void
DecodingTree::buildTree(uint bits)
{
	leaves = 0;
	nodes = bits/2;
	tree = new TreeNode[nodes];

	uint xNodes=0, ptr=0;
	vector<uint> queue;

	while (ptr < bits)
	{
		if (partree->getBit(ptr) == 0)
		{
			// New node: checking its ancestor
			uint level = queue.size();

			if (level > 0)
			{
				// Left child
				if (tree[queue[level-1]].children[0] == -1) tree[queue[level-1]].children[0] = xNodes;
				// Right child
				else tree[queue[level-1]].children[1] = xNodes;		
			}

			queue.push_back(xNodes);
			xNodes++;
		}
		else
		{
			uint position = queue.size()-1;

			// Leaf node
			if (tree[queue[position]].children[0] == -1)
			{
				tree[queue[position]].symbol = symbols[leaves];
				leaves++;
			}

			queue.pop_back();
		}

		ptr++;
	}
}


DecodingTree::~DecodingTree()
{
	delete [] tree;
}
