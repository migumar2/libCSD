/* VByte.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * This class implements the Variable Byte (VByte) Code:
 *
 *   ==========================================================================
 *     "Compressing integers for fast file access"
 *     Hugh Williams and Justin Zobel.
 *     The Computer Journal 42, p.193-201, 1999.
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


#ifndef _VBYTE_H
#define _VBYTE_H

#include <iostream>
using namespace std;

#include <libcdsBasics.h>
using namespace cds_utils;

class VByte
{		
  public:
    /** Encodes the integer 'c' in the sequence of bytes (uchar) 'r'.
	@param c: the number to be encoded.
	@param r: the resulting encoded sequence.
	@returns the number of bytes used for encoding.
    */
    static uint encode(uint c, uchar *r);

    /** Decodes the sequence of bytes (uchar) 'r' into the integer 'c'.
	@param c: the resulting decoded number.
	@param r: the sequence to be decoded.
	@returns the number of bytes read for decoding.
    */
    static uint decode(uint *c, uchar *r);
};

#endif  /* _VBYTE_H */

