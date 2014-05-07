/* BinaryNode.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class models the binary nodes used for building statiscal codes.
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


#ifndef _BINARYNODE_H
#define _BINARYNODE_H

#include <iostream>
using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

class BinaryNode
{		
	public:
		/** Generic Constructor */
		BinaryNode();

		/** Class Constructor.
		    @param type: describes the node type (0=external / 1=internal).
		    @param position: points to the node position in the global 
		      sequence.
		    @param weight: node weight (frequency).
		*/
		BinaryNode(uint type, uint position, uint weight);

		/** Generic Destructor */
		~BinaryNode();

	protected:
		bool type;		// Node type
		int position;		// Node position
		int weight;		// Node weight

		BinaryNode *leftChild;	// Pointer to the (possible) left child
		BinaryNode *rightChild;	// Pointer to the (possible) right child

	friend class HuTucker;
	friend class Huffman;
};





#endif  /* _BINARYNODE_H */

