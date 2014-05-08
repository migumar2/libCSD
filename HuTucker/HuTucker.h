/* HuTucker.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements the Hu-Tucker Code for chars.
 *
 *   ==========================================================================
 *     "The Art of Computer Programming, volume 3: Sorting and Searching"
 *     Donald E. Knuth.
 *     Addison Wesley, 1973.
 *   ==========================================================================
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


#ifndef _HUTUCKER_H
#define _HUTUCKER_H

#include <iostream>
using namespace std;

#include <BitString.h>
#include <libcdsBasics.h>
using namespace cds_utils;

#include "../utils/Coder/BinaryNode.h"
#include "../utils/Coder/Codeword.h"
#include "../utils/Coder/DecodingTree.h"

#define WORD 32 // sizeof(uint)

class HuTucker
{		
	public:
		/** Class Constructor.
		    @param occs: number of char occurrences
		*/
		HuTucker(uint* occs);

		/** Retrieves the codeword assignment.
		    @return a reference to an array containing the codeword
		      assignment for each character. 
		*/
		Codeword* obtainCodewords();

		/** Retrieves, from the k-th level, the decoding subtree for 
		    the given smybol.
		    @param symbol: the required symbol.
		    @param k: the root level for the subtree.
		    @returns the decoding tree.
		*/
		DecodingTree* obtainSubtree(uint symbol, uint k);

		/** Generic Destructor */
		~HuTucker();

	protected:
		int start;		//! Start position in the array
		int end;		//! End position in the array

		int max_v;		//! Maximum value used in the text

		BinaryNode* root; 	//! Root of the tree
		BinaryNode** seq;	//! Sequence of nodes
		int internal;		//! Number of internal nodes
		int* levels;		//! Depth of each leaf

		bool* symbols;		//! Symbols used in the code


		/** Implements the first stage of the Hu-Tucker algorithm by
		    building an optimal binary tree */
		void combination();

		/** Implements the second stage of the Hu-Tucker algorithm by
		    determining the levels of the terminal nodes. It performs a
		    recursive tree traversal assigning its corresponding level
		    to each node.
		    @param node: the node being traversed.
		    @param level: the level of the current node. */
		void levelAssignment(BinaryNode* node, uint level);

		/** Implements the third stage of the Hu-Tucker algorithm by
		    builing an optimal alphabetic binary tree from the initial
		    sequence of terminals and their corresponding levels.
		*/
		void recombination();

		/** Finds the node with minimun weight 
		    @returns the position of the node with minimun weight.
		*/
		uint findMinimumNode();

		/** Finds the 'compatible' node for the given one.
		    @param node: the position of the node with minimum weight.
		    @returns the position of the compatible node for the given one.
		*/
		uint findCompatibleNode(int node);

		/** Merge two binary nodes and returns the new resulting node.
		    @param left: the left node to be merged.
		    @param right: the right node to be merged.
		    @returns the merged node.
		*/
		BinaryNode* mergeNodes(BinaryNode *left, BinaryNode *right);

		/** Encodes the node according to its configuration and 
		    traverses recursively its children.
		    @param node: the node being encoded.
		    @param level: node level in the binary tree.
		    @param codeword: the codeword configuration.
		    @param codewords: the global table containing all codewords
		*/
		void encodeNode(BinaryNode* node, uint level, uint codeword, Codeword *codewords);

		/** Retrieves the subtree rooted by the given node.
		    @param node: the subtre root node.
		    @param tree: the subtree representation as balanced 
		      parenthesis.
		    @param bits: number of bits used in the subtree 
		      representation.
		    @param symbols: symbols stored in the tree leafs.
		*/
		void retrieveSubtree(BinaryNode* node, vector<uint> *tree, uint *bits, vector<uint> *symbols);

		/** Free the memory reserved for the node management
		    @para node: the node to be deleted.
		*/
		void deleteNode(BinaryNode* node);
};

#endif  /* _HUTUCKER_H */

