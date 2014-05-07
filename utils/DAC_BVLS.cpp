/* DAC_BVLS.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class takes the features of Directly Addresable Codes to perform a
 * a symbol reordering on sequences of bytes:
 *
 *   ==========================================================================
 *     "DACs: Bringing Direct Access to Variable-Length Codes"
 *     Nieves Brisaboa, Susana Ladra, and Gonzalo Navarro.
 *     Information Processing and Management 49(1):392-404, 2013.
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

#include "DAC_BVLS.h"

DAC_BVLS::DAC_BVLS(){
	tamCode = 0;
	nLevels = 0;      
	levelsIndex = NULL; 
	levels = NULL;
	bS = NULL; 
	rankLevels = NULL;
}

DAC_BVLS::DAC_BVLS(uint tamCode, uint nLevels, vector<uint> * levelsIndex, vector<uint> * rankLevels, uchar *levels, BitString *bS){
	this->tamCode = tamCode;
	this->nLevels = nLevels;
	this->levels = levels;
	this->bS = new BitSequenceRG(*bS, 4);
	this->levelsIndex = new uint[nLevels];
	this->rankLevels = new uint[nLevels];

	for (uint i=0; i<nLevels; i++) this->levelsIndex[i] = (*levelsIndex)[i];

	this->rankLevels[0] = 0;
	for (uint i=1; i<nLevels; i++) this->rankLevels[i] = (*rankLevels)[i-1]+this->rankLevels[i-1];
}

uint DAC_BVLS::access(uint pos, uint **seq) const{
	uint *sequence = new uint[nLevels];
	uint l_seq =0;
	uint ini = pos-1; 
	uint j=0;
	uint rankini;

	sequence[j] = levels[ini];
	l_seq =1;

	while(bitget(((BitSequenceRG *)bS)->data,ini)){
		rankini = bS->rank1(ini) - rankLevels[j];
		j++;
			
		ini = levelsIndex[j]+rankini -1;
		sequence[j] = levels[ini];
			
		l_seq++;
		if(j==(uint)nLevels-1)
			break;
	}
	*seq = sequence;
	return l_seq;
}

uint DAC_BVLS::access_next(uint l, uint *pos) const{
	uint ini=*pos-1; 
	uint rankini;
	uint seq = levels[ini];
		
	if(l==(uint)nLevels-1)
	{
		*pos = (uint)-1;
		return seq;
	}

	if(bitget(((BitSequenceRG *)bS)->data,ini))
	{
		rankini = bS->rank1(ini) - rankLevels[l];
		*pos = levelsIndex[l+1]+rankini;
	}
	else *pos = (uint)-1;

	return seq;
}

uint DAC_BVLS::getSize() const
{
	uint mem = sizeof(DAC_BVLS);
	mem += sizeof(uint)*(nLevels+1); //levelsIndex 
	mem += sizeof(uchar)*(tamCode); //levels
	mem += bS->getSize();   
	mem += sizeof(uint)*nLevels; //	rankLevels
	return mem;
}

void DAC_BVLS::save(ofstream & fp) const{
	saveValue(fp, tamCode);
	saveValue(fp, nLevels);
	saveValue(fp, levelsIndex, nLevels+1);
	saveValue(fp, levels, tamCode);
	saveValue(fp, rankLevels, nLevels);
	bS->save(fp);
}

DAC_BVLS* DAC_BVLS::load(ifstream & fp){
	DAC_BVLS *rep = new DAC_BVLS();
	rep->tamCode = loadValue<uint>(fp);
	rep->nLevels = loadValue<uint>(fp);
	rep->levelsIndex = loadValue<uint>(fp, rep->nLevels+1); 
	rep->levels = loadValue<uchar>(fp, rep->tamCode);
	rep->rankLevels = loadValue<uint>(fp, rep->nLevels);
	rep->bS = BitSequence::load(fp);

	return rep;
}

DAC_BVLS::~DAC_BVLS()
{
	delete [] levelsIndex;
	delete [] levels;
	delete [] rankLevels;
	delete (BitSequenceRG *)bS;
}


