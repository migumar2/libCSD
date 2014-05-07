/*
 * File: LogSequence.cpp
 * Last modified: $Date: 2011-08-21 05:35:30 +0100 (dom, 21 ago 2011) $
 * Revision: $Revision: 180 $
 * Last modified by: $Author: mario.arias $
 *
 * Copyright (C) 2012, Mario Arias, Javier D. Fernandez, Miguel A. Martinez-Prieto
 * All rights reserved.
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
 *   Mario Arias:               mario.arias@gmail.com
 *   Javier D. Fernandez:       jfergar@infor.uva.es
 *   Miguel A. Martinez-Prieto: migumar2@infor.uva.es
 *
 */

#include "LogSequence.h"

LogSequence::LogSequence()
{
}

LogSequence::LogSequence(unsigned int numbits, size_t capacity)
{
  this->numbits = numbits;
  this->numentries = capacity;
  this->maxval = maxVal(numbits);

  arraysize = numElementsFor(numbits, numentries);
  array = new size_t[arraysize];
  for (size_t i=0; i<arraysize; i++) array[i] = 0;
}

LogSequence::LogSequence(vector<size_t> *v, unsigned int numbits)
{
  this->numbits = numbits;
  this->numentries = v->size();
  this->maxval = maxVal(numbits);

  arraysize = numElementsFor(numbits, numentries);
  array = new size_t[arraysize];
  for (size_t i=0; i<arraysize; i++) array[i] = 0;

  for (size_t i=0; i<numentries; i++) setField(i, (*v)[i]);
}

LogSequence::LogSequence(ifstream &in)
{
  numbits = loadValue<uchar>(in);
  numentries = loadValue<size_t>(in);

  maxval = maxVal(numbits);
  size_t numbytes = numBytesFor(numbits, numentries);
  if ((numbytes % 8) != 0) numbytes += 8-(numbytes%8);

  arraysize = numElementsFor(numbits, numentries);
  array = (size_t*)loadValue<uchar>(in, numbytes);
}

size_t LogSequence::getField(size_t position)
{
  if(position>numentries) {
    throw "Trying to get an element bigger than the array.";
  }

  return get_field(&array[0], numbits, position);
}

size_t LogSequence::getNumberOfElements()
{
  return numentries;
}

void LogSequence::setField(size_t position, size_t value) 
{
  if(position>numentries) {
    throw "Trying to modify a position out of the structure capacity. Use push_back() instead";
  }
  if(value>maxval) {
    throw "Trying to insert a value bigger that expected. Please increase numbits when creating the data structure.";
  }

  set_field(array, numbits, position, value);
}

size_t LogSequence::getSize()
{
  return numBytesFor(numbits, numentries)+sizeof(LogSequence)+sizeof(vector<size_t>);
}


void LogSequence::save(ofstream &out)
{
	saveValue<uchar>(out, numbits);
	saveValue<size_t>(out, numentries);

	size_t numbytes = numBytesFor(numbits, numentries);

	if ((numbytes % 8) != 0) numbytes += 8-(numbytes%8);
	saveValue<uchar>(out, (uchar*)array, numbytes);
}

LogSequence::~LogSequence() 
{
	delete [] array;
}
