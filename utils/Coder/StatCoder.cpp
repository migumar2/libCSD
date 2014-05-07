/* StatCoder.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements a general encoder/decoder for dealing with Huffman
 * and Hu-Tucker codes.
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

#include "StatCoder.h"

uint
StatCoder::encodeSymbol(uchar symbol, uchar *text, uint *offset)
{
	uint codeword = codewords[(int)symbol].codeword;
	uint bits = codewords[(int)symbol].bits;

	uint processed = 0;
	uint bytes = 0;

	while ((bits-processed) >= (8-(*offset)))
	{
		uchar code = ((codeword << (W-bits+processed)) >> (W-8+(*offset)));
		text[bytes] |= code;

		processed += 8-(*offset);
		*offset = 0; bytes++;
		text[bytes] = 0;
	}

	// Encoding the remaining bits
	if (bits > processed)
	{
		uchar code = ((codeword << (W-bits+processed)) >> (W-8+(*offset)));
		text[bytes] |= code;

		*offset += bits-processed;
	}

	return bytes;
}

uchar*
StatCoder::encodeString(uchar *str, uint strLen, uint *encLen, uint *offset)
{
	uchar *encoded = new uchar[4*strLen];
	*encLen = 0; encoded[*encLen] = 0;
	*offset = 0;

	for (uint i=0; i<strLen; i++)
		*encLen += encodeSymbol(str[i], &(encoded[*encLen]), offset);

	if (*offset > 0) (*encLen)++;

	return encoded;
}

uint
StatCoder::decodeString(ChunkScan* c)
{
	c->extracted = 0;

	uint prevLen = c->strLen;
	uint nextLen = 0;
	bool end = false;

	uint shared = 0;

	// Checking if any char has been extracted in advance
	if (c->advanced != 0)
	{
		// Checking if a full string is encoded in these advanced chars
		c->str[prevLen+c->advanced] = 0;
		nextLen = strlen((char*)(c->str+prevLen));

		if ((nextLen < c->advanced) && (nextLen > 0))
		{
			uint read = prevLen+VByte::decode(&(c->strLen), c->str+prevLen);
			shared = c->strLen;

			uint extracted = prevLen+nextLen;

			for (uint i=read; i<=extracted; i++)
			{
				c->str[c->strLen] = c->str[i];
				c->strLen++;
			}

			nextLen++;
			if (nextLen != c->advanced)
			{
				uint xadv = c->advanced-nextLen;
				extracted++;

				for (uint i=0; i<xadv; i++)
				{
					c->str[c->strLen+i] = c->str[extracted+i];
				}

				c->advanced = xadv;
			}
			else c->advanced = 0;

			return shared;
		}
		else
		{
			c->strLen = prevLen+c->advanced;
			c->extracted = c->advanced;
		}
	}

	// Extracts, at least, the two first bytes because represent the VByte
	// encoding of the prefix length
	while ((c->strLen-prevLen) < 2) end = table->processChunk(c);

	// Appends the extracted chars before the common prefix
	uint extracted = c->strLen-prevLen;
	uint read = VByte::decode(&(c->strLen), c->str+prevLen);
	shared = c->strLen;

	for (uint i=read; i<extracted; i++)
	{
		c->str[c->strLen] = c->str[prevLen+i];
		c->strLen++;
	}

	if (end && (c->advanced > 0))
	{
		// Copying advanced chars
		for (uint i=0; i<c->advanced; i++)
			c->str[c->strLen+i] = c->str[prevLen+extracted+i];
	}

	// Extracts the remaining suffix
	while (!end) end = table->processChunk(c);

	return shared;
}
