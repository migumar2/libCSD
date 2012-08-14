/* Build.cpp
 * Copyright (C) 2012, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Script for building indexed string dictionaries in compressed space. These
 * structures are based on the techniques presented in:
 *
 *   ==========================================================================
 *     "Compressed String Dictionaries"
 *     Nieves R. Brisaboa, Rodrigo Canovas, Francisco Claude, 
 *     Miguel A. Martinez-Prieto and Gonzalo Navarro.
 *     10th Symposium on Experimental Algorithms (SEA'2011), p.136-147, 2011.
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
 *   Francisco Claude:  	fclaude@cs.uwaterloo.ca
 *   Rodrigo Canovas:  		rcanovas@student.unimelb.edu.au
 *   Miguel A. Martinez-Prieto:	migumar2@infor.uva.es
 */


#include <fstream>
#include <iostream>

#include "StringDictionary.h"

using namespace std;

void checkFile()
{
	cerr << endl;
	cerr << " ****************************************************************** " << endl;
	cerr << " *** Checks the given path because it does not contain any file *** " << endl;
	cerr << " ****************************************************************** " << endl;
	cerr << endl;
}

void useBuild()
{
	cerr << endl;
	cerr << " ************************************************************************** " << endl;
	cerr << " *** BUILD script for indexing string dictionaries in compressed space. *** " << endl;
	cerr << " ************************************************************************** " << endl;
	cerr << endl;
	cerr << " ----- test <type> <parameters>" << endl;
	cerr << endl;
	cerr << " type: 1 => Build PLAIN FRONT CODING dictionary" << endl;
	cerr << " \t <bucketsize> : number of strings per bucket." << endl;
	cerr << " \t <in> : input file containing the set of '\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;
}
				
int 
main(int argc, char* argv[])
{
	if (argc > 1)
	{
		int type = atoi(argv[1]);

		switch (type)
		{
			case 1:
			{
				if (argc != 5) { useBuild(); break; }

				ifstream in(argv[3]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uchar *str = loadValue<uchar>(in, lenStr);
					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					in.close();

					uint bucketsize = atoi(argv[2]);

					StringDictionary *dict = new StringDictionaryPFC(it, bucketsize);
					delete it;

					string filename = string(argv[4])+string(".pfc");
					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
				}
				else checkFile();

				break;
			}
		
			default:
				useBuild();
		}
	}
	else
	{
		useBuild();
	}
}




/*
		case 'p':
		{
			cerr << "PARSING dictionary" << endl;

			ifstream in(argv[2]);
			if (in.good())
			{
				in.seekg(0,ios_base::end);
				uint size = in.tellg()/sizeof(uchar);
				in.seekg(0,ios_base::beg);

				uchar* dict = loadValue<uchar>(in, size);
				in.close();

				cout << "File of: " << size << " bytes loaded" << endl;

				for (uint i=0; i<size; i++)
					if (dict[i] == '\n') dict[i] = '\0';

				string filename = string(argv[2])+string(".strdic");
				ofstream out((char*)filename.c_str());
				saveValue(out, dict, size);
				out.close();

				delete [] dict;
			}
			else
			{
				cerr << "No hay un fichero en la ruta indicada" << endl;
			}

			break;
		 }

*/


