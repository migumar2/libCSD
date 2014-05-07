/* Utils.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Utilities for String Dictionaries.
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


#ifndef _UTILS_H
#define _UTILS_H

#include <string.h>
#include <sys/time.h>
#include <sys/stat.h> 
#include <sys/types.h> 

#include <libcdsBasics.h>
using namespace cds_utils;

#define GET_TIME_DIVIDER ((double) 1.0 )	//getTime deals with "seconds" as time unit.
#define GET_TIME_UNIT "sec"
#define MSEC_TIME_DIVIDER  ((double) 1000.0)	//1 sec = 1000 msec
#define MSEC_TIME_UNIT 	  "msec"
#define SEC_TIME_DIVIDER  ((double) 1.0	)	//1 sec = 1 sec
#define SEC_TIME_UNIT 	  "sec"
#define MCSEC_TIME_DIVIDER  ((double)1000000)	//1 microsec = 0.000001 sec
#define MCSEC_TIME_UNIT 	  "microsec"

static const size_t NORESULT = 0;

/* Hash-based dictionaries */
static const uint32_t HASHHF      = 11; 	// Familiy of Hash-Huffman dictionaries
static const uint32_t HASHUFFDAC  = 114; 	// HashDAC-Huffman dictionary
static const uint32_t HASHRPF     = 12; 	// Familiy of Hash-RePair dictionaries
static const uint32_t HASHRPDAC   = 124; 	// HashDAC-RePair dictionary

static const uint32_t HASHRP      = 1; 		// Hash-RePair dictionary (used for loading purposes)
static const uint32_t HASHBRP     = 2; 		// HashB-RePair dictionary (used for loading purposes)
static const uint32_t HASHBBRP    = 3; 		// HashBB-RePair dictionary (used for loading purposes)

static const uint32_t HASHUFF     = 1; 		// Hash-Huffman dictionary (used for loading purposes)
static const uint32_t HASHBHUFF   = 2; 		// HashB-Huffman dictionary (used for loading purposes)
static const uint32_t HASHBBHUFF  = 3; 		// HashBB-Huffman dictionary (used for loading purposes)




/* Front-Coding based dictionaries */
static const uint32_t PFC    = 211; 		// Plain Front-Coding dictionary
static const uint32_t RPFC   = 214; 		// Plain Front-Coding dictionary (with RePair for suffixes)
static const uint32_t HTFC   = 221; 		// HuTucker Front-Coding dictionary
static const uint32_t HHTFC  = 222; 		// HuTucker Front-Coding dictionary (with Huffman for suffixes)
static const uint32_t RPHTFC = 223; 		// HuTucker Front-Coding dictionary (with RePair for suffixes)

/* RePair+DAC dictionary */
static const uint32_t RPDAC = 3;		// RePair+DAC dictionary

/* FM-Index based dictionaries */
static const uint32_t FMINDEX = 4;		// FM-Index dictionary (with plain: RG and compressed: RRR variants)

/* FM-Index based dictionaries */
static const uint32_t DXBW = 5;			// XBW dictionary (with plain: RG and compressed: RRR variants)


inline uint
encodeVB2(uint c, uchar *r)
{
	unsigned int i= 0;

	while (c>127)
	{
		r[i] = (unsigned char)(c&127);
		i++;
		c>>=7;
	}

	r[i] = (unsigned char)(c|0x80);
	i++;

	return i;
}
	
inline uint
decodeVB2(uint *c, uchar *r)
{
	*c = 0;
	int i = 0;
	int shift = 0;

	while ( !(r[i] & 0x80) )
	{
		*c |= (r[i] & 127) << shift;
		i++;
		shift+=7;
	}

	*c |= (r[i] & 127) << shift;
	i++;

	return i;
}


inline ushort
mask(uint k) { return (65535u >> (16u-k)); }

inline size_t
Reallocate(uchar** array, size_t len)
{
	size_t llen = len*2;
	uchar *xarr = new uchar[llen];
	memcpy(xarr, *array, len);
	delete [] *array;

	// This initialization is due to Valgrind warnings
	for (uint i=len; i<llen; i++) xarr[i] = 0;

	*array = xarr;

	return llen;
}

inline size_t
Reallocate(int** array, size_t len)
{
	size_t llen = len*2;
	int *xarr = new int[llen];
	memcpy(xarr, *array, len*sizeof(int));
	delete [] *array;

	// This initialization is due to valgrind warnings
	for (uint i=len; i<llen; i++) xarr[i] = 0;

	*array = xarr;

	return llen;
}

inline int 
longestCommonPrefix(const uchar* str1, const uchar* str2, uint length, uint *lcp)
{	
	uint ptr = 0;

	for (;ptr<length; ptr++)
	{
		if (str1[ptr] != str2[ptr])
		{
			*lcp += ptr;
			return (str1[ptr] - str2[ptr]);;
		}
	}

	*lcp += ptr;

	return 0;
}

inline double
getTime(void)
{
	struct rusage buffer;
	struct timeval tp;
	struct timezone tzp;

	getrusage( RUSAGE_SELF, &buffer );
	gettimeofday( &tp, &tzp );

	return (double) buffer.ru_utime.tv_sec + 1.0e-6 * buffer.ru_utime.tv_usec;
}


#endif  /* _UTILS_H */

