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


#ifndef _DECTREE_H
#define _DECTREE_H

#include <assert.h>
#include <string.h>

#include <fstream>
#include <iostream>
using namespace std;

#include <BitString.h>
#include <libcdsBasics.h>
using namespace cds_utils;


class TreeNode
{
	public:
		/** Generic constructor. */
		TreeNode()
		{ 
			this->symbol = -1;
			this->children[0] = -1;  
			this->children[1] = -1; 
		}

		/** Generic destructor. */
		~TreeNode() { }

	private:
		int symbol;		// The symbol represented in the node
		int children[2];	// The node children

	friend class DecodingTree;
	friend class DecodingTable;
	friend class IteratorDictStringHTFC;
	friend class IteratorDictStringHHTFC;
};

class DecodingTree
{	
	public:
		/** Generic constructor. */
		DecodingTree() { this->leaves = 0; this->tree = NULL; };

		/** Class constructor. 
		    @param codeword: the root prefix.
		    @param first: the first encoded symbol.
		    @param partree: the tree structure represented with 
		      balanced parenthesis.
		    @param symbols: symbols stored in the tree leafs.
		*/
		DecodingTree(uint codeword, BitString *partree, vector<uint> *symbols);

		/** Returns the last symbol encoded by the tree
		*/
		uint getLastSymbol() { return symbols[leaves-1]+1; };

		/** Computes the size of the structure in bytes. 
		    @returns the dictionary size in bytes.
		*/
		size_t getSize();

		/** Stores the decoding tree structure into an ofstream.
		    @param out: the oftstream.
		*/
		void save(ofstream &out);

		/** Loads a decoding tree structure from an ifstream.
		    @param in: the ifstream.
		    @returns the loaded decoding table.
		*/
		static DecodingTree *load(ifstream &in);

		/** Generic destructor. */
		~DecodingTree();

	protected:
		uint codeword;			//! Root prefix
		uint leaves;			//! Number of symbols (leaves)
		uint nodes;			//! Total nodes in the tree

		TreeNode *tree;			//! The subtree structure represented with pointers
		BitString *partree;		//! The subtree structure represented with balanced parenthesis
		vector<uint> symbols;		//! Symbols stored in the tree leafs.

		/** Builds the pointer-based representation of the tree from
		    it counterpart based on balanced parenthesis. 
		    @param bits: number of bits used in the representation.
		    @param symbols: symbols stored in the tree leafs.
		*/
		void buildTree(uint bits);

	friend class DecodingTable;
	friend class IteratorDictStringHTFC;
	friend class IteratorDictStringHHTFC;
};

#endif  /* _DECTREE_H */

