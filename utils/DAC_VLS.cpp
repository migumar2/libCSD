/* DAC_VLS.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class takes the features of Directly Addresable Codes to perform a
 * a symbol reordering on sequences of symbols:
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


#include "DAC_VLS.h"

DAC_VLS::DAC_VLS(){
	tamCode = 0;
	base_bits = 0;
	listLength = 0;    
	nLevels = 0;      
	levelsIndex = NULL; 
	levels = NULL;   
	bS = NULL; 
	rankLevels = NULL;
}

DAC_VLS::DAC_VLS(int *list, uint l_Length, uint log_r, uint max_seq_length){
	uint *levelSizeAux;
	uint *contB;
	uint bits_BS_len = 0;
	listLength =0;
	nLevels = max_seq_length;
	levelSizeAux = new uint[nLevels];
	
	for(uint i=0;i<nLevels;i++)
		levelSizeAux[i]=0;

	//Space needed for each level
	for(uint i=0;i<l_Length;i++){
		for(uint j=0; j < nLevels;j++){
			if(list[i] >= 0){
				levelSizeAux[j]++;
				i++;
			}
			else
				break;
		}
		listLength++;
	}

	levelsIndex = new uint[nLevels+1];
	bits_BS_len =0;
	base_bits = log_r;
	//space needed for levels in bits
	uint tamLevels =0;
	for(uint i=0;i<nLevels;i++)
		tamLevels+=base_bits*levelSizeAux[i];	
	tamCode=tamLevels;

	levelsIndex[0]=0;
	contB = new uint[nLevels];
	for(uint j=0;j<nLevels;j++){
		levelsIndex[j+1]=levelsIndex[j] + levelSizeAux[j];
		contB[j]=levelsIndex[j];
	}

	//init levels
	levels = new uint[tamLevels/W+1];		
	for(uint i=0; i< (tamLevels/W+1); i++)
		levels[i] = 0;

	//size of the bitmap
	bits_BS_len = levelsIndex[nLevels-1]+1; 
	//tha last position is 0. this save us time later in the queries
	//init bitmap
	uint * bits_BS = new uint[bits_BS_len/W+1];
	for(uint i=0; i<((bits_BS_len)/W+1);i++)
		bits_BS[i]=0;

	for(uint i=0;i<l_Length;i++){

		for(uint j=0; j<nLevels; j++){
			if(list[i]>=0){
				set_field(levels, base_bits, contB[j], (uint)list[i]);
				contB[j]++;
				i++;
				if(j>0)
					bitset(bits_BS, contB[j-1]-1);
			}
			else
				break;
		}
	}

	//set the bitmap data structure
	bitset(bits_BS,bits_BS_len-1);
	bS = new BitSequenceRG(bits_BS, bits_BS_len , 4); 	
	rankLevels = new uint[nLevels];
	rankLevels[0]=0;
	for(uint j=1;j<nLevels;j++)
		rankLevels[j]= bS->rank1(levelsIndex[j]-1);
	
	delete [] contB;
	delete [] levelSizeAux;
	delete [] bits_BS;
}

uint DAC_VLS::access(uint pos, uint **seq) const{
	uint *sequence = new uint[nLevels];
	uint l_seq =0;
	uint ini = pos-1; 
	uint j=0;
	uint rankini;

	sequence[j] = get_field(levels, base_bits, ini); 
	l_seq =1;

	while(bitget(((BitSequenceRG *)bS)->data,ini)){
		rankini = bS->rank1(ini) - rankLevels[j];
		j++;
			
		ini = levelsIndex[j]+rankini -1;
		sequence[j] = get_field(levels, base_bits, ini);			
			
		l_seq++;
		if(j==(uint)nLevels-1)
			break;
	}
	*seq = sequence;
	return l_seq;
}

uint DAC_VLS::access_next(uint l, uint *pos) const{
	uint ini=*pos-1; 
	uint rankini;
	uint seq = get_field(levels, base_bits, ini);
		
	if(l==(uint)nLevels-1){
		*pos = (uint)-1;
		return seq;
	}
	if(bitget(((BitSequenceRG *)bS)->data,ini)){
		rankini = bS->rank1(ini) - rankLevels[l];
		*pos = levelsIndex[l+1]+rankini;
	}
	else
		*pos = (uint)-1;
	return seq;
}

uint DAC_VLS::getListLength() const{
	return listLength;
}

uint DAC_VLS::getSize() const{
	uint mem = sizeof(DAC_VLS);
	mem += sizeof(uint)*(nLevels+1);
	mem += sizeof(uint)*(tamCode/W+1);
	mem += bS->getSize();   
	mem += sizeof(uint)*nLevels; 
	return mem;
}

void DAC_VLS::save(ofstream & fp) const{
	saveValue(fp, tamCode);
	saveValue(fp, listLength);
	saveValue(fp, nLevels);
	saveValue(fp, base_bits);
	saveValue(fp, levelsIndex, nLevels+1);
	saveValue(fp, levels, tamCode/W+1);
	saveValue(fp, rankLevels, nLevels);
	bS->save(fp);
}

DAC_VLS* DAC_VLS::load(ifstream & fp){
	DAC_VLS *rep = new DAC_VLS();
	rep->tamCode = loadValue<uint>(fp);
	rep->listLength = loadValue<uint>(fp);
	rep->nLevels = loadValue<uint>(fp);
	rep->base_bits = loadValue<ushort>(fp);
	rep->levelsIndex = loadValue<uint>(fp, rep->nLevels+1); 
	rep->levels = loadValue<uint>(fp, rep->tamCode/W+1);
	rep->rankLevels = loadValue<uint>(fp, rep->nLevels);
	rep->bS = BitSequence::load(fp);
	return rep;
}

DAC_VLS::~DAC_VLS(){
	delete [] levelsIndex;
	delete [] levels;
	delete [] rankLevels;
	delete (BitSequenceRG *)bS;
}


