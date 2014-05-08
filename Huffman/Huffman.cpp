/* Huffman.cpp
   Copyright (C) 2008, Francisco Claude, all rights reserved.

   Wrapper for huff written by Gonzalo Navarro

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "Huffman.h"

    Huffman::Huffman(uchar * symb, size_t n) {
    	tree = NULL;
        uchar max_v = 255;
        uint * occ = new uint[max_v+1];

        for(size_t i=0;i<(uint)max_v+1;i++)
            occ[i] = 1; // OJO lo inicializo a 1 por la tabla de decodificacion...
        for(size_t i=0;i<n;i++)
            occ[symb[i]]++;

        huff_table = createHuff(occ, max_v);
	delete [] occ;
    }

    Huffman::Huffman(uint * occs) {
    	tree = NULL;
    	huff_table = createHuff(occs, 255);
    }

    Huffman::Huffman(Array & seq) {
    	tree = NULL;

    	uint max_v = 255;
        uint * occ = new uint[max_v+1];
        for(size_t i=0;i<(uint)max_v+1;i++)
            occ[i] = 0;
        for(size_t i=0;i<seq.getLength();i++)
            occ[seq[i]]++;
        huff_table = createHuff(occ, max_v);
        delete [] occ;
    }

    Huffman::Huffman() {
    	tree = NULL;
    }

    Huffman::~Huffman() {
        freeHuff(huff_table);

        if (tree != NULL) deleteNode(tree);
    }

    size_t Huffman::maxLength(){
        return huff_table.depth;
    }

    size_t Huffman::getSize(){
        return sizeof(Huffman)+sizeHuff(huff_table);
    }

    size_t Huffman::encode(uint symb, uint * stream, size_t pos){
        return encodeHuff(huff_table, symb, stream, pos);
    }

    size_t Huffman::decode(uint * symb, uint * stream, size_t pos){
        return decodeHuff(huff_table, symb, stream, pos);
    }

    void Huffman::save(ofstream & fp){
        saveHuff(huff_table,fp);
    }

    Huffman * Huffman::load(ifstream & fp) {
        Huffman * ret = new Huffman();
        ret->huff_table = loadHuff(fp);
        return ret;
    }

    Codeword*
    Huffman::obtainCodewords()
    {
    	// *************************************
    	// This implementation is optimized for managing codewords up to 32 bits.
    	// *************************************

    	// Codewords are right-aligned to be compliant with the decoding table
    	Codeword* codewords = new Codeword[huff_table.max+1];

    	// The Huffman tree is rebuilt for the new code assignment
    	tree = new BinaryNode();

    	uint *stream = new uint[256];
    	for (uint i=0; i<256; i++) stream[i] = 0;

    	//for (uint i=0; i<=huff_table.max; i++)
    	for (uint i=0; i<=255; i++)
    	{
    		BinaryNode *current = tree;

    		codewords[i].bits = encodeHuff(huff_table, i, stream, (size_t)0);
    		codewords[i].codeword = 0;

    		for (uint j=0; j<codewords[i].bits; j++)
    		{
    			bool bit = (stream[0] >> j) & 1;
    			codewords[i].codeword = codewords[i].codeword << 1;
    			codewords[i].codeword = (codewords[i].codeword | bit);

    			if (bit == 0)
    			{
    				if (current->leftChild == NULL) current->leftChild = new BinaryNode();
    				current = current->leftChild;
    			}
    			else
    			{
    				if (current->rightChild == NULL) current->rightChild = new BinaryNode();
    				current = current->rightChild;
    			}
    		}

    		current->position = i;
    		stream[0] = 0;
    	}

    	delete [] stream;
    	return codewords;
    }

    DecodingTree*
    Huffman::obtainSubtree(uint symbol, uint k)
    {
    	BinaryNode *node = tree;

    	// Traversing the Huffman tree
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
    Huffman::retrieveSubtree(BinaryNode* node, vector<uint> *tree, uint *bits, vector<uint> *symbols)
    {
    	(*bits)++;

    	// Left child
    	if (node->leftChild != NULL) retrieveSubtree(node->leftChild, tree, bits, symbols);
    	// Right child
    	if (node->rightChild != NULL) retrieveSubtree(node->rightChild, tree, bits, symbols);

    	if ((node->leftChild == NULL) && (node->rightChild == NULL))
    		symbols->push_back(node->position);

    	tree->push_back(*bits);
    	(*bits)++;
    }

    void
    Huffman::deleteNode(BinaryNode* node)
    {
    	if (node->leftChild != NULL) deleteNode(node->leftChild);
    	if (node->rightChild != NULL) deleteNode(node->rightChild);

    	delete node;
    }
