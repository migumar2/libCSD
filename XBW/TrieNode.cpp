/* TrieNode.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * TrieNode for the XBW
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


#include "TrieNode.h"

TrieNode::TrieNode(char symbol)
{
	this->symbol = symbol;
	this->last_ch = 0;
	this->parent = 0;
	this->leaf = this->last = false;
}

void
TrieNode::insert(const uchar *str, int len, int *occ, vector<TrieNode*> *nodes)
{
	if(len>=0)
	{
		leaf = false;
		TrieNode * next = children[str[0]];
		if(next == NULL)
		{
			occ[(uint)symbol]++;
			next = new TrieNode(str[0]);
			nodes->push_back(next);
			next->parent = this;
			children[str[0]] = next;

			if(str[0]>=last_ch)
			{
				if(children[last_ch]!=NULL)
					children[last_ch]->last = false;
				last_ch = str[0];
				next->last = true;
			}
		}
		next->insert(str+1,len-1, occ, nodes);
	}
	else
	{
		last = true;
		leaf = true;
	}
}

bool
TrieNode::less(const TrieNode & n) const
{
	assert(this==&n || !(parent==n.parent && symbol==n.symbol));
	if(parent==n.parent && symbol==n.symbol) return true;
	if((uchar)symbol<(uchar)n.symbol) return true;
	if((uchar)symbol>(uchar)n.symbol) return false;
	if(parent==NULL) return true;
	if(n.parent==NULL) return false;
	return (*parent).less(*n.parent);
}

bool
TrieNode::cmp(const TrieNode & n) const
{
	assert(this==&n || !(parent==n.parent && symbol==n.symbol));
	if(parent==n.parent && (uchar)symbol==(uchar)n.symbol) return true;
	if(parent==n.parent && (uchar)symbol<(uchar)n.symbol) return true;
	if(parent==n.parent && (uchar)symbol>(uchar)n.symbol) return false;
	if(parent==NULL) return true;
	if(n.parent==NULL) return false;
	if((*parent).less(*(n.parent))) return true;
	return false;
}

TrieNode::~TrieNode() {
}

