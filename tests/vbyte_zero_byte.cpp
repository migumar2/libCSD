/* Regression test: the HTFC and HHTFC scans took the first byte of the VByte
 * heading an internal string, a 0 where the shared prefix is a multiple of 128
 * chars long, for the end of the previous or of the current string.
 *
 *   make tests/vbyte_zero_byte && tests/vbyte_zero_byte
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include "StringDictionary.h"
#include "iterators/IteratorDictStringPlain.h"

static const char *PATH = "tests/vbyte_zero_byte.dict";

static IteratorDictString *plain(const vector<string> &keys)
{
	size_t bytes = 0;
	for (size_t i=0; i<keys.size(); i++) bytes += keys[i].size()+1;

	uchar *text = new uchar[bytes];
	size_t at = 0;
	for (size_t i=0; i<keys.size(); i++)
	{
		memcpy(text+at, keys[i].c_str(), keys[i].size()+1);
		at += keys[i].size()+1;
	}

	return new IteratorDictStringPlain(text, bytes);
}

/* Saves and loads the dictionary, because the library only queries a loaded
   one, and asks it for every key by string, by ID and through extractTable. */
static void check(StringDictionary *built, const vector<string> &keys, const char *name)
{
	{
		ofstream out(PATH, ios::binary);
		built->save(out);
	}
	delete built;

	ifstream in(PATH, ios::binary);
	StringDictionary *dict = StringDictionary::load(in, 0);
	in.close();

	for (size_t i=0; i<keys.size(); i++)
	{
		uint len = 0;
		uchar *back = dict->extract(i+1, &len);
		bool same = keys[i] == (char*)back;
		delete [] back;

		uint id = dict->locate((uchar*)keys[i].c_str(), keys[i].size());

		if (!same || (id != i+1))
		{
			printf("%s, %zu keys: key %zu has the wrong answer\n", name, keys.size(), i+1);
			exit(1);
		}
	}

	IteratorDictString *it = dict->extractTable();
	for (size_t i=0; i<keys.size(); i++)
	{
		uint len = 0;
		uchar *back = it->next(&len);
		bool same = keys[i] == (char*)back;
		delete [] back;

		if (!same)
		{
			printf("%s, %zu keys: extractTable differs at key %zu\n", name, keys.size(), i+1);
			exit(1);
		}
	}
	delete it;
	delete dict;
}

int main()
{
	const size_t shared[] = {128, 256, 384, 16384};
	const uint buckets[] = {8, 64};

	for (size_t s=0; s<sizeof(shared)/sizeof(shared[0]); s++)
	{
		for (size_t b=0; b<sizeof(buckets)/sizeof(buckets[0]); b++)
		{
			// Groups of keys sharing exactly that many chars with the
			// previous one in the group. So few distinct suffixes keep
			// the codes short, and a chunk ends a string and begins the
			// next, or holds a whole string.
			vector<string> keys;
			for (uint group=0; group<buckets[b]; group++)
				for (uint i=0; i<4; i++)
				{
					string key(shared[s]-1, 'a');
					key += (char)('A'+group);
					key += (char)('a'+i);
					key += string(i%2, 'z');
					keys.push_back(key);
				}

			check(new StringDictionaryHTFC(plain(keys), buckets[b]), keys, "HTFC");
			check(new StringDictionaryHHTFC(plain(keys), buckets[b]), keys, "HHTFC");
		}
	}

	remove(PATH);
	printf("ok\n");
	return 0;
}
