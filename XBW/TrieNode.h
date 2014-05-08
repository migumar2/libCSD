/* TrieNode.h
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

#ifndef TRIENODE_H_
#define TRIENODE_H_

#include <iostream>
#include <vector>
#include <map>
#include <cstring>
using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

class TrieNode {

	public:
		TrieNode(char symbol);

		void insert(const uchar *str, int len, int *occ, vector<TrieNode*> *nodes);
		bool less(const TrieNode & n) const;
		bool cmp(const TrieNode & n) const;

		virtual ~TrieNode();

	protected:
		uchar symbol;
		map<char,TrieNode*> children;
		TrieNode *parent;
		uchar last_ch;
		bool leaf;
		bool last;

	friend class StringDictionaryXBW;
};

#endif /* TRIENODE_H_ */
