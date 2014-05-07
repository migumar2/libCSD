/* Build.cpp
 * Copyright (C) 2014, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Script for building indexed string dictionaries in compressed space. 
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

#ifndef _BUILD_CPP
#define _BUILD_CPP

#include <fstream>
#include <iostream>
using namespace std;

#include "StringDictionary.h"
#include "iterators/IteratorDictStringPlain.h"
#include "utils/LogSequence.h"


void checkDict()
{
	cerr << endl;
	cerr << " ******************************************************************************** " << endl;
	cerr << " *** Checks the given file because it does not represent any valid dictionary *** " << endl;
	cerr << " ******************************************************************************** " << endl;
	cerr << endl;
}

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
	cerr << " ----- ./Build <type> <parameters> <in> <out>" << endl;

	cerr << " type: 1 => Build HASH dictionary" << endl;
	cerr << " \t <compress> : 'h' for Huffman; 'r' for RePair compression." << endl;
	cerr << " \t <overhead> : hash table overhead over the number of strings." << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;

	cerr << " type: 2 => Build HASHDAC dictionary" << endl;
	cerr << " \t <compress> : 'h' for Huffman; 'r' for RePair compression." << endl;
	cerr << " \t <overhead> : hash table overhead over the number of strings." << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;

	cerr << " type: 3 => Build PLAIN FRONT CODING dictionary" << endl;
	cerr << " \t <compress> : 'p' for plain (uncompressed) representation; 'r' for RePair compression" << endl;
	cerr << " \t <bucketsize> : number of strings per bucket." << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;

	cerr << " type: 4 => Build HU-TUCKER FRONT CODING dictionary" << endl;
	cerr << " \t <compress> : tecnique used for internal string compression." << endl;
	cerr << " \t              't' for HuTucker; 'h' for Huffman; 'r' for RePair compression." << endl;
	cerr << " \t <bucketsize> : number of strings per bucket." << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;

	cerr << " type: 5 => Build RE-PAIR DAC dictionary" << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;

	cerr << " type: 6 => Build FMINDEX dictionary" << endl;
	cerr << " \t <compress> : 'p' for plain bitmaps; 'c' for compresed bitmaps." << endl;
	cerr << " \t <BS sampling> : sampling value for the bitmaps" << endl;
	cerr << " \t <BWT sampling> : sampling step for the BWT (0 for no sampling)" << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
	cerr << " \t <out> : output file for storing the dictionary." << endl;
	cerr << endl;

	cerr << " type: 7 => Build XBW dictionary" << endl;
	cerr << " \t <in> : input file containing the set of '\\0'-delimited strings." << endl;
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
				if (argc != 6) { useBuild(); break; }

				ifstream in(argv[4]);

				if (in.good())
				{

					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uint overhead = atoi(argv[3]);

					uchar *str = loadValue<uchar>(in, lenStr+1);
					str[lenStr] = '\0';
					in.close();

					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					StringDictionary *dict = NULL;
					string filename = string(argv[5]);

					if (argv[2][0] == 'h')
					{
						// Huffman compression
						dict = new StringDictionaryHASHHF(it, lenStr, overhead);
						filename += string(".hashhf");
					}
					else
					{
						// RePair compression
						dict = new StringDictionaryHASHRPF(it, lenStr, overhead);
						filename += string(".hashrpf");
					}

					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
				}
				else checkFile();

				break;
			}

			case 2:
			{
				if (argc != 6) { useBuild(); break; }

				ifstream in(argv[4]);

				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uint overhead = atoi(argv[3]);

					uchar *str = loadValue<uchar>(in, lenStr+1);
					str[lenStr] = '\0';
					in.close();

					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					StringDictionary *dict = NULL;
					string filename = string(argv[5]);

					if (argv[2][0] == 'h')
					{
						// Huffman compression
						dict = new StringDictionaryHASHUFFDAC(it, lenStr, overhead);
						filename += string(".hashuffdac");
					}
					else
					{
						// RePair compression
						dict = new StringDictionaryHASHRPDAC(it, lenStr, overhead);
						filename += string(".hashrpdac");
					}

					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
				}
				else checkFile();

				break;
			}

			case 3:
			{
				if (argc != 6) { useBuild(); break; }

				ifstream in(argv[4]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uint bucketsize = atoi(argv[3]);

					uchar *str = loadValue<uchar>(in, lenStr);
					in.close();

					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					StringDictionary *dict = NULL;
					string filename = string(argv[5]);

					if (argv[2][0] == 'p')
					{
						// Uncompressed internal string
						dict = new StringDictionaryPFC(it, bucketsize);
						filename += string(".pfc");
					}
					else if (argv[2][0] == 'r')
					{
						// RePair compression
						dict = new StringDictionaryRPFC(it, bucketsize);
						filename += string(".rpfc");
					}
					else
					{
						useBuild();
						delete it;
						return 0;
					}

					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
				}
				else checkFile();

				break;
			}

			case 4:
			{
				if (argc != 6) { useBuild(); break; }

				ifstream in(argv[4]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uint bucketsize = atoi(argv[3]);

					uchar *str = loadValue<uchar>(in, lenStr);
					in.close();

					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					StringDictionary *dict = NULL;
					string filename = string(argv[5]);

					switch (argv[2][0])
					{
						case 't':
						{
							// HuTucker compression
							dict = new StringDictionaryHTFC(it, bucketsize);
							filename += string(".htfc");
							break;
						}

						case 'h':
						{
							// Huffman compression
							dict = new StringDictionaryHHTFC(it, bucketsize);
							filename += (".hhtfc");
							break;
						}

						case 'r':
						{
							// RePair compression
							dict = new StringDictionaryRPHTFC(it, bucketsize);
							filename += string(".rphtfc");
							break;
						}

						default:
						{
							useBuild();
							delete it;
							return 0;
						}
					}

					ofstream out((char*)filename.c_str());
					dict->save(out);
					out.close();
					delete dict;
				}
				else checkFile();

				break;
			}

			case 5:
			{
				if (argc != 4) { useBuild(); break; }

				ifstream in(argv[2]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uchar *str = loadValue<uchar>(in, lenStr);
					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					in.close();

					StringDictionary *dict = new StringDictionaryRPDAC(it);
					string filename = string(argv[3])+string(".rpdac");

					ofstream out((char*)filename.c_str());
					dict->save(out);
 					out.close();
					delete dict;
				}
				else checkFile();

				break;
			}

			case 6:
			{
				if (argc != 7) { useBuild(); break; }

				ifstream in(argv[5]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uchar *str = loadValue<uchar>(in, lenStr);
					in.close();

					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr);
					StringDictionary *dict = NULL;
					string filename;

					uint BWTsampling = atoi(argv[4]);

					if (argv[2][0] == 'p')
					{
						// Plain bitmaps
						dict = new StringDictionaryFMINDEX(it, false, atoi(argv[3]), BWTsampling);
						delete it;

						filename = string(argv[6])+string(".")+string(argv[3])+string(".rg.fmi");

					}
					else if (argv[2][0] == 'c')
					{
						// Compressed bitmaps
						dict = new StringDictionaryFMINDEX(it, true, atoi(argv[3]), BWTsampling);
						delete it;

						filename = string(argv[6])+string(".")+string(argv[3])+string(".rrr.fmi");
					}
					else
					{
						checkFile();
						break;
					}

					ofstream out((char*)filename.c_str());
					dict->save(out);
 					out.close();
					delete dict;
				}
				else checkFile();

				break;
			}

			case 7:
			{
				if (argc != 4) { useBuild(); break; }

				ifstream in(argv[2]);
				if (in.good())
				{
					in.seekg(0,ios_base::end);
					uint lenStr = in.tellg()/sizeof(uchar);
					in.seekg(0,ios_base::beg);

					uchar *str = loadValue<uchar>(in, lenStr);
					IteratorDictString *it = new IteratorDictStringPlain(str, lenStr-1);
					in.close();

					StringDictionary *dict = new StringDictionaryXBW(it);
					delete it;

					string filename = string(argv[3])+string(".xbw");
					ofstream out((char*)filename.c_str());
					dict->save(out);
			 		out.close();
					delete dict;
				}
				else checkFile();

				break;
			}

			default:
			{
				useBuild();
				break;
			}
		}
	}
	else
	{
		useBuild();
	}
}

#endif  /* _BUILD_CPP */

