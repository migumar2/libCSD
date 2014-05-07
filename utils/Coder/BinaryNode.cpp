/* BinaryNode.cpp
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


#include "BinaryNode.h"

BinaryNode::BinaryNode()
{
	this->type = 0;
	this->position = -1;
	this->weight = -1;

	this->leftChild = this->rightChild = NULL;
}

BinaryNode::BinaryNode(uint type, uint position, uint weight)
{
	this->type = type;
	this->position = position;
	this->weight = weight;

	this->leftChild = this->rightChild = NULL;
}

BinaryNode::~BinaryNode()
{
}

