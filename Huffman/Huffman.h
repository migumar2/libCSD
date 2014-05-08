/* Huffman.h
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

#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <libcdsBasics.h>
#include "huff.h"
#include <Array.h>

#include "../utils/Coder/BinaryNode.h"
#include "../utils/Coder/Codeword.h"
#include "../utils/Coder/DecodingTree.h"

using namespace cds_utils;


    /** Wrapper for the canonical huffman implementation of Gonzalo Navarro.
     *
     *  @author Francisco Claude
     */
    class Huffman{

        public:
            /** Creates the codes for the sequence seq of length n */
            Huffman(uchar * seq, size_t n);
            Huffman(Array & seq);
            Huffman(uint * occ);

            virtual ~Huffman();

            /** Encodes symb into stream at bit-position pos,
             * returns the ending position (bits) */
            virtual size_t encode(uint symb, uint * stream, size_t pos);

            /** decodes into symb from stream at bit-position
             * pos, returns the new position.*/
            virtual size_t decode(uint * symb, uint * stream, size_t pos);

            /** Returns the maximum length of a code */
            virtual size_t maxLength();

            /** Returns the size of the table */
            virtual size_t getSize();

            /** Saves the coder to a file */
            virtual void save(ofstream & fp);

            /** Loads a coder from a file*/
            static Huffman * load(ifstream & fp);


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

        protected:
            Huffman();
            THuffx huff_table;

            BinaryNode *tree;

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

#endif
