/* Utils.h
 * Copyright (C) 2012, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
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
 *   Francisco Claude:  	fclaude@cs.uwaterloo.ca
 *   Rodrigo Canovas:  		rcanovas@student.unimelb.edu.au
 *   Miguel A. Martinez-Prieto:	migumar2@infor.uva.es
 */


#ifndef _UTILS_H
#define _UTILS_H

#include <string.h>

#include <libcdsBasics.h>
using namespace cds_utils;

inline size_t
Reallocate(uchar** array, size_t len)
{
	size_t llen = len*2;
	uchar *xarr = new uchar[llen];
	memcpy(xarr, *array, len);
	delete [] *array;
	*array = xarr;
	return llen;
}

inline int 
longestCommonPrefix(const uchar* str1, const uchar* str2, uint length, uint *lcp)
{	
	uint ptr = 0;

        for (;ptr<length; ptr++)
		if (str1[ptr] != str2[ptr]) break;

	*lcp += ptr;

        return (str1[ptr] - str2[ptr]);
}


/*
inline int 
longestCommonPrefix(const uchar* str1, const uchar* str2, uint lstr1, uint lstr2, uint *lcp)
{	
        uint length = lstr1;
        if (length > lstr2) length = lstr2;

	uint ptr = 0;

        for (;ptr<length; ptr++)
		if (str1[ptr] != str2[ptr]) break;

	*lcp += ptr;

        return (str1[ptr] - str2[ptr]);
}
*/


#endif  /* _UTILS_H */

