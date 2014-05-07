/* StringDictionary.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Abstract class for implementing Compressed String Dictionaries.
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

#include "StringDictionary.h"

StringDictionary*
StringDictionary::load(ifstream & fp, uint opt)
{
	size_t r = loadValue<uint32_t>(fp);
	fp.seekg(0, fp.beg);

	switch(r)
	{
		case HASHHF:		return StringDictionaryHASHHF::load(fp, opt);
		case HASHUFFDAC:	return StringDictionaryHASHUFFDAC::load(fp);
		case HASHRPF:		return StringDictionaryHASHRPF::load(fp, opt);
		case HASHRPDAC:		return StringDictionaryHASHRPDAC::load(fp);

		case PFC:		return StringDictionaryPFC::load(fp);
		case RPFC:		return StringDictionaryRPFC::load(fp);

		case HTFC:		return StringDictionaryHTFC::load(fp);
		case HHTFC:		return StringDictionaryHHTFC::load(fp);
		case RPHTFC:		return StringDictionaryRPHTFC::load(fp);

		case RPDAC:		return StringDictionaryRPDAC::load(fp);
		case FMINDEX:		return StringDictionaryFMINDEX::load(fp);
		case DXBW:		return StringDictionaryXBW::load(fp);
	}

	return NULL;
}

uint
StringDictionary::maxLength()
{
	return maxlength;
}

size_t
StringDictionary::numElements()
{
	return elements;
}

