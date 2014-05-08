/* HashUtils.h
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Utilities for compressed hash dictionaries.
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


#ifndef _HASHUTILS_H
#define _HASHUTILS_H

struct SortString {
	size_t original;	// Original string position in Tdict
	size_t hash;		// String position in the hash table
};

/* Bitwise hash function.  Note that tsize does not have to be prime. */
inline size_t
bitwisehash(uchar *word, size_t len, size_t htsize)
{
	uint h = (uint)(4294967279u);
	int c;
	for(size_t i= 0; i<len; i++){
		c= word[i];
		h = (((h << 15) + h)+(uint)c)%htsize; /* h*33+c */
	}

	return (size_t)(h%htsize);
}

/* Function used to do double hashing (proposed by Donald E. Knuth) */
inline size_t
step_value(uchar *word, size_t len, size_t htsize)
{
	uint h = (uint)(4294967197u);
	for(size_t i = 0; i < len; i++){
		h = ((h << 5) ^ (h >> 27)) ^ word[i];
	}
	h = h%(htsize-1);
	if(h==0)
		return 1;
	return (size_t)h;
}

/*compute the first prime number higher or equal to n*/
inline size_t
nearest_prime(size_t n)
{
	size_t prime = n;
	size_t sqrt_prime, i;
	/*we will supose that prime will not be longer than the maximum  unsigned int value*/
	while(true){
		if(prime%2 !=0){
			sqrt_prime = (size_t)(sqrt(prime)+1);
			for(i=3; i<sqrt_prime; i+=2){
				if(prime%i==0)
					break;
			}
			if(i>=sqrt_prime)
				return prime;
		}
		prime++;
	}
	return prime;
}

#endif  /* _HASHUTILS_H */

