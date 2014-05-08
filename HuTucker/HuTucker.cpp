/* HuTucker.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements the HuTucker Code.
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


#include "HuTucker.h"

HuTucker::HuTucker(uint* occs)
{
	this->start=0;
	this->end=255;
	this->max_v = 256;

	this->root=0;
    	this->seq=new BinaryNode*[this->max_v];
	this->internal=0;
	this->levels=new int[this->max_v];

	this->symbols=new bool[this->max_v];
	
	for (int i=start, point=start; i<=end; i++)
	{
		seq[i] = new BinaryNode(0, i, occs[i]);
		point=point+(occs[i]);

		if (occs[i] > 1) symbols[i] = true;
		else symbols[i] = false;
	}
	
	combination();
	levelAssignment(root, 0);
	recombination();
}

Codeword*
HuTucker::obtainCodewords()
{
	Codeword* codewords = new Codeword[max_v];
	uint codeword = 0;

	encodeNode(root, 0, codeword, codewords);

	return codewords;
}

DecodingTree* 
HuTucker::obtainSubtree(uint symbol, uint k)
{
	BinaryNode *node = root;

	// Traversing the Hu-Tucker tree
	for (uint i=1; i<=k; i++)
	{
		bool bit = ((symbol >> (k-i)) & 1);

		if (bit == 0) node = node->leftChild;
		else node = node->rightChild;
	}

	// Retrieving the subtree
	vector<uint> xTree;
	vector<uint> symbols;
	uint bits = 0;
	retrieveSubtree(node, &xTree, &bits, &symbols);

	BitString *tree = new BitString(2*xTree.size());
	for (uint i=0; i<xTree.size(); i++) tree->setBit(xTree[i]);

	return new DecodingTree(symbol, tree, &symbols);
}

void
HuTucker::combination()
{
	uint pmin=0, pcom, temp;

	while(start!=end)
	{
		pmin = findMinimumNode();
		pcom = findCompatibleNode(pmin);
		
		if (pmin > pcom){ temp = pmin; pmin = pcom; pcom = temp; }

		seq[pmin] = mergeNodes(seq[pmin], seq[pcom]);

		// Update the sequence
		for (int i=pcom; i<end; i++)
			seq[i]=seq[i+1];
		end--;

	}

	root = seq[pmin];
}

void
HuTucker::levelAssignment(BinaryNode* node, uint level)
{
	if (node->type == 1)
	{
		// Internal node
		internal++; level++;

		levelAssignment(node->leftChild, level);
		levelAssignment(node->rightChild, level);

		level--;

		delete node;
	}
	else
	{
		// Leaf node
		levels[node->position] = level;
		seq[node->position] = node;
	}
}

void
HuTucker::recombination()
{
	int *stack = new int[max_v];
	int pos = 0;
	int cont = 0;
	stack[pos]=cont++;

	while(cont < max_v)
	{
		pos++;
		stack[pos]=cont++;

		while(pos > 0 && levels[stack[pos]]==levels[stack[pos-1]])
		{
			pos--;
			seq[stack[pos]] = mergeNodes(seq[stack[pos]], seq[stack[pos+1]]);
			levels[stack[pos]]--;
		}
	}

	root = seq[stack[pos]];

	delete[]stack;
}


uint
HuTucker::findMinimumNode()
{
	uint min = start;

	for (int i=min+1; i<=end; i++)
        	if(seq[min]->weight > seq[i]->weight) min=i;
	
	return min;
}

uint
HuTucker::findCompatibleNode(int node)
{
	int minleft = node, minright = node, pcom;

	// Finding in the left side
	if(node != start)
	{
		minleft=node-1;

        	for (int left=node-2; (left >= start) && (seq[left+1]->type != 0); left--)
			if(seq[minleft]->weight >= seq[left]->weight) minleft=left;
	}

	// Finding in the right side
	if(node != end)
	{
		minright=node+1;

		for (int right=node+2; (right<=end) && (seq[right-1]->type != 0); right++)
            		if(seq[minright]->weight > seq[right]->weight) minright=right;
        }

	// Obtaining the minimum between minleft and minright
	if(minleft == node) pcom=minright;
	else
	{
		if(minright == node) pcom=minleft;
	        else pcom = (seq[minleft]->weight <= seq[minright]->weight) ? minleft : minright;
	}

	return pcom;
}

BinaryNode*
HuTucker::mergeNodes(BinaryNode *left, BinaryNode *right)
{
	BinaryNode* n = new BinaryNode(1, left->position, left->weight+right->weight);
	n->leftChild = left;
	n->rightChild = right;

	return n;
}

void
HuTucker::encodeNode(BinaryNode* node, uint level, uint codeword, Codeword *codewords)
{
	if (node->leftChild != NULL)
	{
		{
			// Left child
			encodeNode(node->leftChild, level+1, codeword, codewords);
		}

		{
			// Right child
			bitset(&codeword, WORD-level-1);
			encodeNode(node->rightChild, level+1, codeword, codewords);
		}
	}
	else
	{
		codewords[node->position].codeword = codeword >> (WORD-level);
		codewords[node->position].bits = level;
	}
}

void
HuTucker::retrieveSubtree(BinaryNode* node, vector<uint> *tree, uint *bits, vector<uint> *symbols)
{
	(*bits)++;

	if (node->leftChild != NULL)
	{
		// Left child
		retrieveSubtree(node->leftChild, tree, bits, symbols);

		// Right child
		retrieveSubtree(node->rightChild, tree, bits, symbols);
	}
	else symbols->push_back(node->position);

	tree->push_back(*bits);
	(*bits)++;
}

void 
HuTucker::deleteNode(BinaryNode* node)
{
	if (node->leftChild != NULL) deleteNode(node->leftChild);
	if (node->rightChild != NULL) deleteNode(node->rightChild);

	delete node;
}

HuTucker::~HuTucker()
{
	deleteNode(root);
	delete [] seq; delete [] levels; delete [] symbols;
}

