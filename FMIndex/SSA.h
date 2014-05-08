/* SSA.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
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

#ifndef SSA_WORDS_H
#define SSA_WORDS_H

#include <SequenceBuilder.h>
#include <Sequence.h>
#include <BitSequenceBuilder.h>
#include <BitSequence.h>

#include <Mapper.h>
#include <algorithm>

#include "SuffixArray.h"

using namespace std;
using namespace cds_static;

	class SSA{
		public:
			SSA(uchar * seq, uint n, bool free_text=false, uint samplesuff=0);
			SSA();
			~SSA();

			bool set_static_sequence_builder(SequenceBuilder * ssb);
			bool set_static_bitsequence_builder(BitSequenceBuilder * sbb);

			bool build_index();

			uint size();
			void print_stats();
			uint length();

			uint LF(uint i);
			uint locate_id(uchar * pattern, uint m);
			uint locate(uchar * pattern, uint m, size_t **occs);
			uint locateP(uchar * pattern, uint m, size_t *left, size_t *right, size_t last);

			uchar * extract_id(uint id, uint *strLen, uint max_len);
			static SSA * load(ifstream & fp);
			void save(ofstream & fp);

		protected:
			uint n;
			Sequence * bwt;

			BitSequence * sampled;
			uint * suff_sample;
			uint samplesuff;			// Suffix sampling

			uint * occ;
			uint maxV;
			bool built;
			bool free_text;
			bool *alphabet;	

			/*use only for construction*/
			uchar * _seq;
			uint * _bwt;   
			ulong * _sa;   
			SequenceBuilder * _ssb;
			BitSequenceBuilder * _sbb;
			/*******************************/

			void build_bwt();
			void build_sa();
			int cmp(uint i, uint j);

		friend class StringDictionaryFMINDEX;
	};

#endif
