/* Test.cpp
 * Copyright (C) 2012, Francisco Claude & Rodrigo Canovas & Miguel A. Martinez-Prieto
 * all rights reserved.
 *
 * Script for testing string dictionaries built with the library of Compressed
 * String Dictionaries (libCSD). These structures are based on the techniques 
 * presented in:
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
 * This library is distributed in the hope that it will be useTestful,
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

#include <time.h>

#include "StringDictionary.h"

using namespace std;

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


void useTest()
{
	cerr << endl;
	cerr << " ******************************************************************************* " << endl;
	cerr << " *** TEST script for the library of Compressed String Dictionaries (libCSD). *** " << endl;
	cerr << " ******************************************************************************* " << endl;
	cerr << endl;
	cerr << " ----- test <mode> <in>" << endl;
	cerr << endl;
	cerr << " <mode> 1 : Test the LOCATE function." << endl;
	cerr << " <mode> 2 : Test the EXTRACT function." << endl;
	cerr << " <mode> 3 : Test the PREFIX-based functions." << endl;
	cerr << " <mode> 3 : Test the SUBSTRING-based functions." << endl;
	cerr << endl;
	cerr << " <in> : input file containing the compressed string dictionary." << endl;
	cerr << endl;
}

void locate(StringDictionary *dict)
{
	{
		cerr << endl;
		cerr << " **************************************************** " << endl;
		cerr << " *** Locating all valid strings in the dictionary *** " << endl;
		cerr << " **************************************************** " << endl;
	
		size_t elements = dict->numElements();
		size_t valid = 0;
		size_t errors = 0;

		for (size_t i=1; i<=elements; i++)
		{
			uint strLen;
			uchar *str = dict->extract(i, &strLen);
			size_t id = dict->locate(str, strLen);
		
			if (id == i) valid++;
			else errors++;

			delete [] str;
		}

		cerr << "     - " << valid << " strings are located correctly." << endl;
		cerr << "     - " << errors << " strings are located with errors." << endl;
	}

	{
		cerr << endl;
		cerr << " **************************************************************** " << endl;
		cerr << " *** Locating non-valid strings preceding the first valid one *** " << endl;
		cerr << " **************************************************************** " << endl;

		size_t valid = 0;
		size_t errors = 0;

		uint strLen;
		uchar *str = dict->extract(1, &strLen);

		for (uint i=strLen; i>0; i--)
		{
			uchar *substr = new uchar[i+1];
			strncpy((char*)substr, (char*)str, i);
			substr[i-1]--; substr[i] = '\0';

			size_t id = dict->locate(substr, i);

			if (id == 0) valid++;
			else errors++;
			
			delete [] substr;
		}

		delete [] str;

		cerr << "     - " << valid << " strings are located correctly." << endl;
		cerr << "     - " << errors << " strings are located with errors." << endl;

	}

	{
		cerr << endl;
		cerr << " **************************************************************** " << endl;
		cerr << " *** Locating non-valid strings succeeding the last valid one *** " << endl;
		cerr << " **************************************************************** " << endl;

		size_t valid = 0;
		size_t errors = 0;

		uint strLen;
		uchar *str = dict->extract(dict->numElements(), &strLen);

		for (uint i=strLen; i>0; i--)
		{
			uchar *substr = new uchar[i+1];
			strncpy((char*)substr, (char*)str, i);
			substr[i-1]++; substr[i] = '\0';

			size_t id = dict->locate(substr, i);

			if (id == 0) valid++;
			else errors++;
			
			delete [] substr;
		}

		delete [] str;

		cerr << "     - " << valid << " strings are located correctly." << endl;
		cerr << "     - " << errors << " strings are located with errors." << endl;
	}

	{
		cerr << endl;
		cerr << " ********************************************* " << endl;
		cerr << " *** Locating non-valid 'internal' strings *** " << endl;
		cerr << " ********************************************* " << endl;

		size_t valid = 0;
		size_t errors = 0;

		srand (time(NULL));

		for (uint i=0; i<100; i++)
		{
			uint id = 1+(rand() % dict->numElements());

			uint strLen;
			uchar *str = dict->extract(id, &strLen);

			for (uint j=strLen; j>0; j--)
			{
				uchar *substr = new uchar[j+1];
				strncpy((char*)substr, (char*)str, j);
				substr[j-1]++; substr[j] = '\0';

				size_t id = dict->locate(substr, j);

				if (id == 0) valid++;
				else 
				{ 
					uchar *substr2 = dict->extract(id, &strLen);

					// It is really an error
					if (strcmp((char*)substr,(char*)substr2) != 0) errors++;
					delete [] substr2;
				}
			
				delete [] substr;
			}
	
			delete [] str;
		}

		cerr << "     - " << valid << " strings are located correctly." << endl;
		cerr << "     - " << errors << " strings are located with errors." << endl;
	}
}

void extract(StringDictionary *dict)
{
	{
		cerr << endl;
		cerr << " *************************************************** " << endl;
		cerr << " *** Extracting all valid IDs in the dictionary *** " << endl;
		cerr << " *************************************************** " << endl;
	
		size_t elements = dict->numElements();
		size_t valid = 0;
		size_t errors = 0;

		for (size_t i=1; i<=elements; i++)
		{
			uint strLen;
			uchar *str = dict->extract(i, &strLen);
			size_t id = dict->locate(str, strLen);
		
			if (id == i) valid++;
			else errors++;

			delete [] str;
		}

		cerr << "     - " << valid << " IDs are extracted correctly." << endl;
		cerr << "     - " << errors << " IDs are extracted with errors." << endl;
	}

	{
		cerr << endl;
		cerr << " ******************************** " << endl;
		cerr << " *** Extracting non-valid IDs *** " << endl;
		cerr << " ******************************** " << endl;

		size_t valid = 0;
		size_t errors = 0;

		{
			uint strLen;
			uchar *str = dict->extract(0, &strLen);

			if ((str == NULL) && (strLen ==0)) valid++;
			else errors++;

			delete [] str;
		}

		{
			uint strLen;
			uchar *str = dict->extract(dict->numElements()+1, &strLen);

			if ((str == NULL) && (strLen == 0)) valid++;
			else errors++;

			delete [] str;
		}

		cerr << "     - " << valid << " IDs are extracted correctly." << endl;
		cerr << "     - " << errors << " IDs are extracted with errors." << endl;
	}
}

void prefix(StringDictionary *dict)
{

	{
		cerr << endl;
		cerr << " ***************************************************************************************** " << endl;
		cerr << " *** Locating ID ranges for all prefixes in the first valid string and extracting them *** " << endl;
		cerr << " ***************************************************************************************** " << endl;

		size_t valid = 0;
		size_t errorsL = 0, errorsR = 0, errorsI = 0, errorsE = 0;

		uint strLen;
		uchar *str = dict->extract(1, &strLen);

		for (uint i=strLen; i>0; i--)
		{
			uchar *substr = new uchar[i+1];

			strncpy((char*)substr, (char*)str, i);
			substr[i] = '\0';

			IteratorDictID* itI = dict->locatePrefix(substr, i);
			IteratorDictString* itS = dict->extractPrefix(substr, i);

			if (itI->hasNext())
			{
				size_t current = itI->next();

				if ((current-1) > 0)
				{
					// Checking if the preceding ID is not prefixed
					size_t prefLen;
					uchar *prefixed = dict->extract(current-1, &prefLen);

					if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsL++;

					delete [] prefixed;
				}

				{
					// Checking the internal IDs
					while (true)
					{
						size_t prefLen;
						uchar *prefixed = dict->extract(current, &prefLen);

						if (strncmp((char*)substr, (char*)prefixed, i) == 0) valid++;
						else errorsI++;

						{
							size_t prefLen;
							uchar *prefixedS = itS->next(&prefLen);

							if (strcmp((char*)prefixed, (char*)prefixedS) != 0) errorsE++;

							delete [] prefixedS;
						}

						delete [] prefixed;

						if (itI->hasNext()) current = itI->next();
						else break;
					}
				}

				if ((current+1) < dict->numElements())
				{
					// Checking if the succeding ID is not prefixed
					size_t prefLen;
					uchar *prefixed = dict->extract(current+1, &prefLen);

					if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsR++;

					delete [] prefixed;
				}

			}
			else
			{
				// The range is empty
				errorsL++; errorsR++;
			}

			delete itI; delete itS;			
			delete [] substr;
		}

		delete [] str;

		cerr << "     - " << valid << " IDs are located and extracted correctly in their corresponding ranges." << endl;
		cerr << "     - " << errorsI << " IDs are located with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsE << " IDs are extracted with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsL << " left limits are no correct." << endl;
		cerr << "     - " << errorsR << " right limits are no correct." << endl;
	}


	{
		cerr << endl;
		cerr << " **************************************************************************************** " << endl;
		cerr << " *** Locating ID ranges for all prefixes in the last valid string and extracting them *** " << endl;
		cerr << " **************************************************************************************** " << endl;

		size_t valid = 0;
		size_t errorsL = 0, errorsR = 0, errorsI = 0, errorsE = 0;

		uint strLen;
		uchar *str = dict->extract(dict->numElements(), &strLen);

		for (uint i=strLen; i>0; i--)
		{
			uchar *substr = new uchar[i+1];
			strncpy((char*)substr, (char*)str, i);
			substr[i] = '\0';

			IteratorDictID* itI = dict->locatePrefix(substr, i);
			IteratorDictString* itS = dict->extractPrefix(substr, i);

			if (itI->hasNext())
			{
				size_t current = itI->next();

				if ((current-1) > 0)
				{
					// Checking if the preceding ID is not prefixed
					size_t prefLen;
					uchar *prefixed = dict->extract(current-1, &prefLen);

					if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsL++;

					delete [] prefixed;
				}

				{
					// Checking the internal IDs
					while (true)
					{
						size_t prefLen;
						uchar *prefixed = dict->extract(current, &prefLen);

						if (strncmp((char*)substr, (char*)prefixed, i) == 0) valid++;
						else errorsI++;

						{
							size_t prefLen;
							uchar *prefixedS = itS->next(&prefLen);

							if (strcmp((char*)prefixed, (char*)prefixedS) != 0) errorsE++;

							delete [] prefixedS;
						}

						delete [] prefixed;

						if (itI->hasNext()) current = itI->next();
						else break;
					}
				}

				if ((current+1) < dict->numElements())
				{
					// Checking if the succeding ID is not prefixed
					size_t prefLen;
					uchar *prefixed = dict->extract(current+1, &prefLen);

					if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsR++;

					delete [] prefixed;
				}

			}
			else
			{
				// The range is empty
				errorsL++; errorsR++;
			}

			delete itI; delete itS;	
			delete [] substr;
		}

		delete [] str;

		cerr << "     - " << valid << " IDs are located and extracted correctly in their corresponding ranges." << endl;
		cerr << "     - " << errorsI << " IDs are located with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsE << " IDs are extracted with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsL << " left limits are no correct." << endl;
		cerr << "     - " << errorsR << " right limits are no correct." << endl;
	}

	{
		cerr << endl;
		cerr << " ************************************************************************************************ " << endl;
		cerr << " *** Locating ID ranges for all prefixes in some 'internal' valid strings and extracting them *** " << endl;
		cerr << " ************************************************************************************************ " << endl;

		size_t valid = 0;
		size_t errorsL = 0, errorsR = 0, errorsI = 0, errorsE = 0;

		srand (time(NULL));

		for (uint z=0; z<100; z++)
		{
			uint id = 1+(rand() % dict->numElements());

			uint strLen;
			uchar *str = dict->extract(id, &strLen);

			for (uint i=strLen; i>0; i--)
			{
				uchar *substr = new uchar[i+1];
				strncpy((char*)substr, (char*)str, i);
				substr[i] = '\0';

				IteratorDictID* itI = dict->locatePrefix(substr, i);
				IteratorDictString* itS = dict->extractPrefix(substr, i);

				if (itI->hasNext())
				{
					size_t current = itI->next();

					if ((current-1) > 0)
					{
						// Checking if the preceding ID is not prefixed
						size_t prefLen;
						uchar *prefixed = dict->extract(current-1, &prefLen);

						if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsL++;

						delete [] prefixed;
					}

					{
						// Checking the internal IDs
						while (true)
						{
							size_t prefLen;
							uchar *prefixed = dict->extract(current, &prefLen);

							if (strncmp((char*)substr, (char*)prefixed, i) == 0) valid++;
							else errorsI++;

							{
								size_t prefLen;
								uchar *prefixedS = itS->next(&prefLen);

								if (strcmp((char*)prefixed, (char*)prefixedS) != 0) errorsE++;

								delete [] prefixedS;
							}

							delete [] prefixed;

							if (itI->hasNext()) current = itI->next();
							else break;
						}
					}

					if ((current+1) < dict->numElements())
					{
						// Checking if the succeding ID is not prefixed
						size_t prefLen;
						uchar *prefixed = dict->extract(current+1, &prefLen);

						if (strncmp((char*)substr, (char*)prefixed, i) == 0) errorsR++;

						delete [] prefixed;
					}

				}
				else
				{
					// The range is empty
					errorsL++; errorsR++;
				}

				delete itI; delete itS;
				delete [] substr;
			}

			delete [] str;
		}

		cerr << "     - " << valid << " IDs are located and extracted correctly in their corresponding ranges." << endl;
		cerr << "     - " << errorsI << " IDs are located with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsE << " IDs are extracted with errors in their corresponding ranges." << endl;
		cerr << "     - " << errorsL << " left limits are no correct." << endl;
		cerr << "     - " << errorsR << " right limits are no correct." << endl;
	}
}
				
int 
main(int argc, char* argv[])
{
	if (argc == 3)
	{
		ifstream in(argv[2]);

		if (in.good())
		{
			StringDictionary *dict = StringDictionary::load(in);
			in.close();

			if (dict == NULL) { checkDict(); exit(0); }

			int type = atoi(argv[1]);

			switch (type)
			{
				case 1:
					locate(dict);
					break;

				case 2:
					extract(dict);
					break;

				case 3:
					prefix(dict);
					break;

				default:
					useTest();
					break;
			}

			delete dict;


		}
		else { checkFile(); exit(0); }
	}
	else useTest();
}

